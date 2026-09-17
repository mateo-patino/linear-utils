#include "linalg/row_operations.h"

#include <math.h>



int swap_rows(size_t i, size_t j, matrixv_t *A) {
    if (!A || i >= A->nrow || j >= A->nrow) {
        return -1;
    }

    /* No op if swapping the same row with itself */
    if (i == j) {
        return 0;
    }

    /* NEEDSWORK: could likely do a non-strided path? Could in theory be slightly faster */
    
    size_t ncol = A->ncol, rs = A->row_stride, cs = A->column_stride;
    size_t i_idx, j_idx;
    scalar *data = A->data;
    scalar tmp;

    for (size_t c = 0; c < ncol; c++) {
        i_idx = i * rs + c * cs;
        j_idx = j * rs + c * cs;

        tmp = data[i_idx];
        data[i_idx] = data[j_idx];
        data[j_idx] = tmp;
    }

    return 0;     
}


/* Does row_i <- row_i + factor * row_j */
int add_row_multiple(size_t i, scalar factor, size_t j, matrixv_t *A) {
    if (!A || i >= A->nrow || j >= A->nrow) {
        return -1;
    }

    size_t ncol = A->ncol, rs = A->row_stride, cs = A->column_stride;
    scalar *data = A->data;

    /* Tiny optimization. row_i <- row_i + factor * row_i equals row_i <- (1 + factor) * row_i */
    if (i == j) {
        factor++;
        #pragma omp simd
        for (size_t c = 0; c < ncol; c++) {
            data[i * rs + c * cs] *= factor;
        }
        return 0;
    }

    #pragma omp simd
    for (size_t c = 0; c < ncol; c++) {
        data[i * rs + c * cs] += factor * data[j * rs + c * cs];
    }

    return 0;
}


int scale_row(size_t i, scalar factor, matrixv_t *A) {
    if (!A || i >= A->nrow) {
        return -1;
    }

    size_t ncol = A->ncol, rs = A->row_stride, cs = A->column_stride;
    scalar *data = A->data;

    #pragma omp simd
    for (size_t j = 0; j < ncol; j++) {
        data[i * rs + j * cs] *= factor;
    }

    return 0;
}


int to_upper_triangular(matrixv_t *A, int *swap_count) {
    if (!A || A->nrow != A->ncol) {
        return -1;
    }

    size_t ncol = A->ncol, nrow = A->nrow, rs = A->row_stride, cs = A->column_stride;
    scalar *data = A->data;

    /* Iterate through the columns in the matrix */
    int swaps = 0;
    scalar pivot_entry, factor;
    size_t largest_entry_row;
    for (size_t j = 0; j < ncol; j++) {
        
        /* 
        * Find the largest entry in the current column and make it the pivot by
        * making its row the pivot row. This helps us avoid multiplying by large
        * values which can amplify numerical errors.
        */
        largest_entry_row = j;
        for (size_t i = j + 1; i < nrow; i++) {
            if (fabs(data[i * rs + j * cs]) > fabs(data[largest_entry_row * rs + j * cs])) {
                largest_entry_row = i;
            }
        }

        /* Move the largest entry's row to the j-th row index. The pivot row is then at index j */
        if (largest_entry_row != j) {
            swap_rows(largest_entry_row, j, A);
            swaps++;
        }
        pivot_entry = data[j * (rs + cs)];

        /* 
        * Check if singular matrix (i.e. all entries in the rows >= j are 0) 
        * NEEDSWORK: Note we check exactly for 0 and not for proximity. Could we 
        * want to check for proximity instead?
        */
        if (pivot_entry == 0) {
            return 1;
        }

        /*
        * Cancel the entries below the pivot entry by adding a multiple of the pivot
        * row to the row of each entry.
        */
        for (size_t i = j + 1; i < nrow; i++) {
            factor = data[i * rs + j * cs] / pivot_entry;
            add_row_multiple(i, -1 * factor, j, A);
        }
        
        /* Repeat with the next column, starting one row below this time (note the i = j + 1) */ 
    }

    if (swap_count) { 
        *swap_count = swaps;
    }
    
    return 0;
}


int to_rref(matrixv_t *A) {
    if (!A) {
        return -1;
    }
    
    size_t nrow = A->nrow, ncol = A->ncol, rs = A->row_stride, cs = A->column_stride;
    scalar *data = A->data;
    size_t pivot_row = 0, largest_entry_row = 0;
    scalar pivot_entry = 0;

    for (size_t j = 0; j < ncol && pivot_row < nrow; j++) {

        /* Find largest entry in the column and swap its row with the pivot row */
        largest_entry_row = pivot_row;
        for (size_t i = pivot_row + 1; i < nrow; i++) {
            if (fabs(data[i * rs + j * cs]) > fabs(data[largest_entry_row * rs + j * cs])) {
                largest_entry_row = i;
            }
        }

        if (largest_entry_row != pivot_row) {
            swap_rows(pivot_row, largest_entry_row, A);
        }

        pivot_entry = data[pivot_row * rs + j * cs];
        
        /* Column has no pivots, so move on to the next column without updating pivot_row */
        if (pivot_entry == 0) {
            continue;
        }
        
        /* Normalize pivot row and zero out the non-pivot entries in this column */
        scale_row(pivot_row, 1 / pivot_entry, A); 
        for (size_t i = 0; i < nrow; i++) {
            if (i == pivot_row) {
                continue;
            }
            add_row_multiple(i, -1 * data[i * rs + j * cs], pivot_row, A);
        }

        /* Move on to the next row */
        pivot_row++;
    }

    return 0;
}

int inv_augmented_gauss_jordan(matrixv_t *restrict C, matrixv_t *restrict A) {
    if (!C || !A || A->nrow != A->ncol || A->nrow != C->nrow || A->ncol != C->ncol) {
        return -1;
    }
    
    size_t nrow = A->nrow, ncol = A->ncol, rs = A->row_stride, cs = A->column_stride;
    scalar *data = A->data;

    size_t largest_entry_row;
    scalar pivot_entry;
    scalar factor;
    for (size_t j = 0; j < ncol; j++) {

        /* In the current column, find row of the largest entry and make it the pivot row */
        largest_entry_row = j;
        for (size_t i = j + 1; i < nrow; i++) {
            if (fabs(data[i * rs + j * cs]) > fabs(data[largest_entry_row * rs + j * cs])) {
                largest_entry_row = i;
            }
        }

        /* Swap the row of the largest entry into the current pivot row. The pivot row is then at row j */
        if (largest_entry_row != j) {
            swap_rows(largest_entry_row, j, A);
            swap_rows(largest_entry_row, j, C);
        }
        pivot_entry = data[j * (rs + cs)];

        /* NEEDSWORK: again, decide if we should check for exact equality of zero. Same question for other row routines */
        if (pivot_entry == 0) {
            return 1;
        }

        /* Normalize pivot entry and zero out all other entries in this column */
        scale_row(j, 1 / pivot_entry, A);
        scale_row(j, 1 / pivot_entry, C);
        for (size_t i = 0; i < nrow; i++) {
            if (i == j) {
                continue;
            }
            factor = -1 * data[i * rs + j * cs];
            add_row_multiple(i, factor, j, A);
            add_row_multiple(i, factor, j, C);
        }

    }

    return 0;
}

