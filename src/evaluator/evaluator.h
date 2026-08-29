#ifndef EVALUATOR_H
#define EVALUATOR_H


#include "arena.h"


/*
* The evaluator module is responsible for evaluating an abstract syntax tree
* (AST) produced by the parser module using the algebra library.
*
* The highest-level function in this module is evaluate_ast which performs 
* semantic pre-evaluation checks on the AST provided (e.g. valid matrix 
* dimensions, valid operand types, etc.), recursively evaluates the AST by 
* orchestrating functions from the algebra library, and returns a pointer
* to a result_t struct.
*
* The semantic checks performed before evaluation are meant to completely 
* cover errors that the algebra library does not catch. Errors that require
* long matrix computations are raised by the algebra library, but this library
* does not check for dimension or operand types to minimize overhead (C's "trust
* the programmer" philosophy). Hence, the evaluator performs this checks before
* calling the algebra module.
*/

#include "types/token.h"
#include "parser/ast.h"

/* An expression's result could be a scalar_t or a matrix_t, so we use this wrapper
* struct to contain a void * to that object. */
typedef struct {
    token_type type;
    void *obj;
} result_t;


/*
* Evaluation error codes.
*/
typedef enum {
    EVAL_OK,
    EVAL_INVALID_AST,
    EVAL_MEMORY_FAILURE
} eval_status;


/*
* Evaluates `ast` that is guaranteed to be semantically valid.
*
* Returns a pointer to a result_t struct. The `obj` member of this result_t 
* will point to an object in the heap that must be freed by the caller.
*/
result_t *evaluate_ast(const ast_t *ast, eval_status *status);


/*
* Recursively evaluates an AST subtree rooted at `node`.
*/
result_t *evaluate_subtree(const node_t *node, arena_t *arena);



#endif
