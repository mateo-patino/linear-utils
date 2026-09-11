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
 

#endif
