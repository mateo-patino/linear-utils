#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>

#include "matrix.h"

/*
* This file defines the basic data types used by this program.
*
* The basic interface for operators and matrices is also defined here.
*/


/*
* Token interface...
*/
typedef enum {
    OPERATOR,
    SCALAR,
    MATRIX,
    LPAREN,
    RPAREN,
    TOKENS_END /* signals the end of a token_t sequence */
} token_type;


typedef struct {
    token_type type;
    void *obj;
    const char *user_str;
} token_t;


/*
* Operator interface...
*/

/* Supported operators */
typedef enum {
    ADD,
    SUB,
    MUL,
    DIV,
    DET,
    RREF,
    INV,
    NUM_OP
} operator_type;

typedef enum {
    MATRIX_OPERAND,
    SCALAR_OPERAND
} operand_type;


/*
* There is no operator struct because all properties of operators are saved
* in static tables that are parametrized by operator_type values.
*
* Tokens of type OPERATOR have an 'obj' pointer to an operator_type value,
* which can be used to query tables for information about the operator.
*/

typedef enum {
    LEFT_ASSOC,
    RIGHT_ASSOC
} assoc;

/* Arity, precedence, associativity, and alias tables for operators */
extern const char arity[NUM_OP];
extern const char precedence[NUM_OP];
extern const assoc associativity[NUM_OP];
extern const char **operator_alias[NUM_OP];


/*
* Calls free_token_objs_by_count and then frees 'tokens'.
*/
void fully_free_tokens(token_t *tokens, size_t count);


/*
* Frees 'count' token_t->obj and token_t->user_str pointers in a 'tokens' array.
* The 'tokens' pointer is not freed, however.
*/
void free_tokens_by_count(token_t *tokens, size_t count);


/*
* Frees a token_t's obj member (but not the token itself).
*/
void free_token_obj(token_t *tok);


/*
* Prints a matrix in format MAT(entries) if there are 4 or fewer entries.
* Otherwise, it prints MAT(first two entries, last two entries).
*/
void print_matrix(const matrix_t *mat);

/*
* Prints the text name of an operator_type enum value.
*/
void print_operator_enum(operator_type op);

/*
* Pretty prints a token to stdout.
*/
void print_token(const token_t *tok);


/*
* Returns true if `tok` is a SCALAR or MATRIX 
*/
bool is_operand_token(const token_t *tok);

#endif
