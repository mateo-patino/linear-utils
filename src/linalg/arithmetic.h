#ifndef ARITHMETIC_H
#define ARITHMETIC_H


#include "linalg/view.h"

/*
* Addition, subrtaction, and multiplication for matrices.
*
* All functions follow the model C = A (op) B. 0 is returned upon success
* and -1 upon failure.
*/


/*
* MATRIX-MATRIX ADDITION, SUBRACTION 
*
* Addition and subtraction allow pointer aliasing because A[i] = A[i] +/- A[i]
* and the like is well-defined.
*/
int matrix_add(matrixv_t *C, const matrixv_t *A, const matrixv_t *B);
int matrix_sub(matrixv_t *C, const matrixv_t *A, const matrixv_t *B);

/*
* MATRIX-MATRIX and SCALAR-MATRIX MULTIPLICATION
*
* Matrix-matrix multiplication does not allow pointer aliasing. 
* Pointers C, A, and B and their respective `data` pointers must be unique to prevent 
* data corruption (updating an entry and then reading it when the original value was 
* expected, specifically). Hence, usage like A = AB or A = AA is not allowed.
*
* Scalar-matrix multiplication does allow pointer aliasing, so A = sA is allowed.
*/
int matrix_mul(matrixv_t *restrict C, const matrixv_t *restrict A, const matrixv_t *restrict B);
int scalar_matrix_mul(matrixv_t *C, scalar s, const matrixv_t *A);

#endif
