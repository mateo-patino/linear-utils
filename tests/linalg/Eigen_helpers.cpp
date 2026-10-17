#include "linalg/Eigen_helpers.hpp"
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
