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


int to_upper_triangular(matrixv_t *A, int *swap_count) {
    if (!A || A->nrow != A->ncol) {
        return -1;
    }

    size_t ncol = A->ncol, nrow = A->nrow, rs = A->row_stride, cs = A->column_stride;
    scalar *data = A->data;

    /* Iterate through the columns in the matrix */
    int swaps = 0;
    for (size_t j = 0; j < ncol; j++) {
        
        /* 
        * Find the largest entry in the current column and make it the pivot by
        * making its row the pivot row. This helps us avoid multiplying by large
        * values which can amplify numerical errors.
        */
        size_t prow = j;
        for (size_t i = j + 1; i < nrow; i++) {
            if (fabs(data[i * rs + j * cs]) > fabs(data[prow * rs + j * cs])) {
                prow = i;
            }
        }

        /* Move the pivot row to the j-th row index. The pivot row is then at index j */
        if (prow != j) {
            swap_rows(prow, j, A);
            swaps++;
        }
        scalar pivot_entry = data[j * rs + j * cs];

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
        scalar factor;
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

