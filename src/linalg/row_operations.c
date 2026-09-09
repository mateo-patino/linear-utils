#include "linalg/row_operations.h"


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

