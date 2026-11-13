#include <math.h> 
#include "linalg/view.h"


bool are_equal(scalar a, scalar b, double abs_tol) {
    return fabs(a - b) <= abs_tol;
}


static scalar get_largest_entry(const matrixv_t *A) {
    const scalar *data = A->data;
    scalar largest = data[0];

    for (size_t i = 1; i < A->nrow * A->ncol; i++) {
        largest = data[i] > largest ? data[i] : largest;
    }

    return largest;
}

scalar get_matrix_epsilon(const matrixv_t *A) {
    if (!A || !A->data) {
        return 0;
    }

    return TOLERANCE_FACTOR * get_largest_entry(A) * SCALAR_EPSILON; 
}
