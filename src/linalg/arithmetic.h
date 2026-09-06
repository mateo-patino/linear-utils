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
* Addition and subtraction allow pointer aliasing because A[i] = A[i] +/- A[i]
* and the like is well-defined.
*/
int matrix_add(matrixv_t *C, const matrixv_t *A, const matrixv_t *B);
int matrix_sub(matrixv_t *C, const matrixv_t *A, const matrixv_t *B);

/*
* Matrix multiplication does not allow pointer aliasing. 
* Pointers C, A, and B and their respective `data` pointers must be unique to prevent 
* undefined behavior (updating an entry and then reading it when the original value was 
* expected, specifically). Hence, usage like A = AB or A = AA is not allowed.
*/
int matrix_mul(matrixv_t *restrict C, const matrixv_t *restrict A, const matrixv_t *restrict B);

#endif
