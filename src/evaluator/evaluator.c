#include "evaluator.h"
#include "linalg/view.h"
#include "linalg/arithmetic.h"
#include "linalg/scalar.h"
#include "types/token.h"
#include "arena.h"
#include "errorprinter.h"

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>


/*
* An internal interface for recording error codes.
*/
static eval_status internal_eval_status = EVAL_OK;
static bool has_error_status = false;

static void set_status(eval_status code) {
    /* If an error status (!= EVAL_OK) has been set, don't overwrite */
    if (has_error_status) {
        return;
    }
    internal_eval_status = code;
    if (code != EVAL_OK) {
        has_error_status = true;
    }
}


static eval_status get_status(void) {
    return internal_eval_status;
}


static void clear_status(void) {
    internal_eval_status = EVAL_OK;
    has_error_status = false;
}


/*
* Writes an error message to the global buffer according to
* the given eval_status code.
*/
static void set_status_errmsg(eval_status st) {
    switch (st) {
        case EVAL_OK:
            set_error("AST evaluation successful");
            return;
        case EVAL_NULL_VALUE:
            set_error("Invalid AST (either NULL or root is NULL)");
            return;
        case EVAL_MEMORY_FAILURE:
            set_error("Memory failure.");
            return;
        case EVAL_TOKEN_CONVERSION_FAILED:
            set_error("'scalar_t' to 'scalar' or 'matrix_t' to 'matrixv_t' conversion failed.");
            return;
        default:
            set_error("Unknown eval status code");
            return;
    }
}


/* A common pattern is to set a status and return NULL */
#define RETURN_NULL_AND_STATUS(x) \
    do { \
        set_status(x); \
        return NULL; \
    } while (0)


/* Set the CALLER's status and return NULL */
#define RETURN_NULL_AND_CSTATUS(x, status) \
    do { \
        eval_status *_st = status; \
        if (_st) { \
            *_st = (eval_status)x; \
        } \
        return NULL; \
    } while (0) 


/*
* Copy (allocate) a result_t at `tmp` to an arena.
* A pointer to the new struct in the arena is returned.
*/
static result_t *copy_result(const result_t *tmp, arena_t *arena) {
    if (!tmp || !arena) {
        return NULL;
    }

    const size_t offset = awrite((char *)tmp, sizeof(result_t), _Alignof(result_t), arena);
    if (offset == SIZE_MAX) {
        return NULL;
    }

    return (result_t *)(arena->start + offset);
}


/*
* Converts a token_t struct to a result_t struct.
* It writes the new result_t struct in the memory arena `arena`,
* and returns a pointer to the struct upon success and NULL upon 
* failure.
*
* The function expects that token is a valid operand token (i.e. a scalar
* or matrix).
*/
static result_t *token_to_result(const token_t *token, arena_t *arena) {
    assert(is_operand_token(token) == true);

    result_t tmp;
    
    if (token->type == SCALAR) {
        tmp.type = SCALAR_RES;
        tmp.obj = to_linalg_scalar(*(scalar_t *)token->obj, arena);
    }
    else if (token->type == MATRIX) {
        tmp.type = MATRIX_RES;
        tmp.obj = create_matrix_view((matrix_t *)token->obj, arena);
    }

    if (!tmp.obj) {
        RETURN_NULL_AND_STATUS(EVAL_TOKEN_CONVERSION_FAILED);
    } 

    return copy_result(&tmp, arena);
}


/*
* Copy (allocate) a scalar value to an arena.
* A pointer to the scalar in the arena is returned.
*/
static scalar *copy_scalar(scalar val, arena_t *arena) {
    if (!arena) {
        return NULL;
    }

    const size_t offset = awrite((char *)&val, sizeof(scalar), _Alignof(scalar), arena);
    if (offset == SIZE_MAX) {
        return NULL;
    }

    return (scalar *)(arena->start + offset);
}


/*
* Copy (allocate) a matrix view `tmp` to `arena`.
*/
static matrixv_t *copy_view(const matrixv_t *tmp, arena_t *arena) {
    if (!tmp || !arena) {
        return NULL;
    }

    const size_t offset = awrite((char *)tmp, sizeof(matrixv_t), _Alignof(matrixv_t), arena);
    if (offset == SIZE_MAX) {
        return NULL;
    }

    return (matrixv_t *)(arena->start + offset);
}


