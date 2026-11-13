#include "linalg/test_la_helpers.hpp"
#include "Eigen_helpers.hpp"
#include <linalg/eigen/Eigen/Dense>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <limits>


/* C inteface */
#include "linalg/view.h"
        

/*
* Copy constructor
*
* Note that this performs a deep copy because "data_(other.data_)" runs the
* std::vector copy constructor, which will allocate memory for the matrix's
* entries in completely new memory address. Hence, the copy and the orignal
* object will not share the same scalar data in memory.
*/
matrix_view::matrix_view(const matrix_view& other) 
    : view_(other.view_), data_(other.data_) {
    
    view_.data = data_.data();
}


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
    view_.row_stride = ncol;
    view_.column_stride = 1;
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


bool are_equal_det(scalar detval, const matrixv_t *A, scalar target) {
    if (!A) {
        return false;
    }
    else if (detval == target) {
        return true;
    }

    /* 
    * Compute product of the norm of each column. This is the upper bound
    * on det(A). See Hadamard's inequality
    */
    scalar *A_data = A->data;
    size_t nrow = A->nrow, ncol = A->ncol, rs = A->row_stride, cs = A->column_stride;
    scalar scale = 1, norm = 0;
    bool scale_overflow = false;

    for (size_t j = 0; j < ncol; j++) {
        norm = 0; 
        for (size_t i = 0; i < nrow; i++) {
            /* std::hypot internally scales inputs to reduce the risk of overflow */
            norm = std::hypot(norm, A_data[i*rs + j*cs]);
        }

        /* Avoid allowing the Hadamard scale to become infinity. */
        if (!std::isfinite(norm) || (norm != 0 && scale > std::numeric_limits<scalar>::max() / norm)) {
            scale_overflow = true;
            break;
        }
        scale *= norm;
    }

    /* Use the determinant values as a finite fallback scale. */
    if (scale_overflow) {
        scale = std::max(std::abs(detval), std::abs(target));
        if (!std::isfinite(scale)) {
            return false;
        }
        scale = std::max<scalar>(1, scale);
    }

    scalar abs_tol = 10.0 * std::numeric_limits<double>::epsilon() * scale;

    return std::abs(detval - target) <= abs_tol;
}


void assert_true_failed(const char *expr, const char *file, int line, const char *func) {
    perr(BOLD, func, ANSI_RED, " FAILED", ANSI_RESET, "\n");
    perr(PAD, "Assertion failed: ", expr, "\n");
    perr(PAD, "at ", file, ":", line, "\n");
    perr(PAD, "in ", func, "\n");
}


void print_success(const test_case& test_case) {
    perr(BOLD, test_case.name(), " ", ANSI_RESET, ANSI_GREEN, "PASS\n", ANSI_RESET);
}


void pretty_print_matrix(std::ostream& stream, const matrixv_t *matrix, int wspace) {
    if (!matrix || !matrix->data) {
        std::cerr << "Error: NULL pointer to matrixv_t\n";
        return;
    }

    /* Get the string length of a scalar `x` */
    auto get_scalar_strlen = [](scalar x) {
        /* NOTE: ostringstream represents floating-point values with 6 decimal places */
        std::ostringstream ss;
        ss << x;
        return ss.str().length();
    }; 


    /* Compute column widths */
    auto get_column_width = [&get_scalar_strlen](const matrixv_t *mat, size_t col) {

        size_t rs = mat->row_stride, cs = mat->column_stride;
        std::size_t curr_strlen = 0, max_strlen = get_scalar_strlen(mat->data[0 * rs + col * cs]);

        for (size_t i = 1; i < mat->nrow; i++) {
            curr_strlen = get_scalar_strlen(mat->data[i * rs + col * cs]);
            max_strlen = curr_strlen > max_strlen ? curr_strlen : max_strlen;
        }

        return max_strlen;
    };

    std::vector<std::size_t> col_widths;
    col_widths.reserve(matrix->ncol);
    for (size_t j = 0; j < matrix->ncol; j++) {
        col_widths.push_back(get_column_width(matrix, j));
    }

    /* Print matrix */
    const scalar *data = matrix->data;
    size_t nrow = matrix->nrow, ncol = matrix->ncol, rs = matrix->row_stride, cs = matrix->column_stride;

    for (size_t i = 0; i < nrow; i++) {

        stream << std::string(wspace, ' ');
        stream << "| ";

        for (size_t j = 0; j < ncol; j++) {
            stream << std::setw(col_widths[j]) << data[i * rs + j * cs] << " "; 
        }

        stream << "|\n";
    }
}


