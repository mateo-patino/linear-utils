#include "linalg/view.h"

/*
* Matrix addition
*/
int matrix_add(matrixv_t *C, const matrixv_t *A, const matrixv_t *B) {
    if (!C || !A || !B || A->nrow != B->nrow || A->ncol != B->ncol 
        || C->nrow != A->nrow || C->ncol != A->ncol) {
        return -1;
    }

    scalar *C_data = C->data;
    const scalar *A_data = A->data, *B_data = B->data;

    /* If C, A, and B are non-strided (contiguous in memory) views, do 1D loop */
    if ((C->row_stride == C->ncol && C->column_stride == 1) &&
        (A->row_stride == A->ncol && A->column_stride == 1) &&
        (B->row_stride == B->ncol && B->column_stride == 1)) {

        size_t nentry = C->nrow * C->ncol;

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
int matrix_sub(matrixv_t *C, const matrixv_t *A, const matrixv_t *B) {
    if (!C || !A || !B || A->nrow != B->nrow || A->ncol != B->ncol
        || C->nrow != A->nrow || C->ncol != A->ncol) {
        return -1;
    }

    scalar *C_data = C->data; 
    const scalar *A_data = A->data, *B_data = B->data;
    
    /* Contiguous in memory (non-strided) view */
    if ((C->row_stride == C->ncol && C->column_stride == 1) &&
        (A->row_stride == A->ncol && A->column_stride == 1) && 
        (B->row_stride == B->ncol && B->column_stride == 1)) {

        size_t nentry = C->ncol * C->nrow;

        #pragma omp simd
        for (size_t i = 0; i < nentry; i++) {
            C_data[i] = A_data[i] - B_data[i];
        }
        
        return 0;
    }

    /* General 2-strided view */
    size_t nrow = C->nrow, ncol = C->ncol;
    size_t C_rs = C->row_stride, C_cs = C->column_stride;
    size_t A_rs = A->row_stride, A_cs = A->column_stride;
    size_t B_rs = B->row_stride, B_cs = B->column_stride;

    for (size_t i = 0; i < nrow; i++) {
        #pragma omp simd
        for (size_t j = 0; j < ncol; j++) {
            C_data[i * C_rs + j * C_cs] = A_data[i * A_rs + j * A_cs] - B_data[i * B_rs + j * B_cs];
        }
    }

    return 0;
}


/*
* Matrix multiplication (in the order AB).
*
* Pointers C, A, and B and their respective `data` pointers must be unique to prevent 
* undefined behavior (updating an entry and then reading it when the original value was 
* expected, specifically ). Hence, usage like A = AB or A = AA is not allowed.
*/
int matrix_mul(matrixv_t *restrict C, const matrixv_t *restrict A, const matrixv_t *restrict B) {
    if (!C || !A || !B || A->ncol != B->nrow || C->nrow != A->nrow
        || C->ncol != B->ncol) {
        return -1;
    }

    size_t nrow = C->nrow, ncol = C->ncol, nentry = ncol * nrow;
    size_t C_rs = C->row_stride, C_cs = C->column_stride;
    size_t A_rs = A->row_stride, A_cs = A->column_stride;
    size_t B_rs = B->row_stride, B_cs = B->column_stride;

    /* All matrix entries MUST unique */
    scalar *restrict C_data = C->data;
    const scalar *restrict A_data = A->data, *restrict B_data = B->data;

    /* A->ncol == B->nrow, this is the matching dimension between A and B */
    size_t shared_dimension = A->ncol; 

    /* Contiguous in memory (non-strided) view */
    if ((C_rs == C->ncol && C_cs == 1) &&
        (A_rs == A->ncol && A_cs == 1) &&
        (B_rs == B->ncol && B_cs == 1)) {
           
        /* Initialize the output matrix's entries to zero */
        #pragma omp simd
        for (size_t i = 0; i < nentry; i++) {
            C->data[i] = 0;
        }

        scalar Aik;
        for (size_t i = 0; i < nrow; i++) {
            for (size_t k = 0; k < shared_dimension; k++) {
                Aik = A_data[i * A_rs + k]; 

                #pragma omp simd
                for (size_t j = 0; j < ncol; j++) {
                    C_data[i * C_rs + j] += Aik * B_data[k * B_rs + j];
                }
            }
        }
        return 0;
    }

    /* Zero out the entries */
    for (size_t i = 0; i < nrow; i++) {
        #pragma omp simd
        for (size_t j = 0; j < ncol; j++) {
            C_data[i * C_rs + j * C_cs] = 0;
        }
    }

    scalar Aik; 
    for (size_t i = 0; i < nrow; i++) {
        for (size_t k = 0; k < shared_dimension; k++) {
            Aik = A_data[i * A_rs + k * A_cs]; 

            #pragma omp simd
            for (size_t j = 0; j < ncol; j++) {
                C_data[i * C_rs + j * C_cs] += Aik * B_data[k * B_rs + j * B_cs];
            }
        }
    }

    return 0;
}


/*
* Scalar-matrix multiplication
*/
int scalar_matrix_mul(matrixv_t *C, scalar s, const matrixv_t *A) {
    if (!C || !A || C->ncol != A->ncol || C->nrow != A->nrow) {
        return -1;
    }

    scalar *C_data = C->data;
    const scalar *A_data = A->data;

    /* Contiguous path */
    if ((C->row_stride == C->ncol && C->column_stride == 1) &&
        (A->row_stride == A->ncol && A->column_stride == 1)) {

        size_t nentry = C->nrow * C->ncol;

        #pragma omp simd
        for (size_t i = 0; i < nentry; i++) {
            C_data[i] = s * A_data[i];
        }

        return 0;
    }
    
    /* General strided path */
    size_t C_rs = C->row_stride, C_cs = C->column_stride;
    size_t A_rs = A->row_stride, A_cs = A->column_stride;
    size_t nrow = C->nrow, ncol = C->ncol;

    for (size_t i = 0; i < nrow; i++) {
        #pragma omp simd
        for (size_t j = 0; j < ncol; j++) {
            C_data[i * C_rs + j * C_cs] = s * A_data[i * A_rs + j * A_cs];
        }
    }

    return 0;
}