/*
* Allocates memory for `nentry` scalar values in `arena`.
* All values are initialized to 0.
*
* A pointer to the first scalar value is returned upon succes.
*/
static scalar *allocate_scalars(size_t nentry, arena_t *arena) {
    const size_t offset = awrite(NULL, nentry * sizeof(scalar), _Alignof(scalar), arena);
    
    if (offset == SIZE_MAX) {
        return NULL;
    }

    scalar *out = (scalar *)(arena->start + offset);
    memset(out, 0, nentry);

    return out;
}


/*
* Allocates and sets up the output view struct for operation an `op`
* with matrix operands. 
*
* It returns a pointer to a matrix view allocated on `arena` 
* with values initialized before the operation. 
*
* The `data` pointer points to an address with enough space to
* accomodate the result of the operation.
*/
static matrixv_t *initialize_output_view(operator_type op, const matrixv_t *left, const matrixv_t *right, arena_t *arena) {
    if (!left || !right) {
        return NULL;
    }

    matrixv_t tmp = {0};
    size_t nrow = 0, ncol = 0;

    switch (op) {

        case ADD:
        case SUB:
            assert(left->ncol == right->ncol && left->nrow == right->nrow);
            nrow = left->nrow; 
            ncol = left->ncol;

            tmp.nrow = nrow;
            tmp.ncol = ncol;

            tmp.column_stride = 1;
            tmp.row_stride = 1;

            tmp.data = allocate_scalars(nrow * ncol, arena);
            if (!tmp.data) {
                return NULL;
            }
            break;

        case MUL:
            /* TODO */
            break;

        default:
            /* TODO */
            break;
    }

    /* Copy the temporary view struct to the arena */
    return copy_view(&tmp, arena);
}


/*
* Allocates a matrix view with dimensions `nrow` and `ncol` 
* in `arena`.
*
* The row and column strides are initialized to 1, and the view points
* to a block with `nrow` * `ncol` scalar entries.
*/
static matrixv_t *init_view_with_dim(size_t nrow, size_t ncol, arena_t *arena) {
    matrixv_t tmp = {0};

    tmp.nrow = nrow;
    tmp.ncol = ncol;

    tmp.row_stride = 1;
    tmp.column_stride = 1;

    tmp.data = allocate_scalars(nrow * ncol, arena);
    if (!tmp.data) {
        return NULL;
    }

    return copy_view(&tmp, arena);
}
 

/************************************
* ADDITION
************************************/

/* Scalar-scalar addition */
static result_t *ss_add(const result_t *left, const result_t *right, arena_t *arena) {
    if (!left || !right) {
        return NULL;
    }
    result_t tmp = {0};

    scalar l_val = *(scalar *)left->obj;
    scalar r_val = *(scalar *)right->obj;

    tmp.type = SCALAR_RES;
    tmp.obj = copy_scalar(l_val + r_val, arena);

    if (!tmp.obj) {
        return NULL;
    }
    
    return copy_result(&tmp, arena);
}


/* Matrix-matrix addition */
static result_t *mm_add(const result_t *left, const result_t *right, arena_t *arena) {
    if (!left || !right || !left->obj || !right->obj) {
        return NULL;
    }
    result_t tmp = {0};

    /* Compute output matrix */
    const matrixv_t *A = (matrixv_t *)left->obj, *B = (matrixv_t *)left->obj;
    matrixv_t *C = initialize_output_view(ADD, A, B, arena);
    if (matrix_add(C, A, B) == -1) {
        return NULL;    
    }
    
    tmp.type = MATRIX_RES;
    tmp.obj = C;

    return copy_result(&tmp, arena); 
}


/************************************
* SUBTRACTION
************************************/

/* Scalar-scalar subtraction */
static result_t *ss_sub(const result_t *left, const result_t *right, arena_t *arena) {
    if (!left || !right || !left->obj || !right->obj) {
        return NULL;
    }
    result_t tmp = {0};

    scalar l_val = *(scalar *)left->obj;
    scalar r_val = *(scalar *)right->obj;

    tmp.type = SCALAR_RES;
    tmp.obj = copy_scalar(l_val - r_val, arena);

    if (!tmp.obj) {
        return NULL;
    }

    return copy_result(&tmp, arena);
}


/* Matrix-matrix subtraction */
static result_t *mm_sub(const result_t *left, const result_t *right, arena_t *arena) {
    if (!left || !right || !left->obj || !right->obj) {
        return NULL;
    }
    result_t tmp = {0};

    const matrixv_t *A = (matrixv_t *)left->obj, *B = (matrixv_t *)left->obj;
    matrixv_t *C = initialize_output_view(SUB, A, B, arena);
    if (matrix_sub(C, A, B) == -1) {
        return NULL;
    }

    tmp.type = MATRIX_RES;
    tmp.obj = C; 

    return copy_result(&tmp, arena);
}


