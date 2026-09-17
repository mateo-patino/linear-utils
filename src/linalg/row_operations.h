#ifndef ROW_OPERATIONS_H
#define ROW_OPERATIONS_H

#include "linalg/view.h"
#include "linalg/scalar.h"


/* 
* Elementary row operations. These operations are done in-place (i.e. they 
* modify matrix A). i and j are zero-indexed.
*/
int swap_rows(size_t i, size_t j, matrixv_t *A);

/* Set row_i <- row_i + factor * row_j */
int add_row_multiple(size_t i, scalar factor, size_t j, matrixv_t *A);

/*
* Multiplies row `i` by `factor`
*/
int scale_row(size_t i, scalar factor, matrixv_t *A);


/*
* Row-reduces `A` to an upper triangular form. 
*
* `A` must be a square matrix. Row-reduction is done in place, so `A` will be 
* directly modified. The number of row swaps performed to reduce `A` to upper
* triangular form will be written to `swap_count` if not NULL.
*
* It returns -1 upon failure, 0 upon success, and 1 if the matrix is singular and 
* cannot be put into upper triangular form.

* This functions uses Gaussian elimination with partial pivoting. 
*/
int to_upper_triangular(matrixv_t *A, int *swap_count);


/*
* Row-reduces `A` to RREF.
*
* Row-reduction is done in-place, so `A` will be directly modified. 
* 0 is returned upon success and -1 upon failure.
*/
int to_rref(matrixv_t *A);


/*
* Row-reduce `A` until it becomes the identity matrix while
* repeating every row operation on matrix `C`. `C` will ultimately
* contain the inverse matrix of `A`.
*
* It returns 0 upon success, -1 upon failure due to a bad input, and 1
* if matrix `A` is singular and cannot be converted into the identity matrix
* (i.e. `A` has no inverse).
*/
int inv_augmented_gauss_jordan(matrixv_t *restrict C, matrixv_t *restrict A);


#endif


