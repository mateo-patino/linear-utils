#include "matrix.h"
#include "linalg/view.h"
#include "linalg/scalar.h"

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>


void free_matrix(matrix_t *mat) {
    if (!mat) {
        return;
    }
    free(mat->data);
    free(mat);
}


matrix_t *init_matrix(scalar_t *data, unsigned int nrow, unsigned int ncol) {
    matrix_t *mat = malloc(sizeof(matrix_t));
    if (!mat) {
        return NULL;
    }
    mat->data = data;
    mat->nrow = nrow;
    mat->ncol = ncol;

    return mat;
}


bool have_equal_dimensions(const matrix_t *a, const matrix_t *b) {
    if (!a || !b) {
        return false;
    }
    return a->nrow == b->nrow && a->ncol == b->ncol;
}



matrixv_t* create_matrix_view(const matrix_t *matrix, arena_t *arena) {
    if (!matrix || !arena) {
        return NULL;
    }

    /*
    * Warn the user if lin's `scalar_t` type has a larger width than
    * linalg's `scalar` type.
    */
    if (sizeof(scalar_t) > sizeof(scalar)) {
        fprintf(stderr, "WARNING: the linear algebra engine uses floating-point types of smaller"
                        " byte size than `lin`. Loss of information is likely.\n");
    }

    /*
    * Copy the matrix data to a temporary location and then write to the memory arena 
    */
    size_t nentry = matrix->ncol * matrix->nrow;
    scalar *temp_data = malloc(nentry * sizeof(scalar));
    if (!temp_data) {
        return NULL;
    }
    for (size_t i = 0; i < nentry; i++) {
        temp_data[i] = (scalar)matrix->data[i];
    }

    /* 
    * Write the array of `scalar` data to the memory arena. 
    * Matrix views' data will hence point to locations in the arena.
    */
    const size_t data_offset = awrite((char *)temp_data, nentry * sizeof(scalar), _Alignof(scalar), arena);
    if (data_offset == SIZE_MAX) {
        free(temp_data);
        return NULL;
    }

    /* Initialize the view in a temporary location and copy it to the arena */
    matrixv_t view;
    matrixv_t *temp_view = &view;

    temp_view->data = (scalar *)(arena->start + data_offset);

    temp_view->ncol = (size_t)matrix->ncol;
    temp_view->nrow = (size_t)matrix->nrow;

    temp_view->row_stride = temp_view->ncol;
    temp_view->column_stride = 1;

    matrixv_t *out;
    size_t view_offset = awrite((char *)temp_view, sizeof(matrixv_t), _Alignof(matrixv_t), arena);
    if (view_offset == SIZE_MAX) {
        out = NULL;    
    }
    else {
        out = (matrixv_t *)(arena->start + view_offset);
    }

    free(temp_data);
    return out;
}



matrix_t *init_matrix_token_from_view(const matrixv_t *view) {
    if (!view) {
        return NULL;
    }

    /* Allocate memory for the scalar data */
    size_t nrow = view->nrow, ncol = view->ncol;
    scalar_t *new_data = malloc(nrow * ncol * sizeof(scalar_t));
    if (!new_data) {
        return NULL;
    }

    /* 
    * Warn the user if scalar_t has smaller bit width than linalg's scalar.
    * NOTE: this warning (and the other in token.c) will likely need to change if
    * you modify the scalar_t struct 
    */
    if (sizeof(scalar_t) < sizeof(scalar)) {
        fprintf(stderr, "WARNING: the linear algebra engine uses floating-point types of larger"
                        " byte size than `lin`. Loss of information is likely.\n");

    }

    scalar *data = view->data;
    size_t k = 0, rs = view->row_stride, cs = view->column_stride;
    for (size_t i = 0; i < nrow; i++) {
        for (size_t j = 0; j < ncol; j++) {
            new_data[k++] = (scalar_t)data[i * rs + j * cs];
        }
    }

    /* Allocate new matrix_t struct */
    matrix_t *out = malloc(sizeof(matrix_t));
    if (!out) {
        free(new_data);
        return NULL;
    }

    out->data = new_data;
    out->nrow = nrow;
    out->ncol = ncol;

    return out;
}


scalar_t *init_scalar_token_from_linalg_scalar(const scalar *scalar) {
    if (!scalar) {
        return NULL;
    }

    /* Allocate memory for the scalar_t */
    scalar_t *out = malloc(sizeof(scalar_t));
    if (!out) {
        return NULL;
    }

    /* 
    * Warn the user if scalar_t has smaller bit width than linalg's scalar.
    * NOTE: this warning (and the other in token.c) will likely need to change if
    * you modify the scalar_t struct 
    */
    if (sizeof(scalar_t) < sizeof(scalar))
        fprintf(stderr, "WARNING: the linear algebra engine uses floating-point types of larger"
                        " byte size than `lin`. Loss of information is likely.\n");

    *out = *scalar;

    return out;
}
