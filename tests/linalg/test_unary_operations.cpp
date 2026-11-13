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


/*
* Test the RREF routine on square non-singular matrices.
*
* The RREF of these matrices should be equal to the identity matrix.
*/
static bool test_matrix_rref_valid_square(void) {

    matrixv_t *in_mat, *original_in_mat, *identity_view;

    for ( matrix_view view : valid_square_nonsingular ) {

        /* Note matrix_rref modifies the matrix in-place, so we must save a copy */
        matrix_view original = view;
        original_in_mat = original.get_view();

        in_mat = view.get_view();
        assert(in_mat->nrow == in_mat->ncol && "Matrix is not square, this test expects square matrices!");

        matrix_view identity = init_identity(in_mat->nrow);
        identity_view = identity.get_view(); 

        ASSERT_TRUE(matrix_rref(in_mat) == 0);
        ASSERT_CORRECT_RREF(in_mat, original_in_mat, identity_view);
    }

    return true;
}


/*
* Test the RREF routine on square singular matrices.
*
* Note this test runs on valid_square_singular_rref inputs. The first
* matrix is a square singular matrix and the second matrix in a given pair
* is the expected RREF.
*/
static bool test_matrix_rref_singular_square(void) {
    
    matrixv_t *in_mat, *original_in_mat, *expected_view;

    for ( matrix_pair pair : valid_square_singular_rref ) {

        /* Save copy of original matrix */
        matrix_view original = pair.in1;
        original_in_mat = original.get_view();

        in_mat = pair.in1.get_view();
        assert(in_mat->nrow == in_mat->ncol && "Matrix is not square, this test expects square matrices!");

        expected_view = pair.in2.get_view();

        ASSERT_TRUE(matrix_rref(in_mat) == 0);
        ASSERT_CORRECT_RREF(in_mat, original_in_mat, expected_view);
    }

    return true;
}


static const std::array test_cases = {
    TEST(test_matrix_det_valid_square),
    TEST(test_matrix_det_singular_square),
    TEST(test_matrix_rref_valid_square),
    TEST(test_matrix_rref_singular_square)
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


