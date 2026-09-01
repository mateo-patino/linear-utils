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
    * Matrix views will hence point to locations in the arena.
    */
    const size_t data_offset = awrite((char *)temp_data, nentry * sizeof(scalar), _Alignof(scalar), arena);
    if (data_offset == SIZE_MAX) {
        free(temp_data);
        return NULL;
    }

    /* Initialize the view in a temporary location and copy it to the arena */
    matrixv_t *temp_view = malloc(sizeof(matrixv_t));
    if (!temp_view) {
        free(temp_data);
        return NULL;
    }
    temp_view->data = (scalar *)(arena->start + data_offset);

    temp_view->ncol = (size_t)matrix->ncol;
    temp_view->nrow = (size_t)matrix->nrow;

    temp_view->column_stride = 1;
    temp_view->row_stride = 1;

    matrixv_t *out;
    size_t view_offset = awrite((char *)temp_view, sizeof(matrixv_t), _Alignof(matrixv_t), arena);
    if (view_offset == SIZE_MAX) {
        out = NULL;    
    }
    else {
        out = (matrixv_t *)(arena->start + view_offset);
    }

    free(temp_data);
    free(temp_view);
    return out;
}
