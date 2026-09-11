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


#endif


