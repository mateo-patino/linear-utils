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
static const std::vector<matrix_pair> valid_square = {
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
* Row-vector pairs used to exercise non-square matrices. The table includes
* zero entries, signed values, fractions, repeated entries, and values with
* large and small magnitudes that remain safely below overflow thresholds.
*/
static const std::vector<matrix_pair> valid_row = {
    {
        { 1, 1, { 0 } },
        { 1, 1, { -0.0 } }
    },
    {
        { 1, 2, { 1, -1 } },
        { 1, 2, { -1, 1 } }
    },
    {
        { 1, 3, { 1, 2, 3 } },
        { 1, 3, { -3, 0, 3 } }
    },
    {
        { 1, 4, { 0.25, -0.5, 0.75, -1.0 } },
        { 1, 4, { -0.25, 0.5, -0.75, 1.0 } }
    },
    {
        { 1, 5, { 1, 1, 1, 1, 1 } },
        { 1, 5, { 0, 0, 0, 0, 0 } }
    },
    {
        { 1, 6, { 1e-10, -1e-10, 1e-9, -1e-9, 1e-8, -1e-8 } },
        { 1, 6, { -1e-10, 1e-10, -1e-9, 1e-9, -1e-8, 1e-8 } }
    },
    {
        { 1, 7, { 10, -20, 30, -40, 50, -60, 70 } },
        { 1, 7, { -7, 6, -5, 4, -3, 2, -1 } }
    },
    {
        { 1, 8, { 1, 0, 0, 0, 0, 0, 0, 1 } },
        { 1, 8, { 0, 1, 0, 0, 0, 0, 1, 0 } }
    },
    {
        { 1, 9, { 1e10, -1e10, 1e10, -1e10, 1e-10, -1e-10, 1e-10, -1e-10, 0 } },
        { 1, 9, { -1e10, 1e10, -1e10, 1e10, -1e-10, 1e-10, -1e-10, 1e-10, 0 } }
    },
    {
        { 1, 10, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } },
        { 1, 10, { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 } }
    },
    {
        { 1, 3, { 1e100, -1e100, 1e-100 } },
        { 1, 3, { -1e100, 1e100, -1e-100 } }
    },
    {
        { 1, 4, { 0.333333333333, 0.666666666667, -0.333333333333, -0.666666666667 } },
        { 1, 4, { 3, -3, 6, -6 } }
    },
    {
        { 1, 5, { -5, -4, -3, -2, -1 } },
        { 1, 5, { 1, 2, 3, 4, 5 } }
    },
    {
        { 1, 8, { 0, 0, 0, 1, 0, 0, 0, 0 } },
        { 1, 8, { 0, 0, 0, 0, 0, 0, 0, 1 } }
    },
    {
        { 1, 10, { 2.5, -3.5, 4.5, -5.5, 6.5, -7.5, 8.5, -9.5, 10.5, -11.5 } },
        { 1, 10, { -11.5, 10.5, -9.5, 8.5, -7.5, 6.5, -5.5, 4.5, -3.5, 2.5 } }
    }
};