/************************************
* MULTIPLICATION
************************************/

/* Scalar-scalar multiplication */
static result_t *ss_mul(const result_t *left, const result_t *right, arena_t *arena) { 
    if (!left || !right) {
        return NULL;
    }
    result_t tmp = {0};

    scalar l_val = *(scalar *)left->obj;
    scalar r_val = *(scalar *)right->obj;

    tmp.type = SCALAR_RES;
    tmp.obj = copy_scalar(l_val * r_val, arena);

    if (!tmp.obj) {
        return NULL;
    }
    
    return copy_result(&tmp, arena);
}


/* Scalar-matrix multiplication. It is assumed that `left` is the scalar and `right` is the matrix */
static result_t *sm_mul(const result_t *left, const result_t *right, arena_t *arena) {   
    if (!left || !right) {
        return NULL;
    }
    result_t tmp = {0};

    const scalar s = *(scalar *)left->obj;
    const matrixv_t *A = (matrixv_t *)right->obj;
    matrixv_t *C = init_view_with_dim(A->nrow, A->ncol, arena);
    if (scalar_matrix_mul(C, s, A) == -1) {
        return NULL;
    } 
    
    tmp.type = MATRIX_RES;
    tmp.obj = C;

    return copy_result(&tmp, arena);
}


/* Matrix-matrix multiplication */
static result_t *mm_mul(const result_t *left, const result_t *right, arena_t *arena) {
    if (!left || !right) {
        return NULL;
    }
    result_t tmp = {0};

    const matrixv_t *A = (matrixv_t *)left->obj, *B = (matrixv_t *)right->obj;
    matrixv_t *C = initialize_output_view(MUL, A, B, arena);
    if (matrix_mul(C, A, B) == -1) {
        return NULL;
    }
    
    tmp.type = MATRIX_RES;
    tmp.obj = C;
    
    return copy_result(&tmp, arena);
}


/************************************
* DIVISION
************************************/

/* Scalar-scalar division */
static result_t *ss_div(const result_t *left, const result_t *right, arena_t *arena) {
    if (!left || !right) {
        return NULL;
    }
    result_t tmp = {0};

    scalar l_val = *(scalar *)left->obj, r_val = *(scalar *)right->obj;
    
    tmp.type = SCALAR_RES;
    tmp.obj = copy_scalar(l_val / r_val, arena);

    if (!tmp.obj) {
        return NULL;
    }

    return copy_result(&tmp, arena);
}



/*
* Dispatches the operation `op` to the linalg library with operands `left` and  `right`.
* `left` and `right` MUST point to data structures used by the linalg library (`scalar`
* and `matrixv_t`). 
*
* It returns a pointer to a result_t struct containing the result of the operation upon
* success and NULL otherwise.
*/
static result_t *perform_operation(operator_type op, result_t *left, result_t *right, arena_t *arena) {
    /*
    * Reject NULL right for unary operators and NULL left or right for 
    * binary operators.
    */
    if ((is_unary_operator_enum(op) && !right) || (!left || !right)) {
        RETURN_NULL_AND_STATUS(EVAL_NULL_VALUE);
    }

    result_t *out;
    eval_status st = EVAL_OK;

    switch (op) {

        case ADD:
            if (left->type == SCALAR_RES && right->type == SCALAR_RES) {
                out = ss_add(left, right, arena); 
            }
            assert(left->type == MATRIX_RES && right->type == MATRIX_RES);
            out = mm_add(left, right, arena);

            if (!out) { st = EVAL_ADD_FAILED; }
            break;

        case SUB:
            if (left->type == SCALAR_RES && right->type == SCALAR_RES) {
                out = ss_sub(left, right, arena);
            }
            assert(left->type == MATRIX_RES && right->type == MATRIX_RES);
            out = mm_sub(left, right, arena);

            if (!out) { st = EVAL_SUB_FAILED; }
            break;

        case MUL:
            if (left->type == SCALAR_RES && right->type == SCALAR_RES) {
                out = ss_mul(left, right, arena);
            }
            /* We allow both scalar-matrix multiplication in either order: 10 * A or A * 10. */
            else if (left->type == SCALAR_RES && right->type == MATRIX_RES) {
                out = sm_mul(left, right, arena);
            }
            else if (left->type == MATRIX_RES && right->type == SCALAR_RES) {
                out = sm_mul(right, left, arena);
            }
            assert(left->type == MATRIX_RES && right->type == MATRIX_RES);
            out = mm_mul(left, right, arena);

            if (!out) { st = EVAL_MUL_FAILED; }
            break;
        
        case DIV:
            assert(left->type == SCALAR_RES && right->type == SCALAR_RES);
            out = ss_div(left, right, arena);
            break;

        case DET:
            assert(left == NULL && right != NULL);
            /* TODO */
            break;

        case RREF:
            assert(left == NULL && right != NULL);
            /* TODO */
            break;

        case INV:
            assert(left == NULL && right != NULL);
            /* TODO */
            break;

        case NUM_OP:
        default:
            RETURN_NULL_AND_STATUS(EVAL_NULL_VALUE);
    }

    if (!out) {
        RETURN_NULL_AND_STATUS(st);
    }

    return out;
}


