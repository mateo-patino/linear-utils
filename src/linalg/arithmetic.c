#include "linalg/view.h"

/*
* Matrix addition
*/
int matrix_add(matrixv_t *C, matrixv_t *A, matrixv_t *B) {
    if (!C || !A || !B || A->nrow != B->nrow || A->ncol != B->ncol 
        || C->nrow != A->nrow || C->ncol != A->ncol) {
        return -1;
    }
    
    /* Naive vectorized implementation for a contiguous (non-strided) view */
    scalar *A_data = A->data, *B_data = B->data, *C_data = C->data;
    size_t nentry = A->nrow * A->ncol;
    #pragma omp simd /* Asks the compiler to a SIMD vector instruction instead of sequential loop */
    for (size_t i = 0; i < nentry; i++) {
        C_data[i] = A_data[i] + B_data[i];
    }
    return 0;

    /* NEEDSWORK: will need to implement addition strided view at some point */
}


/*
* Matrix subtraction.
*/
int matrix_sub(matrixv_t *C, matrixv_t *A, matrixv_t *B) {
    if (!C || !A || !B || A->nrow != B->nrow || A->ncol != B->ncol
        || C->nrow != A->nrow || C->ncol != A->ncol) {
        return -1;
    }
    
    /* Naive vectorized implementation for a contiguous (non-strided) view */
    scalar *A_data = A->data, *B_data = B->data, *C_data = C->data;
    size_t nentry = A->nrow * A->ncol;

    #pragma omp simd /* Asks the compiler to a SIMD vector instruction instead of sequential loop */
    for (size_t i = 0; i < nentry; i++) {
        C_data[i] = A_data[i] - B_data[i];
    }
    return 0;

    /* NEEDSWORK: will need to implement addition strided view at some point */
}


/*
* Matrix multiplication (in the order AB)
*/
int matrix_mul(matrixv_t *C, matrixv_t *A, matrixv_t *B) {
    if (!C || !A || !B || A->ncol != B->nrow || C->nrow != A->nrow
        || C->ncol != B->ncol) {
        return -1;
    }

    /* Initialize the output matrix's entries to zero */
    #pragma omp simd
    for (size_t i = 0, nentry = C->nrow * C->ncol; i < nentry; i++) {
        C->data[i] = 0;
    }

    size_t nrow = C->nrow, ncol = C->ncol;
    size_t C_rs = C->row_stride, A_rs = A->row_stride, B_rs = B->row_stride;
    scalar *C_data = C->data, *A_data = A->data, *B_data = B->data;

    /* A->ncol == B->nrow, this is the matching dimension between A and B */
    size_t shared_dimension = A->ncol; 

    for (size_t i = 0; i < nrow; i++) {
        for (size_t j = 0; j < ncol; j++) {
            for (size_t k = 0; k < shared_dimension; k++) {
                C_data[i * C_rs + j] += A_data[i * A_rs + k] * B_data[k * B_rs + j];
            }
        }
    }
    return 0;

    /*
    * NEEDSWORK: we need the 2D strided version of these functions 
    */
}


