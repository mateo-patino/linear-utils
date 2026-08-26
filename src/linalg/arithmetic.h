#ifndef ARITHMETIC_H
#define ARITHMETIC_H


#include "linalg/view.h"

/*
* Addition, subrtaction, and multiplication for matrices.
*
* All functions follow the model C = A (op) B. 0 is returned upon success
* and -1 upon failure.
*/

int matrix_add(matrixv_t *C, matrixv_t *A, matrixv_t *B);
int matrix_sub(matrixv_t *C, matrixv_t *A, matrixv_t *B);
int matrix_mul(matrixv_t *C, matrixv_t *A, matrixv_t *B);



#endif
