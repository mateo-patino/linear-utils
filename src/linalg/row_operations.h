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

#endif


