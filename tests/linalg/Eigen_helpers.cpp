#include "linalg/Eigen_helpers.hpp"
#include "linalg/test_la_helpers.hpp"


#include <stdexcept>

Eigen::MatrixXd get_Eigen_matrix(const matrixv_t *matrix) {
    if (!matrix) {
        throw std::invalid_argument("Got NULL matrixv_t pointer!");
    }
    
    size_t nrow = matrix->nrow, ncol = matrix->ncol, rs = matrix->row_stride, cs = matrix->column_stride;
    scalar *data = matrix->data;

    Eigen::MatrixXd out(nrow, ncol);

    for (size_t i = 0; i < nrow; i++) {
        for (size_t j = 0; j < ncol; j++) {
            out(i, j) = data[i * rs + j * cs];
        }
    }

    return out;
}


bool are_equal_matrices(const Eigen::MatrixXd& A, const matrixv_t *B) {

    if (!B) {
        return false;
    }

    /* Check dimensions match */
    if (static_cast<size_t>(A.rows()) != B->nrow || static_cast<size_t>(A.cols()) != B->ncol) {
        return false;
    }

    /* Compare Eigen's result to `C` */
    size_t nrow = B->nrow, ncol = B->ncol, rs = B->row_stride, cs = B->column_stride;
    scalar *B_data = B->data;

    for (size_t i = 0; i < nrow; i++) {
        for (size_t j = 0; j < ncol; j++) {

            if (!are_equal(B_data[i * rs + j * cs], A(i, j))) {
                return false;
            }

        }
    }

    return true;
}


bool are_equal_matrices(const matrixv_t *A, const matrixv_t *B) {

    if (!A || !B) {
        return false;
    }

    /* Check dimensions match */
    if (A->nrow != B->nrow || A->ncol != B->ncol) {
        return false;
    }

    /* Compare the entries of `A` and `B` */
    size_t nrow = A->nrow, ncol = A->ncol;
    size_t A_rs = A->row_stride, A_cs = A->column_stride;
    size_t B_rs = B->row_stride, B_cs = B->column_stride;
    scalar *A_data = A->data;
    scalar *B_data = B->data;

    for (size_t i = 0; i < nrow; i++) {
        for (size_t j = 0; j < ncol; j++) {

            if (!are_equal(A_data[i * A_rs + j * A_cs],
                           B_data[i * B_rs + j * B_cs])) {
                return false;
            }

        }
    }

    return true;
}
