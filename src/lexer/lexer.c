#include "lexer.h"
#include "types/matrix.h"
#include "errorprinter.h"
#include "arena.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>


/*
* Returns a scalar_t according to 'str' upon success. 
*
* If no conversion is done, 0 is returned and the error is 
* written to 'status' if not NULL.
*/
scalar_t str_to_scalar_t(const char *str, matrix_status *status) {
    if (!str || *str == '\0') {
        goto RETURN_UPON_ERROR;
    }

    char *endptr;
    errno = 0;
    double val = strtod(str, &endptr);

    if (endptr == str || *endptr != '\0' || errno != 0) {
        set_error("Invalid scalar '%s'", str);
        goto RETURN_UPON_ERROR;
    }
    if (status) { *status = MATRIX_OK; }
    return (scalar_t)val;

RETURN_UPON_ERROR:
    if (status) { *status = MATRIX_INVALID_ENTRY; }
    return 0;
}


/*
* Calls strtok(NULL, TOKEN_DELIM) 'n' times and returns a pointer to a
* heap-allocated array of 'n' scalar_t values upon success.
*
* It calls set_error and returns NULL upon failure. No memory is allocated
* upon failure.
*/
static scalar_t *get_matrix_entries_from_str(unsigned int n, tokens_status *status) {
    scalar_t *data = malloc(n*sizeof(scalar_t));
    if (!data) {
        if (status) { *status = TOKENS_MEMORY_FAILURE; }
        return NULL;
    }
    matrix_status mat_st;
    char *str;
    scalar_t val;
    for (unsigned int i = 0; i < n; i++) {
        str = strtok(NULL, TOKEN_DELIM);
        
        /* If strtok returns NULL, not enough entries exist */
        if (!str) {
            set_error("Expected %u matrix entries, got %u", n, i);
            if (status) { *status = TOKENS_INVALID_MATRIX; }
            goto RETURN_UPON_ERROR;
        }

        val = str_to_scalar_t(str, &mat_st);
        if (mat_st != MATRIX_OK) {
            set_error("Invalid entry '%s'", str);
            if (status) { *status = TOKENS_INVALID_MATRIX; }
            goto RETURN_UPON_ERROR;
        }
        data[i] = val;
    }
    if (status) { *status = TOKENS_OK; }
    return data;

RETURN_UPON_ERROR:
    free(data);
    return NULL;
}


/*
* Returns true if 'str' has the form "axb" where "a" and "b" are positive unsigned integers
* and false otherwise.
*/
static bool is_matrix_marker(const char *str, unsigned int *nrow, unsigned int *ncol) {
    if (!str || *str == '\0' || *str == 'x') {
        return false;
    }
    const char *original = str;
    char *endptr;
    errno = 0;
    unsigned long a = strtoul(str, &endptr, 10);

    /* endptr must point to 'x' */
    if (*endptr != 'x' || errno == ERANGE) {
        return false;
    }
    
    /* Move endptr one up. It must not be NUL if the string is valid */
    if (*endptr++ == '\0') {
        return false;
    }

    /* Set str to the 'b' value in 'axb' */
    str = endptr;
    errno = 0;
    unsigned long b = strtoul(str, &endptr, 10);
    if (endptr == str || *endptr != '\0' || errno == ERANGE) {
        return false;
    }

    if (!a || !b) {
        set_error("Dimensions cannot be zero '%s'", original);
        return false;
    }

    /* Check for uint overflow. a*b must fit inside of an unsigned int. */
    if (a > UINT_MAX || b > UINT_MAX || a > UINT_MAX / b) {
        set_error("Dimensions are too large '%s'", original);
        return false;
    }

    if (nrow) { *nrow = (unsigned int)a; }
    if (ncol) { *ncol = (unsigned int)b; }

    return true;
}


/* 
* Returns a pointer to a reallocated memory region twice as big as current_size upon success.
* Returns 'tokens' upon failure so caller can free the memory at this address. Since the return
* value cannot signal a failure, caller MUST check errno == ENOMEM for failure. 
*
* Note that the caller's size variable is updated accordingly.
*/
static token_t *resize_tokens(token_t *tokens, size_t *current_size) {
    if (!tokens || !current_size) {
        return NULL;
    }
    size_t new_size = 2 * (*current_size);
    token_t *tmp = realloc(tokens, new_size*sizeof(token_t));
    if (!tmp) {
        /* Note: memory at 'tokens' is still valid if realloc fails, so caller can free it */
        return tokens;
    }
    *current_size = new_size;
    return tmp;
}


