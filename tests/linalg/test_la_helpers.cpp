#include "linalg/test_la_helpers.hpp"
#include "Eigen_helpers.hpp"
#include <linalg/eigen/Eigen/Dense>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>


/* C inteface */
#include "linalg/view.h"
#include "linalg/scalar.h"


/*
* Initialize a view with dimensions (nrow, ncol) to default values.
*/
matrix_view::matrix_view(size_t nrow, size_t ncol) {

    /* Entries are initialized to zero */
    data_.resize(nrow * ncol);

    view_.data = data_.data();
    view_.nrow = nrow;
    view_.ncol = ncol;
    view_.row_stride = ncol;
    view_.column_stride = 1;
}


/*
* Initializes a view with dimensions (nrow, ncol) to the values in `entries`
* Accordingly, `entries` must have nrow * ncol entries.
*
* The values from `entries` are deep copied.
*/
matrix_view::matrix_view(size_t nrow, size_t ncol, const std::vector<scalar>& entries) {  

    /* Automatically performs a deep copy of the std::vector's data */
    data_ = entries;
    
    view_.data = data_.data();
    view_.nrow = nrow;
    view_.ncol = ncol;
    view_.row_stride = ncol;
    view_.column_stride = 1;
}


/*
* Resizes the internal data vector a size of `nrow` * `ncol`.
*/
void matrix_view::resize_view(size_t nrow, size_t ncol) {
        
    /* Automatically zeroes out entries */
    data_.resize(nrow * ncol);

    /* We must set view_.data again because resizing the data_ vector might reallocate
    * the matrix entries to a new address */
    view_.data = data_.data();
    view_.nrow = nrow;
    view_.ncol = ncol;
}


/* 
* Returns a mutable pointer to the internal matrixv_t struct
*/
const matrixv_t *matrix_view::get_view() const {
    return &view_;
}


/*
* Returns a mutable pointer to the internal matrixv_t struct
*/
matrixv_t *matrix_view::get_view() {
    return &view_;
}



template <typename... Args>
void perr(Args&&... args) {
    (std::cerr << ... << std::forward<Args>(args));
}


bool are_equal(double a, double b, double rel_tol, double abs_tol) {
    
    /* Handles infinity and NaN */
    if (a == b) {
        return true;
    }
    else if (std::isnan(a) || std::isnan(b)) {
        return false;
    }
    
    return std::abs(a - b) <= std::max(abs_tol, rel_tol * std::max(std::abs(a), std::abs(b)));
}


void assert_true_failed(const char *expr, const char *file, int line, const char *func) {
    perr(BOLD "%s " ANSI_RED "FAILED" ANSI_RESET "\n", func);
    perr("   Assertion failed: %s\n", expr);
    perr("   at %s:%i\n", file, line);
    perr("   in %s\n", func);
}





/* 
* Returns true if C = A + B, where A + B is performed by Eigen and 
* the result is checked against `C`
*/
bool assert_correct_add(const matrixv_t *C, const matrixv_t *A, const matrixv_t *B) {

    /* Compute A + B using Eigen */
    const Eigen::MatrixXd eigen_A = get_Eigen_matrix(A);
    const Eigen::MatrixXd eigen_B = get_Eigen_matrix(B);
    const Eigen::MatrixXd eigen_C = eigen_A + eigen_B;

    /* Check dimensions match */
    if (static_cast<size_t>(eigen_C.rows()) != C->nrow || static_cast<size_t>(eigen_C.cols()) != C->ncol) {
        return false;
    }

    /* Compare Eigen's result to `C` */
    size_t nrow = C->nrow, ncol = C->ncol, rs = C->row_stride, cs = C->column_stride;
    scalar *C_data = C->data;

    for (size_t i = 0; i < nrow; i++) {
        for (size_t j = 0; j < ncol; j++) {

            if (!are_equal(C_data[i * rs + j * cs], eigen_C(i, j))) {
                return false;
            }

        }
    }

    return true;
}



void assert_correct_add_failed(const matrixv_t *C, const matrixv_t *A, const matrixv_t *B, 
                               const char *file, int line, const char *func) {
    
    perr(BOLD "%s " ANSI_RED "FAILED" ANSI_RESET "\n", func);
    perr("Attempted to add matrices:\n");
    perr("Got:\n");
    perr("   at %s:%i\n", file, line);
    perr("   in %s\n", func);
}
