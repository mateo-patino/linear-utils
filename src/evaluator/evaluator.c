#include "evaluator.h"
#include "linalg/view.h"
#include "types/token.h"
#include "linalg/scalar.h"
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
        case EVAL_INVALID_AST:
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


result_t *evaluate_ast(const ast_t *ast, eval_status *status) {
    if (!ast || !ast->root) {
        RETURN_NULL_AND_CSTATUS(EVAL_INVALID_AST, status);
    }

    /* Set internal status to OK before starting */
    clear_status();
    
    /* 
    * This memory arena will hold all of the matrix view objects
    * used during evaluation.
    */
    arena_t *arena = create_arena(MiB(12));

    result_t *tmp = evaluate_subtree(ast->root, arena);  
    eval_status st = get_status();
    if (st != EVAL_OK) {
        set_status_errmsg(st);
        free_arena(arena);
        RETURN_NULL_AND_CSTATUS(st, status);
    }

    /* The result_t pointer returned by evaluate_subtree lives in the heap,
    * so copy to another address and return it.
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

    result_t result = {0};
    result_t *tmp = &result;
    
    if (token->type == SCALAR) {
        tmp->type = SCALAR_RES;
        tmp->obj = create_linalg_scalar(*(scalar_t *)token->obj, arena);
    }
    else if (token->type == MATRIX) {
        tmp->type = MATRIX_RES;
        tmp->obj = create_matrix_view((matrix_t *)token->obj, arena);
    }

    if (!tmp->obj) {
        RETURN_NULL_AND_STATUS(EVAL_TOKEN_CONVERSION_FAILED);
    } 

    size_t offset = awrite((char *)tmp, sizeof(result_t), _Alignof(result_t), arena);
    if (offset == SIZE_MAX) {
        return NULL;
    }

    return (result_t *)(arena->start + offset);
}

/*
* scalar-scalar addition 
*/
static result_t *ss_add(const result_t *left, const result_t *right, arena_t *arena) {
    if (!left || !right) {
        return NULL;
    }
    result_t result;
    result_t *tmp = &result;
    
    tmp->type = SCALAR_RES;

    scalar val = (*(scalar *)left->obj) + (*(scalar *)right->obj);
    const size_t val_offset = awrite((char *)&val, sizeof(scalar), _Alignof(scalar), arena);
    if (val_offset == SIZE_MAX) {
        return NULL;
    }
    tmp->obj = arena->start + val_offset;

    const size_t result_offset = awrite((char *)&result, sizeof(result_t), _Alignof(result_t), arena);
    if (result_offset == SIZE_MAX) {
        return NULL;
    }
    
    return (result_t *)(arena->start + result_offset);
}


/*
* Dispatches the operation `op` to the linalg library with operands `left` and  `right`.
* `left` and `right` MUST point to data structures used by the linalg library (`scalar`
* and `matrixv_t`). The perform_* functions do not 
*
* It returns a pointer to a result_t struct containing the result of the operation upon
* success and NULL otherwise.
*/
static result_t *perform_operation(operator_type op, result_t *left, result_t *right, arena_t *arena) {
    assert(left != NULL && right != NULL);

    result_t *out;
    switch (op) {

        case ADD:
            if (left->type == SCALAR_RES && right->type == SCALAR_RES) {
                out = ss_add(left, right, arena); /* will add more */
            }
            assert(left->type == MATRIX_RES && right->type == MATRIX_RES);
            out = mm_add(left, right, arena);
            break;

        case SUB:
            out = perform_sub(left, right, arena);
            break;

        case MUL:
            out = perform_mul(left, right, arena);
            break;
        
        case DIV:
            out = perform_div(left, right, arena);
            break;

        case DET:
            assert(left == NULL && right != NULL);
            out = perform_det(right, arena);
            break;

        case RREF:
            assert(left == NULL && right != NULL);
            out = perform_rref(right, arena);
            break;

        case INV:
            assert(left == NULL && right != NULL);
            out = perform_inv(right, arena);
            break;

        case NUM_OP:
        default:
            RETURN_NULL_AND_STATUS(EVAL_INVALID_AST);
    }

    if (!out) {
        /* Set the status to EVAL_FAILED in case a more specific error hasn't been set */
        if (!has_error_status) {
            RETURN_NULL_AND_STATUS(EVAL_FAILED);
        }
        return NULL;
    }

    return out;
}


/*
* This is primary helper function to evaluate_ast function.
*
* This helper does not free the memory arena and does not write errors
* to the global error buffer (this is done by evaluate_ast). It only
* sets the internal status.
*/
result_t *evaluate_subtree(const node_t *node, arena_t *arena) {
    if (!node) {
        /* CHECK: i dont think we need this/might mistakenly report a failure */
        RETURN_NULL_AND_STATUS(EVAL_INVALID_AST);
    }
    const token_t *token = node->token;
    assert(token != NULL);

    /* If token is an operand, return it as a result_t */
    if (is_operand_token(token)) {
        /* token_to_result sets status to EVAL_TOKEN_CONVERSION_FAILED upon failure */
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
    if (is_unary_operator(token)) {
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

    result_t *out = perform_operation(op, left, right, arena);
    if (!out) {
        /* perform_operation sets an internal status, so no need to set it here */
        return NULL;
    }
    return out;
}

