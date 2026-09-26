/*
* Entry point
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

#include "types/token.h"
#include "types/matrix.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/semantic.h"
#include "evaluator/evaluator.h"
#include "printer/printer.h"
#include "errorprinter.h"


/*
* Checks if an error message exists in the errorprinter buffer and prints it. If
* a message from errorprinter was printed, it returns true.
*
* If no message existed in the buffer, false is returned/
*/
static bool print_error_message(void) {
    if (!has_error()) {
        return false;
    }
    fprintf(stderr, "Error: %s\n", get_error());
    return true;
}


/*
* Pretty prints an array of 'tc' tokens. Used for debugging and development only.
*/
static void inspect_tokens(const token_t *tokens, size_t tc) {
    fputc('[', stdout);
    for (size_t i = 0; i < tc-1; i++) {
         print_token(tokens + i);
         fprintf(stdout, ", ");
    }
    print_token(tokens + tc - 1);
    fprintf(stdout, "]\n");
}


int main(int argc, char **argv) {

    /* Input must be a single string */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [expression]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Assume argv[1] is the sole input string */
    char *expr = argv[1];
    
    /* Create tokens array. This array is on the heap and must be freed. */
    tokens_status tok_status = TOKENS_OK;
    size_t token_count = 0;
    token_t *tokens;

    if (!(tokens = create_tokens_from_string(expr, &token_count, &tok_status)) 
        || tok_status != TOKENS_OK) {
        if (!print_error_message()) {
            fprintf(stderr, "Error: Invalid expression. %s\n", expr);
        }
        goto FREE_TOKENS_FAIL;
    }
    else if (!token_count) {
        fprintf(stderr, "Error: Missing expression.\n");
        goto FREE_TOKENS_FAIL;
    }
    /* Quite unlikely but technically possible */
    else if (token_count > INT_MAX) {
        fprintf(stderr, "Error: Expression too large. Only %i logical tokens are supported.\n", 
                INT_MAX);
        goto FREE_TOKENS_FAIL;
    }

    /* Parse the tokens to create an AST */
    parse_status parse_status = PARSE_OK;
    ast_t *ast = create_ast_from_tokens(tokens, token_count, &parse_status); 
    if (parse_status != PARSE_OK) {
        if (!print_error_message()) {
            fprintf(stderr, "Error: Invalid expression. %s\n", expr);
        }
        goto FREE_AST_AND_TOKENS_FAIL;
    }

    /* Perform semantic (math) checks on the AST */
    io_type out_type;
    semantic_status sem_status = is_semantically_valid_ast(ast, &out_type);
    if (sem_status != SEMANTIC_OK) {
        if (!print_error_message()) {
            fprintf(stderr, "Error: Mathematically invalid expression.\n");
        }
        goto FREE_AST_AND_TOKENS_FAIL;
    }

    /* Evaluate the AST. `out` must be freed. */
    eval_status evaluate_status = EVAL_OK;
    result_t *final_result = evaluate_ast(ast, &evaluate_status);
    
    if (evaluate_status != EVAL_OK || !final_result) {
        assert(evaluate_status != EVAL_OK && !final_result);
        fprintf(stderr, "Error: expression evaluation failed. %s\n", get_error());
        goto FREE_AST_AND_TOKENS_FAIL;
    }

    /*
    * Convert the result_t struct returned by the evaluator into a printout_t struct
    * that the printer can display to the terminal.
    */
    printout_t pout;
    if (final_result->type == MATRIX_RES) {
        pout.type = MATRIX_PRINTOUT;
        pout.obj = init_matrix_token_from_view((const matrixv_t *)final_result->obj);
    }
    else {
        assert(final_result->type == SCALAR_RES);
        pout.type = SCALAR_PRINTOUT;
        pout.obj = init_scalar_token_from_linalg_scalar((const scalar *)final_result->obj);
    }

    if (!pout.obj) {
        fprintf(stderr, "Error: could not allocate memory.\n");
        free_result(final_result);
        goto FREE_AST_AND_TOKENS_FAIL;
    }

    /* TODO: now feed pout to the printer! and don't forget to free it*/ 
    if (!pretty_print(&pout) && !print_error_message()) {
        fprintf(stderr, "Error: could not print object.\n");
        free_result(final_result);
        goto FREE_AST_AND_TOKENS_FAIL; /* Fix this crappy cleanup routine */
    }


    /* TODO: improve your failure cleanup/goto routine. It's currently getting really awkward. */
    (void)inspect_tokens;
    free_result(final_result);
    fully_free_tokens(tokens, token_count);
    fully_free_ast(ast);

    return EXIT_SUCCESS;

FREE_TOKENS_FAIL:
    fully_free_tokens(tokens, token_count);
    return EXIT_FAILURE;


FREE_AST_AND_TOKENS_FAIL:
    fully_free_tokens(tokens, token_count);
    fully_free_ast(ast);
    return EXIT_FAILURE;
}
