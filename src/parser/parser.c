#include "parser.h"
#include "ast.h"
#include "types/token.h"
#include "errorprinter.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>


/* 
* A private status variable and interface for reporting errors arising from
* this module. The status is forwarded to any callers via create_ast_from_tokens 
*/
static parse_status internal_parser_status = PARSE_OK;
static bool has_error_status = false;

static void set_status(parse_status val) {
    /* If an error status has already been set, do not overwrite */
    if (has_error_status) {
        return;
    }
    internal_parser_status = val;
    if (val != PARSE_OK) {
        has_error_status = true;
    }
}


static parse_status get_status(void) {
    return internal_parser_status;
}


static void clear_status(void) {
    internal_parser_status = PARSE_OK;
    has_error_status = false;
}

/* Setting an error status and returning NULL is a commnon pattern.
* Note this macro sets the INTERNAL status. */
#define RETURN_NULL_AND_STATUS(val)  \
    do { \
        set_status(val); \
        return NULL; \
    } while (0)

/* This macro sets the CALLER'S status */
#define RETURN_NULL_AND_CSTATUS(val, status) \
    do { \
        parse_status *_status = (status); \
        if (_status) { *_status = val; } \
        return NULL; \
    } while (0)


/*
* Helper to find_last_op_index function (see below).
*
* Populate the tuple `current_op` with depth, prec, assoc, and 
* index values.
*/
static void set_operator_tuple(tuple_t *op_tuple, operator_type op_type, int depth, int index) {
    if (!op_tuple) {
        return;
    }
    op_tuple->op_type = op_type;
    op_tuple->depth = depth;
    op_tuple->prec = precedence[op_type];
    op_tuple->assoc = associativity[op_type];
    op_tuple->index = index;
}


/*
* Helper to the find_last_op_index function (see below).
*
* The function below updates `last_so_far` by comparing it to `other` so 
* `last_so_far` contains the operator that would be evaluated last in
* an expression.
*/
static void compare_operators(tuple_t *other, tuple_t *last_so_far) {
    if (!other || !last_so_far) {
        return;
    }

    /* Always choose the operator with the lowest depth */
    if (other->depth < last_so_far->depth) {
        goto UPDATE_LAST_SO_FAR;
    }
    else if (other->depth == last_so_far->depth) {
        /* If depths are equal, choose op with least precedence */
        if (other->prec < last_so_far->prec) {
            goto UPDATE_LAST_SO_FAR;
        }
        else if (other->prec == last_so_far->prec) {

            /* 
            * If precedences are equal, then choose the operator that would be
            * evaluated last according to associativity and index. A key invariant: if two
            * operators have the same precedence, they must have the same associativity
            * (any correctly defined precedence table must satisfy this). Therefore,
            * `other` and `last_so_far` must have the same associativity. If they're
            * left-associative, pick the operator with the greatest `index` (furthest to
            * the right); if they're right-associative, pick the operator furthest to the 
            * left).
            */

            if (other->assoc == LEFT_ASSOC) { /* Could've checked last_so_far->assoc too */
                if (other->index > last_so_far->index) {
                    goto UPDATE_LAST_SO_FAR;
                }
            }
            else if (other->assoc == RIGHT_ASSOC){
                if (other->index < last_so_far->index) {
                    goto UPDATE_LAST_SO_FAR;
                }
            }

        }
    }
    return;

UPDATE_LAST_SO_FAR:
    memcpy(last_so_far, other, sizeof(tuple_t));
}


/*
* Returns the index of the last operation that would occur according to the
* precedence and associativity rules in types/token.h should one evaluate the
* tokens expression in the range [low, high] exclusive.
*
* If no operator token is found in the [low, high], -1 is returned.
*/
static int find_last_op_index(const token_t *tokens, int low, int high) {
    
    /* Some starting values that no valid operator could have */
    tuple_t last_so_far = {NUM_OP, LONG_MAX, LONG_MAX, -1, -1};
    tuple_t current_op;
    int current_depth = 0;
    token_type tok_type;
    operator_type op;

    for (int i = low; i <= high; i++) {

        tok_type = tokens[i].type;

        switch (tok_type) {
            case SCALAR:
            case MATRIX:
            case TOKENS_END:
                continue;
            case LPAREN:
                current_depth++;
                continue;
            case RPAREN:
                current_depth--;
                continue;
            case OPERATOR:
                op = *(operator_type *)tokens[i].obj;
                set_operator_tuple(&current_op, op, current_depth, i);
                compare_operators(&current_op, &last_so_far);
        }
    }

    return last_so_far.index;
}


/*
* Returns the index inside of the range [low, high] of exactly one operand type.
* If no operands exist or more than one does, -1 is returned.
*/
static int get_remaining_operand(const token_t *tokens, int low, int high) {
    int index = -1;
    for (int i = low; i <= high; i++) {
        if (is_operand_token(tokens + i)) {
            /* `index` must be -1 before setting it to a valid index value. *
               If `index` has been previously set and we attempt to set it again,
               more than one operand token must exist, so we return in failure. */
            if (index != -1) {
                return -1;
            }
            index = i;
        }
    }
    return index;
}