void pretty_print_matrix(std::ostream& stream, const Eigen::MatrixXd& matrix, int wspace) {
    
    /* Get the string length of a scalar `x` */
    auto get_scalar_strlen = [](double x) {
        /* NOTE: ostringstream represents floating-point values with 6 decimal places */
        std::ostringstream ss;
        ss << x;
        return ss.str().length();
    };


    /* Compute column width of column `col` */
    auto get_column_width = [&get_scalar_strlen](const Eigen::MatrixXd& matrix, Eigen::Index col) {
        std::size_t curr_strlen = 0, max_strlen = get_scalar_strlen(matrix(0, col));

        for (Eigen::Index i = 1; i < matrix.rows(); i++) {
            curr_strlen = get_scalar_strlen(matrix(i, col));
            max_strlen = curr_strlen > max_strlen ? curr_strlen : max_strlen;
        }

        return max_strlen;
    };

    std::vector<std::size_t> col_widths;
    col_widths.reserve(matrix.cols());
    for (Eigen::Index j = 0; j < matrix.cols(); j++) {
        col_widths.push_back(get_column_width(matrix, j));
    }

    /* Print matrix */
    for (Eigen::Index i = 0, rows = matrix.rows(); i < rows; i++) {

        stream << std::string(wspace, ' ');
        stream << "| ";

        for (Eigen::Index j = 0, cols = matrix.cols(); j < cols; j++) {
            stream << std::setw(col_widths[j]) << matrix(i, j) << " ";
        }

        stream << "|\n";
    }
}


matrix_view init_identity(std::size_t n) {

    /* Entries initialized to zero */
    matrix_view out(n, n);
    matrixv_t *view = out.get_view();
    size_t rs = view->row_stride;

    for (std::size_t i = 0; i < n; i++) {
        view->data[i*(rs + 1)] = 1;
    }

    return out;
}


/* 
* Returns true if C = A + B, where A + B is performed by Eigen and 
* the result is checked against `C` if `subtract` is false. Same operation
* is performed but with C = A - B if `subtract` is true.
*
* `expected` is an in-out parameter where the expected sum according to Eigen is written.
*/
bool assert_correct_add(const matrixv_t *C, const matrixv_t *A, const matrixv_t *B, Eigen::MatrixXd& expected, bool subtract) {
    if (!C || !A || !B) {
        std::cerr << "Error: NULL matrix view!\n";
        return false;
    } 


    /* Compute A + B using Eigen */
    const Eigen::MatrixXd eigen_A = get_Eigen_matrix(A);
    const Eigen::MatrixXd eigen_B = get_Eigen_matrix(B);
    Eigen::MatrixXd eigen_C;

    if (subtract) {
        eigen_C = eigen_A - eigen_B;
    }
    else {
        eigen_C = eigen_A + eigen_B;
    }
    expected = eigen_C;
    
    return are_equal_matrices(eigen_C, C);
}


/*
* Prints an error message for a failed addition (or subtraction) test case
*/
void assert_correct_add_failed(const matrixv_t *C, const matrixv_t *A, const matrixv_t *B, 
                               const char *file, int line, const char *func, const Eigen::MatrixXd& expected, bool subtract) {

    std::string operation = subtract ? "SUB" : "ADD";
    
    perr(BOLD, func, " ", ANSI_RED, "FAILED", ANSI_RESET, "\n");
    perr("\n", PAD, "Attempted to ", operation, " matrices:\n");
    perr("\n", PAD, "A =\n");
    pretty_print_matrix(std::cerr, A, sizeof(PAD) - 1);
    perr("\n", PAD, "B = \n");
    pretty_print_matrix(std::cerr, B, sizeof(PAD) - 1);
    perr("\n", PAD, "Got:\n");
    pretty_print_matrix(std::cerr, C, sizeof(PAD) - 1);
    perr("\n", PAD, "Expected:\n");
    pretty_print_matrix(std::cerr, expected, sizeof(PAD) - 1);
    perr("\n", PAD, "at ", file, ":", line, "\n");
    perr(PAD, "in ", func, "\n");
}


bool assert_correct_mmul(const matrixv_t *C, const matrixv_t *A, const matrixv_t *B, Eigen::MatrixXd& expected) {
    if (!C || !A || !B) {
        std::cerr << "Error: NULL matrix view!\n";
        return false;
    } 

    /* Compute AB using Eigen */
    const Eigen::MatrixXd eigen_A = get_Eigen_matrix(A);
    const Eigen::MatrixXd eigen_B = get_Eigen_matrix(B);
    const Eigen::MatrixXd eigen_C = eigen_A * eigen_B;

    expected = eigen_C;

    return are_equal_matrices(eigen_C, C);
}


