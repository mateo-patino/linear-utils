#include <math.h> 
#include "linalg/view.h"


bool are_equal(scalar a, scalar b, double abs_tol) {
    return fabs(a - b) <= abs_tol;
}


static scalar get_largest_entry(const matrixv_t *A) {
    size_t nrow = A->nrow, ncol = A->ncol, rs = A->row_stride, cs = A->column_stride;
    const scalar *data = A->data;

    scalar largest = 0, entry = 0;

    for (size_t i = 0; i < nrow; i++) {
        for (size_t j = 0; j < ncol; j++) {
            entry = fabs(data[i*rs + j*cs]);
            largest = entry > largest ? entry : largest; 
        }
    }

    return largest;
}

scalar get_matrix_epsilon(const matrixv_t *A) {
    if (!A || !A->data) {
        return 0;
    }

    return TOLERANCE_FACTOR * get_largest_entry(A) * SCALAR_EPSILON; 
}