/*
* Returns true if at least one operand token exists in the range [low, high]
*/
static bool has_any_operands(const token_t *tokens, int low, int high) {
    if (!tokens) {
        return false;
    }

    for (int i = low; i <= high; i++) {
        if (is_operand_token(tokens + i)) {
            return true;
        }
    }
    return false;
}


/*
* Returns true if 1) `tokens` has balanced parentheses (i.e. all parenthesis opened are closed),
* 2) all opening parenthesis come before closing ones, and 3) all parenthesis have content inside.
*/
static bool has_balanced_parenthesis(const token_t *tokens, size_t sz) {
    token_type type;
    int open_count = 0;
    bool expect_content = false;

    for (size_t i = 0; i < sz; i++) {
        type = tokens[i].type;

        switch (type) {
            case SCALAR:
            case MATRIX:
            case OPERATOR:
                expect_content = false;
                continue;

            case LPAREN:
                open_count++;
                expect_content = true;
                continue;
                    
            case RPAREN:
                /* ) came before ( or empty (  ) */
                if (open_count == 0 || expect_content) {
                    return false;
                }
                open_count--;
                continue;

            case TOKENS_END:
            default:
                continue;
        }
    }

    return !open_count;
}


/*
* `status` is a variable provided by the caller, and internally, the parser module will
* have its own (static) `parse_status` variable.
* 
* The create_ast_from_tokens function is the only point where the caller's status shall be 
* updated.
*/
ast_t *create_ast_from_tokens(const token_t *tokens, size_t sz, parse_status *status) {
    /* 
    * Clear any status from previous calls to this function (note 
    * the internal status is static and could propagate if not cleared 
    */
    clear_status();

    if (!tokens || !sz) {
        RETURN_NULL_AND_CSTATUS(PARSE_INVALID_TOKENS, status);
    }

    /* Do parenthesis validation on tokens */
    if (!has_balanced_parenthesis(tokens, sz)) {
        set_error("Invalid parentheses.");
        RETURN_NULL_AND_CSTATUS(PARSE_UNBALANCED_PARENS, status);
    }
    
    ast_t *tree;
    if (!(tree = malloc(sizeof(ast_t)))) {
        RETURN_NULL_AND_CSTATUS(PARSE_MEMORY_FAILURE, status);
    }

    /* create_ast_helper sets the internal status */
    tree->root = create_ast_helper(tokens, 0, sz-1);
    if (!tree->root || get_status() != PARSE_OK) {
        fully_free_ast(tree);
        RETURN_NULL_AND_CSTATUS(get_status(), status);
    }

    assert(get_status() == PARSE_OK);
    if (status) { *status = PARSE_OK; }

    return tree;
}

node_t *create_ast_helper(const token_t *tokens, int low, int high) {
    if (!tokens) {
        return NULL;
    }

    node_t *new_node;
    node_t *left;
    node_t *right;

    const int last_op_index = find_last_op_index(tokens, low, high);

    /* No operator token found in [low, high]. Exactly ONE operand must exist */
    if (last_op_index == -1) {
        int remaining_operand_index;

        /* -1 means not EXACTLY ONE operand was found. `tokens` is a malformed expression. */
        if ((remaining_operand_index = get_remaining_operand(tokens, low, high)) == -1) {
            set_error("Invalid algebraic expression.");
            RETURN_NULL_AND_STATUS(PARSE_INVALID_EXPRESSION);
        }

        /* We have one operand, so recursion stops. Initialize node and return it. */
        new_node = initialize_node(tokens + remaining_operand_index, NULL, NULL);
        if (!new_node) {
            set_error("malloc() failed.");
            RETURN_NULL_AND_STATUS(PARSE_MEMORY_FAILURE);
        }

        return new_node;
    }

    /* 
    * An operator token was found, so recurse on the right only for unary operators and on 
    * both sides for binary operators
    */
    if (is_unary_operator_token(tokens + last_op_index)) {
        
        /*
        * When a unary operator is the last operator to be evaluated in the window [low, high], 
        * we must check that there are no extraneous operands to the left of the operator.
        * In other words, there must be no operand tokens in the range [low, last_op_index].
        */
        if (has_any_operands(tokens, low, last_op_index)) {
            set_error("Invalid algebraic expression.");
            RETURN_NULL_AND_STATUS(PARSE_INVALID_EXPRESSION);
        }

        /* 
        * Convention: unary operators will have their RIGHT child set and their 
        * LEFT child will be NULL.
        */
        left = NULL;
        right = create_ast_helper(tokens, last_op_index + 1, high);
        goto RETURN_NEW_NODE;
    }
    
    left = create_ast_helper(tokens, low, last_op_index - 1);
    right = create_ast_helper(tokens, last_op_index + 1, high);

RETURN_NEW_NODE:
    new_node = initialize_node(tokens + last_op_index, left, right);
    if (!new_node) {
        set_error("malloc() failed.");
        
        /* Left and right subtrees could have been allocated so free */
        free_subtree(left);
        free_subtree(right);

        RETURN_NULL_AND_STATUS(PARSE_MEMORY_FAILURE);
    }
 
    return new_node;
}