result_t *evaluate_ast(const ast_t *ast, eval_status *status) {
    if (!ast || !ast->root) {
        RETURN_NULL_AND_CSTATUS(EVAL_NULL_VALUE, status);
    }

    /* Set internal status to OK before starting */
    clear_status();
    
    /* 
    * This memory arena will hold all of the matrix view and result objects
    * used during evaluation.
    */
    arena_t *arena = create_arena(MiB(12));

    result_t *tmp = evaluate_subtree(ast->root, arena);  
    eval_status st = get_status();

    /* Both tmp and st should indicate an error simultaneously so the || is just a precaution */
    if (st != EVAL_OK || !tmp) {
        assert(st != EVAL_OK && tmp == NULL);         
        set_status_errmsg(st);
        free_arena(arena);
        RETURN_NULL_AND_CSTATUS(st, status);
    }

    /* 
    * The result_t pointer returned by evaluate_subtree lives in the arena,
    * so copy to another address and return the copy.
    *
    * TODO: consider translating the result_t object retured by evaluate_subtree
    * to anothet struct that is agnostic of the linalg module.
    */
    result_t *final = malloc(sizeof(result_t));
    if (!final) {
        set_status_errmsg(EVAL_MEMORY_FAILURE);
        free_arena(arena);
        RETURN_NULL_AND_CSTATUS(EVAL_MEMORY_FAILURE, status);
    }
    memcpy(final, tmp, sizeof(result_t));

    free_arena(arena);
    return final;
}


/*
* This is primary helper function to evaluate_ast function.
*
* This helper does not free the memory arena and does not write errors
* to the global error buffer (this is done by evaluate_ast). It only
* sets the internal status.
*
* Do not call this function on NULL nodes. This is done so that if a NULL node
* is passed, we raise and error and can detect it and debug it properly.
*/
result_t *evaluate_subtree(const node_t *node, arena_t *arena) {
    if (!node) {
        /*
        * Do not call this function on correct (intended) NULL nodes, otherwise
        * an error will be set.
        */
        RETURN_NULL_AND_STATUS(EVAL_NULL_VALUE);
    }

    const token_t *token = node->token;
    assert(token != NULL);

    /* If token is an operand, return it as a result_t */
    if (is_operand_token(token)) {
        /*
        * token_to_result sets status to EVAL_TOKEN_CONVERSION_FAILED upon conversion failure
        * but not malloc failure.
        */
        result_t *out = token_to_result(token, arena);
        if (!out && !has_error_status) {
            RETURN_NULL_AND_STATUS(EVAL_MEMORY_FAILURE);
        }
        return out;
    }

    /* If not operand token, it must be an operator token, so recurse as appropriate */
    assert(token->type == OPERATOR);
    result_t *left, *right;

    /*
    * As is convention in this codebase, if a node is a unary operator,
    * we set its left child to NULL and recurse on the right only.
    */
    if (is_unary_operator_token(token)) {
        assert(node->left == NULL && node->right != NULL);
        left = NULL;
        right = evaluate_subtree(node->right, arena);
    }
    else {
        assert(node->left != NULL && node->right != NULL);
        left = evaluate_subtree(node->left, arena);
        right = evaluate_subtree(node->right, arena);
    }

    /* Perform operation between `left` and `right` and return result */
    assert(token->obj != NULL);
    operator_type op = *(operator_type *)token->obj;

    /*
    * perform_operation ensures `left` and/or `right` are not NULL
    * and returns NULL if so or if another error happens. 
    */
    return perform_operation(op, left, right, arena);
}

