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

typedef enum {
    SCALAR_RES,
    MATRIX_RES,
} result_type;


/*
* The result_t struct is used as a wrapper around the scalar or matrix
* outputs returned by each node triplet in the AST. An operation can yield
* a scalar or a matrix, so the void *obj member in this struct shall point to 
* either a `matrixv_t` or `scalar` object. Theese types are the structures
* used by the linear algebra library. `obj` does NOT point to the `scalar_t`
* or `matrix_t` objects used by the `lin` program.
*/
typedef struct {
    result_type type;
    void *obj;
} result_t;


/*
* Evaluation error codes.
*/
typedef enum {
    EVAL_OK,
    EVAL_INVALID_AST,
    EVAL_MEMORY_FAILURE,

    /* When conversion form scalar_t/matrix_t to linalg's scalar/matrixv_t fails */
    EVAL_TOKEN_CONVERSION_FAILED,

    /* A general error code for when a specific error cause is unknown but we must signal failure */
    EVAL_FAILED 
} eval_status;


/*
* Evaluates `ast` that is guaranteed to be semantically valid.
*
* Returns a pointer to a result_t struct. The `obj` member of this result_t 
* will point to an object in the heap that must be freed by the caller. This
* object will be either a matrixv_t or scalar struct from the linear algebra
* library. TODO: should we translate the result_t to another data structure
* that is agnostic of the linalg internal structs? as main and perhaps other
* high-level callers might be reading from it and do not need to know about 
* the view or scalar structs.
*/
result_t *evaluate_ast(const ast_t *ast, eval_status *status);


/*
* Recursively evaluates an AST subtree rooted at `node`.
*
* Upon succesful evaluation, it returns a pointer to a result_t
* struct allocated in the memory arena `arena`. If an error
* occurs during evaluation, NULL is returned and the internal
* status is set to indicate the error.
*
* This function only sets the internal status and does not write 
* to the global error buffer (that's handled by evaluate_ast).
*/
result_t *evaluate_subtree(const node_t *node, arena_t *arena);



#endif
