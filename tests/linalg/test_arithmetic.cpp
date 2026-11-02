#include "linalg/test_arithmetic.hpp"
#include "linalg/test_la_helpers.hpp"
#include "linalg/test_inputs.hpp"

#include <cassert>
#include <array>

/* C interface */
#include "linalg/arithmetic.h"



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
static bool test_matrix_mmul_square(void) {

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
* Matrix-matrix multiplication between valid row and column vectors.
*/
static bool test_matrix_mmul_row_vec(void) {

    matrix_view out_view;
    const matrixv_t *in1, *in2;
    matrixv_t *out;

    for (size_t i = 0; i < valid_row_col.size(); i++) {

        in1 = valid_row_col[i].in1.get_view();
        in2 = valid_row_col[i].in2.get_view();

        assert(in1->ncol == in2->nrow && "Invalid matrix dimensions for multiplication!");

        out_view.resize_view(in1->nrow, in2->ncol);
        out = out_view.get_view();

        ASSERT_TRUE(matrix_mul(out, in1, in2) == 0);
        ASSERT_CORRECT_MMUL(out, in1, in2);
    }

    return true;
}



/* 
* Scalar-matrix multiplication on square matrices.
*/
static bool test_matrix_smul_square(void) {

    matrix_view out_view;
    const matrixv_t *in_mat;
    matrixv_t *out;


    /* We'll iterate over all matrices in valid_square_pairs for now */
    std::array<const matrix_view *, 2> matrices;
    for (size_t i = 0; i < valid_square.size(); i++) {
        matrices = std::array<const matrix_view *, 2>{ &valid_square[i].in1, &valid_square[i].in2 };

        for (size_t j = 0; j < 2; j++) {
            in_mat = matrices[j]->get_view();
            out_view.resize_view(in_mat->nrow, in_mat->ncol); 
            out = out_view.get_view();

            /* Multiply the current matrix by the scalars in `scalars` */
            for ( scalar s : scalars ) {
                ASSERT_TRUE(scalar_matrix_mul(out, s, in_mat) == 0); 
                ASSERT_CORRECT_SMUL(out, s, in_mat);
            }
        }
    }

    return true;
}


/*
* Scalar-matrix multiplication on valid row vectors.
*/
static bool test_matrix_smul_row_vec(void) {

    matrix_view out_view;
    const matrixv_t *in_mat;
    matrixv_t *out;

    std::array<const matrix_view *, 2> matrices;
    for (size_t i = 0; i < valid_row.size(); i++) {
        matrices = std::array<const matrix_view *, 2>{ &valid_row[i].in1, &valid_row[i].in2 };

        for (size_t j = 0; j < 2; j++) {
            in_mat = matrices[j]->get_view();
            out_view.resize_view(in_mat->nrow, in_mat->ncol);
            out = out_view.get_view();

            for (scalar s : scalars) {
                ASSERT_TRUE(scalar_matrix_mul(out, s, in_mat) == 0);
                ASSERT_CORRECT_SMUL(out, s, in_mat);
            }
        }
    }

    return true;
}


/*
* Scalar-matrix multiplication on valid column vectors.
*/
static bool test_matrix_smul_col_vec(void) {

    matrix_view out_view;
    const matrixv_t *in_mat;
    matrixv_t *out;

    std::array<const matrix_view *, 2> matrices;
    for (size_t i = 0; i < valid_column.size(); i++) {
        matrices = std::array<const matrix_view *, 2>{ &valid_column[i].in1, &valid_column[i].in2 };

        for (size_t j = 0; j < 2; j++) {
            in_mat = matrices[j]->get_view();
            out_view.resize_view(in_mat->nrow, in_mat->ncol);
            out = out_view.get_view();

            for (scalar s : scalars) {
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
    TEST(test_matrix_mmul_square),
    TEST(test_matrix_mmul_row_vec),
    TEST(test_matrix_smul_square),
    TEST(test_matrix_smul_row_vec),
    TEST(test_matrix_smul_col_vec)
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



