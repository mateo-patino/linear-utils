#include "linalg/view.h"

/*
* Matrix addition
*/
int matrix_add(matrixv_t *C, matrixv_t *A, matrixv_t *B) {
    if (!C || !A || !B || A->nrow != B->nrow || A->ncol != B->ncol 
        || C->nrow != A->nrow || C->ncol != A->ncol) {
        return -1;
    }

    /*
    * CONTINUE: document the use of restrict and indicate to callers what it requires
    *
    * Look into other keywords like const you can add to the function declarations here
    * to help the compiler optimize things.
    */

    scalar *restrict C_data = C->data;
    const scalar *restrict A_data = A->data, *restrict B_data = B->data;

    /* If C, A, and B are non-strided (contiguous in memory) views, do 1D loop */
    if ((C->row_stride == C->ncol && C->column_stride == 1) &&
        (A->row_stride == A->ncol && A->column_stride == 1) &&
        (B->row_stride == B->ncol && B->column_stride == 1)) {

        size_t nentry = A->nrow * A->ncol;

        #pragma omp simd 
        for (size_t i = 0; i < nentry; i++) {
            C_data[i] = A_data[i] + B_data[i];
        }

        return 0;
    }
   
    /* General loop if at least one view is strided (not contiguous in memory) */
    size_t nrow = C->nrow, ncol = C->ncol;
    size_t C_rs = C->row_stride, C_cs = C->column_stride;
    size_t A_rs = A->row_stride, A_cs = A->column_stride;
    size_t B_rs = B->row_stride, B_cs = B->column_stride;

    for (size_t i = 0; i < nrow; i++) { 
        #pragma omp simd
        for (size_t j = 0; j < ncol; j++) {
            C_data[i * C_rs + j * C_cs] = A_data[i * A_rs + j * A_cs] + B_data[i * B_rs + j * B_cs];
        }
    }

    return 0;
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