/*
* Column-vector pairs used to exercise non-square matrices. The cases mirror
* the row-vector coverage while retaining independent data and edge values.
*/
static const std::vector<matrix_pair> valid_column = {
    {
        { 1, 1, { 0 } },
        { 1, 1, { -0.0 } }
    },
    {
        { 2, 1, { 1, -1 } },
        { 2, 1, { -1, 1 } }
    },
    {
        { 3, 1, { 1, 2, 3 } },
        { 3, 1, { -3, 0, 3 } }
    },
    {
        { 4, 1, { 0.25, -0.5, 0.75, -1.0 } },
        { 4, 1, { -0.25, 0.5, -0.75, 1.0 } }
    },
    {
        { 5, 1, { 1, 1, 1, 1, 1 } },
        { 5, 1, { 0, 0, 0, 0, 0 } }
    },
    {
        { 6, 1, { 1e-10, -1e-10, 1e-9, -1e-9, 1e-8, -1e-8 } },
        { 6, 1, { -1e-10, 1e-10, -1e-9, 1e-9, -1e-8, 1e-8 } }
    },
    {
        { 7, 1, { 10, -20, 30, -40, 50, -60, 70 } },
        { 7, 1, { -7, 6, -5, 4, -3, 2, -1 } }
    },
    {
        { 8, 1, { 1, 0, 0, 0, 0, 0, 0, 1 } },
        { 8, 1, { 0, 1, 0, 0, 0, 0, 1, 0 } }
    },
    {
        { 9, 1, { 1e10, -1e10, 1e10, -1e10, 1e-10, -1e-10, 1e-10, -1e-10, 0 } },
        { 9, 1, { -1e10, 1e10, -1e10, 1e10, -1e-10, 1e-10, -1e-10, 1e-10, 0 } }
    },
    {
        { 10, 1, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } },
        { 10, 1, { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 } }
    },
    {
        { 3, 1, { 1e100, -1e100, 1e-100 } },
        { 3, 1, { -1e100, 1e100, -1e-100 } }
    },
    {
        { 4, 1, { 0.333333333333, 0.666666666667, -0.333333333333, -0.666666666667 } },
        { 4, 1, { 3, -3, 6, -6 } }
    },
    {
        { 5, 1, { -5, -4, -3, -2, -1 } },
        { 5, 1, { 1, 2, 3, 4, 5 } }
    },
    {
        { 8, 1, { 0, 0, 0, 1, 0, 0, 0, 0 } },
        { 8, 1, { 0, 0, 0, 0, 0, 0, 0, 1 } }
    },
    {
        { 10, 1, { 2.5, -3.5, 4.5, -5.5, 6.5, -7.5, 8.5, -9.5, 10.5, -11.5 } },
        { 10, 1, { -11.5, 10.5, -9.5, 8.5, -7.5, 6.5, -5.5, 4.5, -3.5, 2.5 } }
    }
};


/* 
* Matrix-matrix addition on valid square matrices..
*/
static bool test_matrix_add_square(void) {

    matrix_view out_view;
    const matrixv_t *in1, *in2;
    matrixv_t *out;

    for (size_t i = 0; i < valid_square.size(); i++) {
        
        in1 = valid_square[i].in1.get_view();
        in2 = valid_square[i].in2.get_view();

        out_view.resize_view(in1->nrow, in1->ncol); 
        out = out_view.get_view();

        assert(in1->nrow == in2->nrow && in1->ncol == in2->ncol && "Invalid matrix dimensions!");
    
        ASSERT_TRUE(matrix_add(out, in1, in2) == 0); 
        ASSERT_CORRECT_ADD(out, in1, in2);
    }

    return true;
}


/*
* Matrix-matrix addition on valid row vectors.
*/
static bool test_matrix_add_row_vec(void) {

    matrix_view out_view;
    const matrixv_t *in1, *in2;
    matrixv_t *out;

    for (size_t i = 0; i < valid_row.size(); i++) {

        in1 = valid_row[i].in1.get_view();
        in2 = valid_row[i].in2.get_view();

        out_view.resize_view(in1->nrow, in1->ncol);
        out = out_view.get_view();

        assert(in1->nrow == in2->nrow && in1->ncol == in2->ncol && "Invalid matrix dimensions!");

        ASSERT_TRUE(matrix_add(out, in1, in2) == 0);
        ASSERT_CORRECT_ADD(out, in1, in2);
    }

    return true;
}