/*
* This function assumes marker_str points to the first byte of a marker lexeme in m_str 
* (from create_tokens_from_string), and it reads `nentry` entries starting at marker_str.
* It keeps track of how many entries it has read by counting NULs, so this function MUST
* be called after the entire matrix string has been modified by strtok and NULs have
* been put after each string representing a matrix entry. It must also be called only after 
* verifying that at least `nentry` entries are available for reading.
*
* NEEDSWORK: this function is a hack to be able to create a user_str for matrix
* tokens. Any simpler way to do this that doesn't involve making as many fragile assumptions 
* across functions and memory objects would be welcomed.
*/
static const char *create_matrix_user_str(const char *marker_str, size_t nentry) {
    if (!marker_str || !nentry) {
        return NULL;
    }

    /* A heap buffer where we'll write the user_str byte by byte */
    arena_t *arena = create_arena(KiB(1));

    /* pinter that will be moved each iteration and offset of the ' ' after last object read */
    const char *ptr = marker_str;
    size_t last_space_offset = 0;
 
    /* + 1 corresponds to the marker lexeme */
    size_t nobjects = nentry + 1;
    size_t objects_read = 0;
    char space = ' ';
    bool seen_object = false;

    while (objects_read < nobjects) {
        seen_object = false;
        
        /* Write bytes to buffer until finding a '\0' */
        while (*ptr != '\0') {
            seen_object = true;
            awrite(ptr++, sizeof(char), _Alignof(char), arena);
        }

        /* Add a whitespace between every object read */
        if (seen_object) {
            last_space_offset = awrite(&space, sizeof(char), _Alignof(char), arena);
            objects_read++;
        }
        ptr++;
    }
    
    /* Write a NUL terminator at the end of the string */
    set_char_at('\0', last_space_offset, arena); 
    
    char *user_str = strdup(arena->start);
    free_arena(arena);

    return user_str;
}



tokens_status create_matrix_token(const char *marker_str, token_t *token, unsigned int nrow, unsigned int ncol) {
    if (nrow <= 0 || ncol <= 0 || !token || !marker_str) {
        return TOKENS_INVALID_ARG;
    }
    
    /* Call strtok(NULL, ...) to get the entries that follow the marker_str */
    tokens_status status;
    scalar_t *data = get_matrix_entries_from_str(nrow*ncol, &status);
    if (!data || status != TOKENS_OK) {
        set_error("Cannot get matrix entries.");
        return status;
    }

    matrix_t *mat = init_matrix(data, nrow, ncol);
    if (!mat) {
        free(data);
        return TOKENS_MEMORY_FAILURE;
    }
    token->type = MATRIX;
    token->obj = mat;
    token->user_str = create_matrix_user_str(marker_str, nrow * ncol);

    return TOKENS_OK;
}


tokens_status create_operator_token(const char *str, operator_type op_type, token_t *dst) {
    if (!dst || !str) { 
        return TOKENS_INVALID_ARG;
    }
    operator_type *op = malloc(sizeof(operator_type));
    if (!op) {
        return TOKENS_MEMORY_FAILURE;
    }
    *op = op_type;
    dst->type = OPERATOR;
    dst->obj = op;
    dst->user_str = strdup(str);
    
    if (!dst->user_str) {
        return TOKENS_MEMORY_FAILURE;
    }
    
    return TOKENS_OK;
}


tokens_status create_parens_token(const char *str, token_t *dst) {
    if (!dst || !str) {
        return TOKENS_INVALID_ARG;
    }
    assert(strlen(str) == 1);
    
    char c = str[0];
    switch (c) {
        case '(':
            dst->type = LPAREN;
            break;
        case ')':
            dst->type = RPAREN;
            break;
        default:
            set_error("Invalid parenthesis '%c'", c);
            return TOKENS_INVALID_ARG;
    }
    dst->obj = NULL;
    dst->user_str = strdup(str);

    if (!dst->user_str) {
        return TOKENS_MEMORY_FAILURE;
    }

    return TOKENS_OK;
}


tokens_status create_scalar_token(const char *str, scalar_t scalar, token_t *dst) {
    if (!dst || !str) {
        return TOKENS_INVALID_ARG;
    }
    scalar_t *obj = malloc(sizeof(scalar_t));
    if (!obj) {
        return TOKENS_MEMORY_FAILURE;
    }
    dst->type = SCALAR;
    *obj = scalar;
    dst->obj = obj;
    dst->user_str = strdup(str);

    if (!dst->user_str) {
        return TOKENS_MEMORY_FAILURE;
    }

    return TOKENS_OK;
}