/*
* Prints an error message for a failed matrix multiplication test case
*/
void assert_correct_mmul_failed(const matrixv_t *C, const matrixv_t *A, const matrixv_t *B,
                                const char *file, int line, const char *func, const Eigen::MatrixXd& expected) {

    perr(BOLD, func, " ", ANSI_RED, "FAILED", ANSI_RESET, "\n");
    perr("\n", PAD, "Attempted to multiply matrices:\n");
    perr("\n", PAD, "A =\n");
    pretty_print_matrix(std::cerr, A, sizeof(PAD) - 1);
    perr("\n", PAD, "B = \n");
    pretty_print_matrix(std::cerr, B, sizeof(PAD) - 1);
    perr("\n", PAD, "Got:\n");
    pretty_print_matrix(std::cerr, C, sizeof(PAD) - 1);
    perr("\n", PAD, "Expected:\n");
    pretty_print_matrix(std::cerr, expected, sizeof(PAD) - 1);
    perr("\n", PAD, "at ", file, ":", line, "\n");
    perr(PAD, "in ", func, "\n");
}


bool assert_correct_smul(const matrixv_t *C, scalar s, const matrixv_t *A, Eigen::MatrixXd& expected) {
    if (!C || !A) {
        std::cerr << "Error: NULL matrix view!\n";
        return false;
    }

    /* Compute s * A */
    const Eigen::MatrixXd eigen_A = get_Eigen_matrix(A);
    const Eigen::MatrixXd eigen_C = static_cast<double>(s) * eigen_A;
    expected = eigen_C;

    return are_equal_matrices(eigen_C, C); 
}


void assert_correct_smul_failed(const matrixv_t *C, scalar s, const matrixv_t *A, const char *file, int line, 
                                const char *func, const Eigen::MatrixXd& expected) {
    
    perr(BOLD, func, " ", ANSI_RED, "FAILED", ANSI_RESET, "\n");
    perr("\n", PAD, "Attempted to multiply matrix:\n");
    perr("\n", PAD, "A =\n");
    pretty_print_matrix(std::cerr, A, sizeof(PAD) - 1);
    perr("\n", PAD, "by scalar: ", s, "\n");
    perr("\n", PAD, "Got:\n");
    pretty_print_matrix(std::cerr, C, sizeof(PAD) - 1);
    perr("\n", PAD, "Expected:\n");
    pretty_print_matrix(std::cerr, expected, sizeof(PAD) - 1);
    perr("\n", PAD, "at ", file, ":", line, "\n");
    perr(PAD, "in ", func, "\n");

}


bool assert_correct_det(scalar detval, const matrixv_t *A, double& expected) {
    if (!A) {
        std::cerr << "Error: NULL matrix view!\n";
        return false;
    }
    else if (A->nrow != A->ncol) {
        std::cerr << "Error: non-square matrix, expected square matrix!\n";
        return false;
    }
    
    const Eigen::MatrixXd eigen_A = get_Eigen_matrix(A);
    const double eigen_detval = eigen_A.determinant();
    expected = eigen_detval;

    /*
    * Note that we don't use are_equal to check that the determinants
    * are the same. Instead call are_equal_det which is more numerically
    * robust for comparing a determinant to some target value.
    */
    return are_equal_det(detval, A, static_cast<scalar>(eigen_detval));
}   


void assert_correct_det_failed(scalar detval, const matrixv_t *A, const char *file, int line, const char *func, 
                               double expected) {
    
    perr(BOLD, func, " ", ANSI_RED, "FAILED", ANSI_RESET, "\n");
    perr("\n", PAD, "Attempted to compute determinant of matrix:\n");
    perr("\n", PAD, "A =\n");
    pretty_print_matrix(std::cerr, A, sizeof(PAD) - 1);
    perr("\n", PAD, "Got: ", detval, "\n");
    perr("\n", PAD, "Expected: ", expected, "\n");
    perr("\n", PAD, "at ", file, ":", line, "\n");
    perr(PAD, "in ", func, "\n");

}


bool assert_correct_rref(const matrixv_t *rref_A, const matrixv_t *expected) {
    if (!rref_A || !expected) {
        std::cerr << "Error: NULL matrix view!\n";
        return false;
    }

    return are_equal_matrices(rref_A, expected);
}


void assert_correct_rref_failed(const matrixv_t *rref_A, const matrixv_t *A, const char *file, int line, const char *func, const matrixv_t *expected) {

    perr(BOLD, func, " ", ANSI_RED, "FAILED", ANSI_RESET, "\n");
    perr("\n", PAD, "Attempted to compute RREF of matrix:\n");
    perr("\n", PAD, "A =\n");
    pretty_print_matrix(std::cerr, A, sizeof(PAD) - 1);
    perr("\n", PAD, "Got:\n");
    pretty_print_matrix(std::cerr, rref_A, sizeof(PAD) - 1);
    perr("\n", PAD, "Expected:\n");
    pretty_print_matrix(std::cerr, expected, sizeof(PAD) - 1);
    perr("\n", PAD, "at ", file, ":", line, "\n");
    perr(PAD, "in ", func, "\n");
}
