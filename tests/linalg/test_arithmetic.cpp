#include "linalg/test_arithmetic.hpp"
#include "linalg/test_la_helpers.hpp"

#include <cassert>
#include <array>

/* C interface */
#include "linalg/arithmetic.h"


/*
* Each row in this table is a matrix. Two rows form the set of input matrices
* for a single test case.
*/
static const std::vector<matrix_pair> valid_square_pairs = {
    {
        { 1, 1, { 1 } },
        { 1, 1, { 1 } }
    },
    {
        { 2, 2, { 1, 2, 0, 0 } },
        { 2, 2, { 3, 4, -1, -1 } }
    },
    {
        { 2, 2, { 1, 2, 3, 4 } },
        { 2, 2, { 1, 2, 3, 4 } }
    },
    {
        { 2, 2, { 0, 0, 0, 0 } },
        { 2, 2, { 0, 0, 0, 0 } }
    },
    {
        { 2, 2, { -1, -1, -1, -1 } },
        { 2, 2, { 1, 1, 1, 1 } }
    },
    {
        { 2, 2, { 0.5, 1.0, 1.5, 2.0 } },
        { 2, 2, { 0, 0, 0, 0 } }
    },
    {
        { 2, 2, { 0.333, 0.3333, 0.33333, 0.333333 } },
        { 2, 2, { -1, 0, 0, -1 } }
    },
    {
        { 2, 2, { -5, 3, 7, -9 } },
        { 2, 2, { 5, -8, -7, 9 } }
    },
    {
        { 3, 3, { 1, 0, 0, 0, 1, 0, 0, 0, 1 } },
        { 3, 3, { 2, 3, 4, 5, 6, 7, 8, 9, 10 } }
    },
    {
        { 3, 3, { 2, 0, 0, 0, -3, 0, 0, 0, 4 } },
        { 3, 3, { -5, 0, 0, 0, 7, 0, 0, 0, -9 } }
    },
    {
        { 3, 3, { 1, 0, 0, 0, 0, 2, 0, 0, 0 } },
        { 3, 3, { 0, 0, 3, 0, 4, 0, 5, 0, 0 } }
    },
    {
        { 2, 2, { 0.125, -0.25, 0.375, -0.5 } },
        { 2, 2, { -0.125, 0.75, -0.625, 0.5 } }
    },
    {
        { 2, 2, { 1e10, 1e-10, -1e10, -1e-10 } },
        { 2, 2, { -1e10, 2e-10, 1e10, -2e-10 } }
    },
    {
        { 4, 4, { 1, 2, 3, 4, -5, -6, -7, -8, 9, 10, 11, 12, -13, -14, -15, -16 } },
        { 4, 4, { -1, 0, 1, 2, 3, 4, 5, 6, -9, -10, -11, -12, 13, 14, 15, 16 } }
    },
    {
        { 3, 3, { 0.5, 1.25, -2.5, 3.75, -4.5, 5.125, -6.25, 7.5, 8.875 } },
        { 3, 3, { 0.5, 0.75, 2.5, -0.75, 4.5, -0.125, 6.25, -7.5, -0.875 } }
    }
    /* TODO: we need to test really large positive and negative values near DOUBLE_MAX! Think again about
    * what the intended overflow/underflow handling should be for the linalg library */
};


/* 
* Matrix-matrix addition on valid square matrices..
*/
static bool test_matrix_add(void) {

    matrix_view out_view;
    const matrixv_t *in1, *in2;
    matrixv_t *out;

    for (size_t i = 0; i < valid_square_pairs.size(); i++) {
        
        in1 = valid_square_pairs[i].in1.get_view();
        in2 = valid_square_pairs[i].in2.get_view();

        out_view.resize_view(in1->nrow, in1->ncol); 
        out = out_view.get_view();

        assert(in1->nrow == in2->nrow && in1->ncol == in2->ncol && "Invalid matrix dimensions!");
    
        ASSERT_TRUE(matrix_add(out, in1, in2) == 0); 
        ASSERT_CORRECT_ADD(out, in1, in2);
    }

    return true;
}



/* 
* Matrix-matrix addition on valid square matrices..
*/
static bool test_matrix_sub(void) {

    matrix_view out_view;
    const matrixv_t *in1, *in2;
    matrixv_t *out;

    for (size_t i = 0; i < valid_square_pairs.size(); i++) {
        
        in1 = valid_square_pairs[i].in1.get_view();
        in2 = valid_square_pairs[i].in2.get_view();

        out_view.resize_view(in1->nrow, in1->ncol); 
        out = out_view.get_view();

        assert(in1->nrow == in2->nrow && in1->ncol == in2->ncol && "Invalid matrix dimensions!");
    
        ASSERT_TRUE(matrix_sub(out, in1, in2) == 0); 
        ASSERT_CORRECT_SUB(out, in1, in2);
    }

    return true;
}


/* 
* Matrix-matrix multiplication in the order in1 * in2.
*/
static bool test_matrix_mmul(void) {

    matrix_view out_view;
    const matrixv_t *in1, *in2;
    matrixv_t *out;

    for (size_t i = 0; i < valid_square_pairs.size(); i++) {
        
        in1 = valid_square_pairs[i].in1.get_view();
        in2 = valid_square_pairs[i].in2.get_view();

        assert(in1->ncol == in2->nrow && "Invalid matrix dimensions for multiplication!");

        out_view.resize_view(in1->nrow, in2->ncol); 
        out = out_view.get_view();

        ASSERT_TRUE(matrix_mul(out, in1, in2) == 0); 
        ASSERT_CORRECT_MMUL(out, in1, in2);
    }

    return true;
}


/*
* To run a test, add it to this table, which is read by run_arithmetic_tests
* to perform all tests listed here.
*
* This style of initialization should work in the C++17+ standard
*/
static const std::array test_cases = {
    test_matrix_add,
    test_matrix_sub,
    test_matrix_mmul
};


unsigned run_arithmetic_tests(int& total) {

    /* Run all tests in `test_cases` */ 
    unsigned success_count = 0;

    for (auto test : test_cases) {
        if (test()) {
            success_count++;
        }
    }

    total = test_cases.size();

    return success_count;
}



