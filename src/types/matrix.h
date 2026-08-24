#ifndef MATRIX_H
#define MATRIX_H

#include <stdbool.h>
#include <float.h>

/* Scalar type */
typedef double scalar_t;
#define PRISCALAR "%.*f"
#define SCALAR_PRECISION 3
#define SCALAR_MAX DBL_MAX

/* Matrix interface... */
typedef struct {
    unsigned int nrow;
    unsigned int ncol;
    scalar_t *data;
} matrix_t;


typedef enum {
    MATRIX_OK,
    MATRIX_INVALID_ENTRY
} matrix_status;


/*
* Frees mat->data and mat pointers.
*/
void free_matrix(matrix_t *mat);

/*
* Parametrized constructor for matrix_t. Returns pointer to 
* heap-allocated matrix_t and NULL upon failure.
*/
matrix_t *init_matrix(scalar_t *data, unsigned int nrow, unsigned int ncol);


/*
* Returns true if `a` and `b` have the same dimensions
*/
bool have_equal_dimensions(const matrix_t *a, const matrix_t *b);

#endif
