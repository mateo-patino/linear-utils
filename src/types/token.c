#include <stdio.h>

#include "token.h"
#include "matrix.h"

/* Table of operator arities */
const char arity[NUM_OP] = {
    [ADD] = 2,
    [SUB] = 2,
    [MUL] = 2,
    [DIV] = 2,
    [DET] = 1,
    [RREF] = 1,
    [INV] = 1
};

/* Table of precedences */
const char precedence[NUM_OP] = {
    [ADD] = 0,
    [SUB] = 0,
    [MUL] = 1,
    [DIV] = 1,
    [DET] = 2,
    [RREF] = 2,
    [INV] = 2
};

/* Table of associativities */
const assoc associativity[NUM_OP] = {
    [ADD] = LEFT_ASSOC,
    [SUB] = LEFT_ASSOC,
    [MUL] = LEFT_ASSOC,
    [DIV] = LEFT_ASSOC,
    [DET] = RIGHT_ASSOC,
    [RREF] = RIGHT_ASSOC,
    [INV] = RIGHT_ASSOC
};


/* Operator aliases. Note each array of aliases is NULL terminated. */
static const char *add_alias[] = { "add", "plus", "+", NULL };
static const char *sub_alias[] = { "sub", "minus", "-", NULL };
static const char *mul_alias[] = { "mul", "times", "*", NULL };
static const char *div_alias[] = { "div", "over", "/", NULL };
static const char *det_alias[] = { "det", "determinant", "detof", NULL };
static const char *rref_alias[] = { "rref", "reduced", NULL };
static const char *inv_alias[] = { "inv", "inverse", NULL };

const char **operator_alias[NUM_OP] = {
    [ADD] = add_alias,
    [SUB] = sub_alias,
    [MUL] = mul_alias,
    [DIV] = div_alias,
    [DET] = det_alias,
    [RREF] = rref_alias,
    [INV] = inv_alias,
};


void fully_free_tokens(token_t *tokens, size_t count) {
    free_tokens_by_count(tokens, count);
    free(tokens);
}


void free_tokens_by_count(token_t *tokens, size_t count) {
    if (!tokens) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        free_token_obj(tokens + i);
        free((char *)tokens[i].user_str);
    }
}


void free_token_obj(token_t *tok) {
    if (!tok || !tok->obj) {
        return;
    }
    
    if (tok->type == MATRIX) {
        free_matrix((matrix_t *)tok->obj);
    }
    else {
        free(tok->obj);
    }
}


void print_matrix(const matrix_t *mat) {
    if (!mat || !mat->data || !(mat->ncol && mat->nrow)) {
        return;
    }
    fprintf(stdout, "MAT(");
    int digits = 2;
    scalar_t *data = mat->data;
    size_t nentry = mat->ncol * mat->nrow;

    if (nentry <= 4) {
        for (size_t i = 0; i < nentry-1; i++) {
            fprintf(stdout, PRISCALAR ", ", digits, data[i]);
        }
        fprintf(stdout, PRISCALAR, digits, data[nentry-1]);
    }
    else {
        /* Print a, b ... c, d */
        fprintf(stdout, PRISCALAR ", ", digits, data[0]);
        fprintf(stdout, PRISCALAR " ... ", digits, data[1]);
        fprintf(stdout, PRISCALAR ", ", digits, data[nentry-2]);
        fprintf(stdout, PRISCALAR, digits, data[nentry-1]);
    }
    fputc(')', stdout);
}


void print_operator_enum(operator_type op) {
    switch (op) {
        case ADD:
            fprintf(stdout, "ADD");
            break;
        case SUB:
            fprintf(stdout, "SUB");
            break;
        case MUL:
            fprintf(stdout, "MUL");
            break;
        case DIV:
            fprintf(stdout, "DIV");
            break;
        case DET:
            fprintf(stdout, "DET");
            break;
        case RREF:
            fprintf(stdout, "RREF");
            break;
        case INV:
            fprintf(stdout, "INV");
            break;
        case NUM_OP:
            break;
    }
}


void print_token(const token_t *tok) {
    if (!tok) {
        return;
    }
    token_type tok_type = tok->type;

    switch (tok_type) {
        case OPERATOR:
            print_operator_enum(*(operator_type *)tok->obj);
            break;
        case SCALAR:
            if (tok->obj) { fprintf(stdout, PRISCALAR, 2, *(scalar_t *)tok->obj); }
            break; 
        case MATRIX:
            if (tok->obj) { print_matrix((matrix_t *)tok->obj); }
            break;
        case LPAREN:
            fprintf(stdout, "LPAREN");
            break;
        case RPAREN:
            fprintf(stdout, "RPAREN");
            break;
        case TOKENS_END:
            fprintf(stdout, "TOKENS_END");
            break;
        default:
            break;
    }
}

bool is_operand_token(const token_t *tok) {
    if (!tok || !tok->obj) {
        return false;
    }
    return tok->type == SCALAR || tok->type == MATRIX;
}


bool is_unary_operator_token(const token_t *token) {
    if (!token || token->type != OPERATOR || !token->obj) {
        return false;
    }
    operator_type op = *(operator_type *)token->obj;
    return arity[op] == 1;
}

