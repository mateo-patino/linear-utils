#include "linalg/test_arithmetic.hpp"
#include "linalg/test_la_helpers.hpp"

#include <cassert>

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
        { 2, 1, { 1, 2 } },
        { 2, 1, { 3, 4 } }
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
    }
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

        assert(in1->nrow == in2->nrow && in1->ncol == in2->ncol && "Invalid matrix dimensions!\n");
    
        ASSERT_TRUE(matrix_add(out, in1, in2) == 0); 
        ASSERT_CORRECT_ADD(out, in1, in2);
    }

    return true;
}


unsigned run_arithmetic_tests(void) 
{
    return test_matrix_add() ? 1 : 0;
}



