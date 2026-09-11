#include "linalg/unary_operations.h"
#include "linalg/row_operations.h"


int matrix_det(scalar *out, matrixv_t *A) {
    if (!out || !A || A->nrow != A->ncol) {
        return -1;
    }

    /* Put into upper triangular form */
    int row_swaps = 0;
    int ok;
    if ((ok = to_upper_triangular(A, &row_swaps)) == -1) {
        return -1;
    }
    else if (ok == 1) {
        return 1;
    }

    /* Adjust sign for row swaps */
    scalar factor = ((row_swaps & 1) == 0) ? 1 : -1;

    /* Multiply entries along the diagonal */
    scalar diagonal_product = 1;
    size_t nrow = A->nrow, rs = A->row_stride, cs = A->column_stride;
    const scalar *data = A->data;
    for (size_t i = 0; i < nrow; i++) {
        diagonal_product *= data[i * rs + i * cs];
    }

    *out = factor * diagonal_product;

    return 0;
}



int matrix_rref(matrixv_t *A) {
    if (!A) {
        return -1;
    }
    return to_rref(A);
}
