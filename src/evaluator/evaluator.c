#include "evaluator.h"
#include "arena.h"
#include <stdbool.h>


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


/* A common pattern is to set a status and return NULL */
#define RETURN_NULL_AND_STATUS(x) \
    do { \
        set_status(x); \
        return NULL; \
    } while (0)


/* Set the CALLER's status and return NULL */
#define RETURN_NULL_AND_CSTATUS(x, status) \
    do { \
        eval_status *st = status; \
        if (st) { \
            *st = x; \
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
    arena_t *arena = create_arena(MiB(32));

    result_t *tmp = evaluate_subtree(ast->root, arena);  
    eval_status st = get_status();
    if (st != EVAL_OK) {
        set_status_errmsg(st);
        free_arena(arena);
        RETURN_NULL_AND_CSTATUS(st, status);
    }

    /* The result_t pointer returned by evaluate_subtree lives in the heap,
    * so copy to another address and return it */
    result_t *final = malloc(sizeof(result));
    if (!final) {
        set_status_errmsg(EVAL_MEMORY_FAILURE);
        free_arena(arena);
        RETURN_NULL_AND_CSTATUS(EVAL_MEMORY_FAILURE, status);
    }
    memcpy(final, tmp, sizeof(result_t));

    free_arena(arena);
    return final;
}
