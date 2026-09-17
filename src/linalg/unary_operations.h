#ifndef UNARY_OPERATIONS_H
#define UNARY_OPERATIONS_H

/*****************************************************************
* This file provides public functions for performing
* unary operations, such as computing determinants, converting
* to RREF, inversion, etc.
*****************************************************************/


#include "linalg/view.h"
#include "linalg/scalar.h"


/*
* Compute the determinant of `A`. 
*
* The result is written to `out` and 0 is returned upon success.
* If the determinant of `A` cannot be computed, 1 is returned if `A`
* is a singular matrix and -1 is returned if `A` is not square or 
* or another issue occurs. The function will reject any non-square
* matrix.
*
* Note: The entries of `A` will be modified by this function. Internally,
* row-reduction is used to put `A` into upper-triangular form, so provide
* a deep copy of `A` if you wish to preserve the original matrix.
*/
int matrix_det(scalar *out, matrixv_t *A);


/*
* Compute the Reduced Row-Echelon Form (RREF) of `A`.
* 
* The RREF of `A` is performed in-place, so `A` will be modified by this
* function.  0 is returned upon success and -1 upon failure.
*/
int matrix_rref(matrixv_t *A);


/*
* Compute the inverse matrix of `A`.
*
* `A` is a view of the input matrix to invert, and `C` must be
* a view containing the identity matrix that matches the dimensions 
* of `A`. 
*
* This function uses Gauss-Jordan elimination to row-reduce `A` 
* and `C` in-place until `A` is in RREF, at which point `C` will
* contain the inverse of `A`.
*
* 0 is returned upon success and -1 upon failure. 1 for math error???
*
* Note: This function reads from and writes to `C->data` and `A->data` directly.
* The `data` arrays must not overlap each other in storage or we'll get 
* undefined behavior.
*/
int matrix_inv(matrixv_t *restrict C, matrixv_t *restrict A);
/* TODO: implement the row operation kernel for this bad boy above */ 

#endif