/*
* Matrix-matrix addition on valid column vectors.
*/
static bool test_matrix_add_col_vec(void) {

    matrix_view out_view;
    const matrixv_t *in1, *in2;
    matrixv_t *out;

    for (size_t i = 0; i < valid_column.size(); i++) {

        in1 = valid_column[i].in1.get_view();
        in2 = valid_column[i].in2.get_view();

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
static bool test_matrix_sub_square(void) {

    matrix_view out_view;
    const matrixv_t *in1, *in2;
    matrixv_t *out;

    for (size_t i = 0; i < valid_square.size(); i++) {
        
        in1 = valid_square[i].in1.get_view();
        in2 = valid_square[i].in2.get_view();

        out_view.resize_view(in1->nrow, in1->ncol); 
        out = out_view.get_view();

        assert(in1->nrow == in2->nrow && in1->ncol == in2->ncol && "Invalid matrix dimensions!");
    
        ASSERT_TRUE(matrix_sub(out, in1, in2) == 0); 
        ASSERT_CORRECT_SUB(out, in1, in2);
    }

    return true;
}


/*
* Matrix-matrix subtraction on valid row vectors.
*/
static bool test_matrix_sub_row_vec(void) {

    matrix_view out_view;
    const matrixv_t *in1, *in2;
    matrixv_t *out;

    for (size_t i = 0; i < valid_row.size(); i++) {

        in1 = valid_row[i].in1.get_view();
        in2 = valid_row[i].in2.get_view();

        out_view.resize_view(in1->nrow, in1->ncol);
        out = out_view.get_view();

        assert(in1->nrow == in2->nrow && in1->ncol == in2->ncol && "Invalid matrix dimensions!");

        ASSERT_TRUE(matrix_sub(out, in1, in2) == 0);
        ASSERT_CORRECT_SUB(out, in1, in2);
    }

    return true;
}


/*
* Matrix-matrix subtraction on valid column vectors.
*/
static bool test_matrix_sub_col_vec(void) {

    matrix_view out_view;
    const matrixv_t *in1, *in2;
    matrixv_t *out;

    for (size_t i = 0; i < valid_column.size(); i++) {

        in1 = valid_column[i].in1.get_view();
        in2 = valid_column[i].in2.get_view();

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

    for (size_t i = 0; i < valid_square.size(); i++) {
        
        in1 = valid_square[i].in1.get_view();
        in2 = valid_square[i].in2.get_view();

        assert(in1->ncol == in2->nrow && "Invalid matrix dimensions for multiplication!");

        out_view.resize_view(in1->nrow, in2->ncol); 
        out = out_view.get_view();

        ASSERT_TRUE(matrix_mul(out, in1, in2) == 0); 
        ASSERT_CORRECT_MMUL(out, in1, in2);
    }

    return true;
}



/* 
* Scalar-matrix multiplication.
*/
static bool test_matrix_smul(void) {

    matrix_view out_view;
    const matrixv_t *in_mat;
    matrixv_t *out;

    std::vector<scalar> scalars = {
        0.0, 1.0, -1.0, 2.0, -2.0,
        0.5, -0.5, 3.14, -3.14, 10.0,
        -10.0, 100.0, -100.0, 1.5, -1.5,
        0.001, -0.001, 42.0, -42.0, 7.25,
        -1e12, 1e-12, 1e12
    };

    /* We'll iterate over all matrices in valid_square_pairs for now */
    std::array<const matrix_view *, 2> matrices;
    for (size_t i = 0; i < valid_square.size(); i++) {
        matrices = std::array<const matrix_view *, 2>{ &valid_square[i].in1, &valid_square[i].in2 };

        for (size_t j = 0; j < 2; j++) {
            in_mat = matrices[j]->get_view();
            out_view.resize_view(in_mat->nrow, in_mat->ncol); 
            out = out_view.get_view();

            /* Multiply the current matrix by all scalars in the vector above */
            for ( scalar s : scalars ) {
                ASSERT_TRUE(scalar_matrix_mul(out, s, in_mat) == 0); 
                ASSERT_CORRECT_SMUL(out, s, in_mat);
            }
        }
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
    TEST(test_matrix_add_square),
    TEST(test_matrix_add_row_vec),
    TEST(test_matrix_add_col_vec),
    TEST(test_matrix_sub_square),
    TEST(test_matrix_sub_row_vec),
    TEST(test_matrix_sub_col_vec),
    TEST(test_matrix_mmul),
    TEST(test_matrix_smul)
};


unsigned run_arithmetic_tests(int& total) {

    /* Run all tests in `test_cases` */ 
    unsigned success_count = 0;

    for (const test_case& test : test_cases) {
        if (test.run()) {
            print_success(test);
            success_count++;
        }
    }

    total = test_cases.size();

    return success_count;
}



