#include "linalg/test_unary_operations.hpp"
#include "linalg/test_inputs.hpp"

/* C interface */  
#include "linalg/unary_operations.h"


/*
* Test determinant routine on square matrices
*/
static bool test_matrix_det_valid_square(void) {
    
    scalar detval;
    matrixv_t *in_mat;
    std::array<matrix_view *, 2> matrices;


    /* TODO: check that all matrices in valid_square are non-singular */
    for ( matrix_pair pair : valid_square ) {
        matrices = std::array<matrix_view *, 2>{ &pair.in1, &pair.in2 }; 
        
        for (size_t i = 0; i < 2; i++) {
            matrix_view original = *matrices[i];
            in_mat = matrices[i]->get_view();
            
            assert(in_mat->nrow == in_mat->ncol && "Matrix is not square, can't compute determinant!");
                         
            /* Note `in_mat` is modified in-place by matrix_det */
            ASSERT_TRUE(matrix_det(&detval, in_mat) == 0);
            ASSERT_CORRECT_DET(detval, original.get_view()); 
        }
    }

    return true;
}



unsigned run_unary_operations_tests(int& total) {
    total = 1;
    return test_matrix_det_valid_square() ? 1 : 0;
}


