#ifndef VIEW_H
#define VIEW_H

#include <stdlib.h>
#include <limits.h>
#include <float.h>

#ifdef __cplusplus
    #define RESTRICT __restrict
#else
    #define RESTRICT restrict
#endif


/*
* Global definition of a scalar.
*/
typedef double scalar;

#define SCALAR_MAX DBL_MAX
#define SCALAR_EPSILON DBL_EPSILON
#define TOLERANCE_FACTOR 10


/*
* The core matrix structure in this library is the  matrix view
* struct `matrixv_t`. It encompasses all the information required
* to read data from and write data to a matrix's `data` array.
*/
typedef struct {
    scalar *data;
    size_t ncol;
    size_t nrow;
    size_t column_stride;
    size_t row_stride;
} matrixv_t;


/*
* Returns true if `a` and `b` are within `abs_tol` of each other.
*/
bool are_equal(scalar a, scalar b, double abs_tol);


/*
* Computes TOLERANCE_FACTOR * max_entry(A) * machine epsilon, where
* max_entry(A) is the largest entry in matrix A.
*
* The purpose of this function is to provide a matrix-dependent epsilon value
* for comparing two floating-point values.
*
* A significant limitation of this approach is that matrices with very large and
* very small entries will likely produce invalid results, as the small entries
* will likely be dectected as numerical zero since the large entries will drastically
* inflate the tolerance/epsilon used for comparisons against zero. This is not a problem
* for the vast majority of matrices where the entries are not too many orders of
* magnitude away from each other.
*
* NEEDSWORK: in the future, I would like to implement a --abs-tol flag that lets users
* control the numerical tolerance the linalg routines use for comparisons. If they know
* they will input an ill-scaled matrix, this command-line argument can probably help them
* adjust tolerances as needed.
*/
scalar get_matrix_epsilon(const matrixv_t *A);


#endif
