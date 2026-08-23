#ifndef LEXER_H
#define LEXER_H

#include "types/token.h"
#include "types/matrix.h"

#include <stdlib.h>
#include <stdbool.h>

/*
* Tokens status model. TOKENS_OK means user input was correctly tokenized.
*/
typedef enum {
    TOKENS_OK,
    TOKENS_INVALID_ARG,
    TOKENS_INVALID_MATRIX,
    TOKENS_MEMORY_FAILURE 
} tokens_status;


/*
* Creates a matrix token given the number of entries in the matrix (nrow * ncol). 
* Internally, it calls strtok(NULL, TOKEN_DELIM) nrow*ncol times to consume the
* entries of the matrix. Hence, it should only be called after strtok has been fed 
* an initial string.
*/
tokens_status create_matrix_token(token_t *token, unsigned int nrow, unsigned int ncol);


/*
* Tokenizes 'str' into an operator token.
*/
tokens_status create_operator_token(const char *str, operator_type op_type, token_t *dst);

/*
* Tokenizes 'str' into a LPAREN or RPAREN token. The new token is written to 'dst'.
* A tokens_status code is returned to indicate the success or failure of the tokenization.
*/
tokens_status create_parens_token(const char *str, token_t *dst);


/*
* Creates a scalar token given a (valid) scalar value. The new token is written to dst.
*/
tokens_status create_scalar_token(const char *str, scalar_t scalar, token_t *dst);


/*
* Takes a constant string and produces a token array terminated with a token of type TOKENS_END. 
* It returns a pointer to an TOKENS_END-terminated array of token_t upon sucess and NULL upon 
* failure.
*
* If 'token_count' is not NULL, the number of tokens generated is written there.
*
* If 'status' is not NULL, the status of tokenization is written there. This status
* will be TOKENS_OK if the string was correctly tokenized. Otherwise, the corresponding
* error code is written there.
*/
token_t *create_tokens_from_string(const char *str, size_t *token_count, tokens_status *status);


/*
* Reads a string and writes its corresponding token_t object to 'dst' upon success. TOKENS_OK is 
* returned. If the string cannot be tokenized, the corresponding error status code is returned and nothing
* is written to 'dst'.
*
* Note that this function only tokenizes single lexemes (i.e. `str` must not have any whitespaces). Multi-lexeme
* objects like matrices cannot be passed as a single string into this function. A matrix '2x2 1 2 3 4' is
* tokenized by first calling strtok and retrieving '2x2'; this gets fed into create_token_from_str, which 
* uses is_matrix_marker to recognize that '2x2' signals the beginning of a matrix and consumes the next 4 tokens
* using strtok's global status. If `str` is '2x2 1 2 3 4', is_matrix_marker fails and the matrix is not read.
*
* The token_t 'obj' pointer points to a heap address and must be freed by the caller.
*/
tokens_status create_token_from_str(const char *str, token_t *dst);


/*
* Returns true if 'str' is a valid operator label. If 'type' is not NULL, the operator type is
* written there.
*
* The array of alias strings for each operator must be NULL terminated.
*/
bool is_operator(const char *str, operator_type *type);


/*
* Returns true if 'str' is a valid scalar. If 'val' is not NULL, the scalar value is written there.
* It is NON-REPORTING should 'str' not be a valid scalar. As such, it does not call str_to_scalar_t,
* which is reporting.
*/
bool is_scalar(const char *str, scalar_t *val);

/*
* Tokens must be separated from each other by at least one of these characters below.
*/
#define TOKEN_DELIM " \t\n\v\f\r"

/*
* The first token array allocated will have this size, and the lexer will enlarge the array if needed.
*
* See create_tokens_from_string to see how it's used.
*/
#define TOKENS_ARR_SIZE 24


#endif
