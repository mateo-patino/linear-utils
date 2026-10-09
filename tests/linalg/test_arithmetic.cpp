#include "linalg/test_arithmetic.hpp"
#include "linalg/test_la_helpers.hpp"

/* C interface */
extern "C" {
    #include "linalg/arithmetic.h"
}


/* Struct to hold two operand matrix views */
struct matrix_pair {
    matrix_view in1;
    matrix_view in2;
};


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
        { 2, 2, { -1 -1 -1 -1 } },
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
* Matrix-matrix addition.
* 
*/
static bool test_matrix_add(void) {
    
    matrix_view out; 
    const matrixv_t *in1, *in2;
    for (size_t i = 0; i < valid_square_pairs.size(); i++) {
        
        in1 = valid_square_pairs[i].in1.get_view();
        in2 = valid_square_pairs[i].in2.get_view();

        out.resize_view(in1->nrow, in1->ncol); 

        /* TODO */
        ASSERT_EQ_INT(matrix_add(out.get_view(), in1, in2) == 0); 
    }

    return true;
}


