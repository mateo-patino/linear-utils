#ifndef TEST_LA_HELPERS_HPP
#define TEST_LA_HELPERS_HPP

/* C interfaces */
#include "linalg/view.h"
#include "linalg/scalar.h"

#include <vector>


/* ANSI COLORS */
#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_CYAN "\x1b[36m"
#define ANSI_RESET "\x1b[0m"
#define BOLD "\x1b[1m"


/*
* This class is a small wrapper around the C matrix view (matrixv_t) struct.
* 
* The matrixv_t struct is stored in each class object, so it does not need
* to be freed manually. The internal scalar *data pointer of each struct is managed
* with a std::vector that frees this pointer automatically when the wrapper's
* destructor runs, so the view's data doesn't need to be manually freed.
*/
class matrix_view {

    private:
        matrixv_t view_;
        std::vector<scalar> data_;

    public:

        matrix_view(size_t nrow, size_t ncol);
        matrix_view(size_t nrow, size_t ncol, const std::vector<scalar>& entries);

        void resize_view(size_t nrow, size_t ncol);

        const matrixv_t *get_view() const;
        matrixv_t *get_view();
};


/* 
* Struct to conveniently hold two operand matrix views.
* It is widely used to define tables of matrix_pair structs
* and then run a set of tests on all matrix pairs on the table,
* where each pair represents the operands for some matrix operation.
*/
struct matrix_pair {
    matrix_view in1;
    matrix_view in2;
};


/* 
* Calls fprintf(stderr, fmt, ...). Used to print error messages when a test fails.
*/
void perr(const char *fmt, ...);


/*
* Returns true if two floating-point doubles `a` and `b` are equal to each other.
*/
bool are_equal(double a, double b, double rel_tol = 1e-12, double abs_tol = 1e-15);


/*
* This test suite uses macros to assert conditions.
*
* The macros below perform various condition checks. Their names 
* should be self-explanatory.
*/
#define ASSERT_TRUE(x) \
    do { \
        if (!(x)) { \
            assert_true_failed(#x, __FILE__, __LINE__, __func__); \
            return false; \
        } \
    } while (0)

void assert_true_failed(const char *expr, const char *file, int line, const char *func);

/* For 3 matrixv_t matrices, check that C = A + B. */
#define ASSERT_CORRECT_ADD(C, A, B) \
    do { \
        const matrixv_t *_C = (const matrixv_t *)C; \
        const matrixv_t *_A = (const matrixv_t *)A; \
        const matrixv_t *_B = (const matrixv_t *)B; \
        if (!assert_correct_add(_C, _A, _B)) { \
            assert_correct_add_failed(_C, _A, _B, __FILE__, __LINE__, __func__); \
            return false; \
        } \
    } while(0)

/* 
* Returns true if C = A + B, where A + B is performed by Eigen and 
* the result is checked against `C`
*/
bool assert_correct_add(const matrixv_t *C, const matrixv_t *A, const matrixv_t *B);
void assert_correct_add_failed(const matrixv_t *C, const matrixv_t *A, const matrixv_t *B, 
                               const char *file, int line, const char *func);


#endif