token_t *create_tokens_from_string(const char *str, size_t *token_count, tokens_status *status) {
    if (!str || *str == '\0') {
        return NULL;
    }

    /* Make copy of 'str' */
    char *m_str = strdup(str);
    if (!m_str) {
        if (status) { *status = TOKENS_MEMORY_FAILURE; }
        return NULL;
    }

    /* Allocate token_t array */
    size_t size = TOKENS_ARR_SIZE;
    token_t *tokens = malloc(size*sizeof(token_t)); /* Tokens live in the heap! */
    if (!tokens) {
        set_error("realloc() failed"); 
        if (status) { *status = TOKENS_MEMORY_FAILURE; }
        free(m_str);
        return NULL;
    }
    
    size_t tc = 0;
    char *tok_str;
    tokens_status st;

    /* Consume first token  */
    tok_str = strtok(m_str, TOKEN_DELIM);
    if ((st = create_token_from_str(tok_str, tokens)) != TOKENS_OK) {
        if (tok_str) {
            set_error("Invalid argument '%s'", tok_str);
        }
        if (status) { *status = st; }
        goto FREE_UPON_ERROR_1;
    }
    tc++;

    /* Tokenize the rest of the string */
    while ((tok_str = strtok(NULL, TOKEN_DELIM)) != NULL) {

        /* Resize if token count has reached current max size */
        if (tc == size) {
            tokens = resize_tokens(tokens, &size);
            if (errno == ENOMEM) {
                set_error("realloc() failed"); 
                if (status) { *status = TOKENS_MEMORY_FAILURE; }
                goto FREE_UPON_ERROR_2; 
            }
        }

        /* Tokenize tok_str */
        if ((st = create_token_from_str(tok_str, tokens + tc)) != TOKENS_OK) {
            if (tok_str) {
                set_error("Invalid argument '%s'", tok_str);
            }
            if (status) { *status = st; }
            goto FREE_UPON_ERROR_2;
        }
        tc++;
    }

    /* Terminate the tokens array with the marker type TOKENS_END */
    if (tc == size) {
        errno = 0;
        tokens = resize_tokens(tokens, &size);
        if (errno == ENOMEM) {
            set_error("realloc() failed"); 
            if (status) { *status = TOKENS_MEMORY_FAILURE; }
            goto FREE_UPON_ERROR_2;
        }
    }
    tokens[tc].type = TOKENS_END;
    tokens[tc].obj = NULL;
    /* We do not increment tc here because tc counts non-END tokens */

    if (token_count) { *token_count = tc; }
    if (status) { *status = TOKENS_OK; }
    free(m_str);

    return tokens;

FREE_UPON_ERROR_1:
    free(tokens);
    free(m_str);
    return NULL;

FREE_UPON_ERROR_2:
    free_token_objs_by_count(tokens, tc);
    free(tokens);
    free(m_str);
    return NULL;
}


tokens_status create_token_from_str(const char *str, token_t *dst) {
    if (!str || *str == '\0') {
        return TOKENS_INVALID_ARG;
    }

    /* in-out arguments */
    tokens_status matrix_token_status;
    unsigned int nrow = 0, ncol = 0;
    operator_type op_type;
    scalar_t val;

    /* Tokenize parenthesis */
    if (!strcmp(str, ")") || !strcmp(str, "(")) {
        return create_parens_token(str, dst);
    }
    /* Tokenize scalars */
    else if (is_scalar(str, &val)) {
        return create_scalar_token(str, val, dst); 
    }
    /* Tokenize operators */
    else if (is_operator(str, &op_type)) {
        return create_operator_token(str, op_type, dst);
    } 
    /* Tokenize matrices */
    else if (is_matrix_marker(str, &nrow, &ncol)) {
        if ((matrix_token_status = create_matrix_token(str, dst, nrow, ncol)) != TOKENS_OK) {
            set_error("Invalid matrix starting at '%s'", str);
            return matrix_token_status;
        } 
        return TOKENS_OK;
    }

    set_error("Invalid argument '%s'", str);
    return TOKENS_INVALID_ARG;
}


bool is_operator(const char *str, operator_type *type) {
    if (!str || *str == '\0') {
        return false;
    } 
    const char *alias;
    for (int i = 0; i < NUM_OP; i++) {
        const char **aliases = operator_alias[i];
        int j = 0;
        while ((alias = aliases[j++])) {
            if (!strcmp(str, alias)) {
                if (type) { *type = (operator_type)i; }
                return true;
            }
        }
    }
    return false;
}


bool is_scalar(const char *str, scalar_t *val) {
    if (!str || *str == '\0') {
        return false;
    }
    
    /* "NAN" and "INF" tokenize to NAN and INFINITY respectively */
    if (!strcmp(str, "NAN")) {
        if (val) { *val = NAN; }
        return true;
    }
    else if (!strcmp(str, "INF")) {
        if (val) { *val = INFINITY; }
        return true;
    }
    
    char *endptr;
    errno = 0;
    double scalar = strtod(str, &endptr);

    if (endptr == str || *endptr != '\0' || errno != 0) {
        return false;
    }

    if (val) { *val = (scalar_t)scalar; }
    return true;
}
