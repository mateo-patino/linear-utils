#include "linalg/test_unary_operations.hpp"
#include "linalg/test_inputs.hpp"

/* C interface */  
#include "linalg/unary_operations.h"


/*
* Test determinant routine on square matrices.
*
* Note that matrix_det uses row reduction to put the matrix into an
* upper triangular form and compute its determinant. This test then
* also tests the row reduction routine indirectly.
*/
static bool test_matrix_det_valid_square(void) {
    
    scalar detval;
    matrixv_t *in_mat;

    for ( matrix_view view : valid_square_nonsingular ) {
    
        matrix_view original = view;
        in_mat = view.get_view();
        
        assert(in_mat->nrow == in_mat->ncol && "Matrix is not square, can't compute determinant!");

        /* Note `in_mat` is modified in-place by matrix_det, hence why we make the original copy above */
        ASSERT_TRUE(matrix_det(&detval, in_mat) == 0);
        ASSERT_CORRECT_DET(detval, original.get_view()); 
    }

    return true;
}



/*
* Test determinant routine on square singular matrices.
*
* Note that matrix_det uses row reduction to put the matrix into an
* upper triangular form and compute its determinant. This test then
* also tests the row reduction routine indirectly.
*/
static bool test_matrix_det_singular_square(void) {
    
    scalar detval;
    matrixv_t *in_mat;

    for ( matrix_view view : valid_square_singular ) {
    
        matrix_view original = view;
        in_mat = view.get_view();
        
        assert(in_mat->nrow == in_mat->ncol && "Matrix is not square, can't compute determinant!");

        ASSERT_TRUE(matrix_det(&detval, in_mat) == 1);
        ASSERT_CORRECT_DET(detval, original.get_view()); 
    }

    return true;
}


static const std::array test_cases = {
    TEST(test_matrix_det_valid_square),
    TEST(test_matrix_det_singular_square)
};

unsigned run_unary_operations_tests(int& total) {

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


