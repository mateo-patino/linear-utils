#include "test_assert.h"
#include "test_helpers.h"
#include "test_lexer.h"
#include "types/token.h"
#include "lexer/lexer.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>


/*
* Test that all supported aliases are correctly tokenized.
*/
static bool test_operator_lexer_valid(void) {
    for (int i = 0; i < NUM_OP; i++) {
        const char **aliases = operator_alias[i];
        ASSERT_TRUE(aliases != NULL);
        ASSERT_TRUE(aliases[0] != NULL);

        for (int j = 0; aliases[j] != NULL; j++) {
            token_t token;

            ASSERT_TRUE(create_token_from_str(aliases[j], &token) == TOKENS_OK);
            ASSERT_TRUE(token.type == OPERATOR);
            ASSERT_TRUE(token.obj != NULL);
            ASSERT_TRUE(*(operator_type *)token.obj == (operator_type)i);
            ASSERT_TRUE(strcmp(aliases[j], token.user_str) == 0);

            free_token_obj(&token);
        }
    }

    return true;
}

/*
* Test that various valid scalars are being correctly tokenized 
*/
static bool test_scalar_lexer_valid(void) {
    static const char *valid_scalars[] = {
        "0", "42", "-42", "+7", "1.25", "-0.125",
        ".5", "4.", ".0625", "3e2", "6.25e1", "-2.5E-1",
        "1E-6", "2.5e+3", "-8E2", "9.75", "-17.5", "0.0001"
    };
    static const scalar_t expected_scalars[] = {
        0.0, 42.0, -42.0, 7.0, 1.25, -0.125,
        0.5, 4.0, 0.0625, 300.0, 62.5, -0.25,
        0.000001, 2500.0, -800.0, 9.75, -17.5, 0.0001
    };

    for (size_t i = 0; i < ARRAY_LEN(valid_scalars); i++) {
        token_t token;

        ASSERT_TRUE(create_token_from_str(valid_scalars[i], &token) == TOKENS_OK);
        ASSERT_TRUE(token.type == SCALAR);
        ASSERT_TRUE(token.obj != NULL);
        ASSERT_EQ_SCALAR(*(scalar_t *)token.obj, expected_scalars[i]);
        ASSERT_TRUE(strcmp(valid_scalars[i], token.user_str) == 0);

        free_token_obj(&token);
    }

    return true;
}

static const scalar_t expect1[] = { 1.0 };
static const scalar_t expect2[] = { 0.0 };
static const scalar_t expect3[] = { 1.0, 2.0, 3.0, 4.0 };
static const scalar_t expect4[] = { -1.0, -2.0, -3.0, -4.0 };
static const scalar_t expect5[] = { 1.01, 1.002, 1.0003, 1.00004 };
static const scalar_t expect6[] = { 100.0, 200.0 };
static const scalar_t expect7[] = { 67.0, -67.0 };

static const matrix_test_case_t easy_matrix_cases[] = {
    { "1x1 1", 1, 1, expect1 },
    { "1x1 0", 1, 1, expect2 },
    { "2x2 1 2 3 4", 2, 2, expect3 },
    { "2x2 -1 -2 -3 -4", 2, 2, expect4 },
    { "2x2 1.01 1.002 1.0003 1.00004", 2, 2, expect5 },
    { "2x1 100 200", 2, 1, expect6 },
    { "1x2 67 -67", 1, 2, expect7 }
};


/*
* Test that small valid matrices are being correctly tokenized.
*
* We call create_tokens_from_string because matrices are multi-lexeme objects
* and require a different logic flow that single-lexeme tokens like scalars or
* operators. Single-lexeme objects can be fully tokenized by create_token_from_str
* but multi-lexeme objects need to be fed into create_tokens_from_string which
* performs additional logic with the strtok interface that create_token_from_str
* does not.
*/
static bool test_matrix_lexer_easy_valid(void) {
    token_t *token;
    size_t token_count;
    tokens_status tok_status;
    matrix_t *mat;
    const matrix_test_case_t *case_t;
    
    size_t test_count = ARRAY_LEN(easy_matrix_cases);

    for (size_t i = 0; i < test_count; i++) {
        case_t = easy_matrix_cases + i;
        token = create_tokens_from_string(case_t->str, &token_count, &tok_status);

        ASSERT_TRUE(token != NULL);
        ASSERT_TRUE(tok_status == TOKENS_OK);
        ASSERT_TRUE(token_count == 1);

        ASSERT_TRUE(token->type == MATRIX);
        ASSERT_TRUE(token->obj != NULL);
        mat = (matrix_t *)token->obj;

        ASSERT_TRUE(mat->nrow == case_t->nrow && mat->ncol == case_t->ncol);
        ASSERT_EQ_MATDATA(mat->data, case_t->expected_data, case_t->nrow * case_t->ncol);

        fully_free_tokens(token, token_count);
    }

    return true;
}

static const scalar_t medium_expect1[] = {
    1.0, 2.0, 3.0,
    4.0, 5.0, 6.0,
    7.0, 8.0, 9.0
};
static const scalar_t medium_expect2[] = {
    1.5, -2.0, 0.25, 300.0,
    -4.5, 5.0, 0.000001, -800.0,
    9.75, -10.0, 11.125, 12.0
};
static const scalar_t medium_expect3[] = {
    0.0, -1.0, 2.0,
    -3.0, 4.0, -5.0,
    6.0, -7.0, 8.0,
    -9.0, 10.0, -11.0
};
static const scalar_t medium_expect4[] = {
    0.0, 1.0, -1.0, 0.5,
    -0.5, 4.0, 60.0, -0.07
};
static const scalar_t medium_expect5[] = {
    10.0, 20.0, 30.0, 40.0,
    50.0, 60.0, 70.0, 80.0
};
static const scalar_t medium_expect6[] = {
    0.001, -200.0, 3.5,
    0.25, -0.0, 4.0
};
static const scalar_t medium_expect7[] = {
    1.0, 2.0, 3.0, 4.0, 5.0,
    6.0, 7.0, 8.0, 9.0, 10.0
};

static const matrix_test_case_t medium_matrix_cases[] = {
    { "3x3 1 2 3 4 5 6 7 8 9", 3, 3, medium_expect1 },
    { "3x4 1.5 -2 .25 3e2 -4.5 5 1E-6 -8E2 9.75 -10 11.125 12",
      3, 4, medium_expect2 },
    { "4x3 0 -1 2 -3 4 -5 6 -7 8 -9 10 -11", 4, 3, medium_expect3 },
    { " \t1x8 0 +1 -1 .5 -.5 4. 6e1 -7E-2 \n", 1, 8, medium_expect4 },
    { "8x1\t10\n20\t30\n40 50\t60\n70\t80", 8, 1, medium_expect5 },
    { "03x002 1e-3 -2E+2 +3.5 .25 -0.0 4.", 3, 2, medium_expect6 },
    { "2x5\v1\f2\r3\t4\n5 6\v7\f8\r9\t10", 2, 5, medium_expect7 }
};

/*
* Test that medium-sized valid matrices are being correctly tokenized.
*/
static bool test_matrix_lexer_medium_valid(void) {
    token_t *token;
    size_t token_count;
    tokens_status tok_status;
    matrix_t *mat;
    const matrix_test_case_t *case_t;
    
    size_t test_count = ARRAY_LEN(medium_matrix_cases);

    for (size_t i = 0; i < test_count; i++) {
        case_t = medium_matrix_cases + i;
        token = create_tokens_from_string(case_t->str, &token_count, &tok_status);

        ASSERT_TRUE(token != NULL);
        ASSERT_TRUE(tok_status == TOKENS_OK);
        ASSERT_TRUE(token_count == 1);

        ASSERT_TRUE(token->type == MATRIX);
        ASSERT_TRUE(token->obj != NULL);
        mat = (matrix_t *)token->obj;

        ASSERT_TRUE(mat->nrow == case_t->nrow && mat->ncol == case_t->ncol);
        ASSERT_EQ_MATDATA(mat->data, case_t->expected_data, case_t->nrow * case_t->ncol);

        fully_free_tokens(token, token_count);
    }

    return true;
}



static const test_case_t lexer_tests[] = { 
    TEST(test_operator_lexer_valid),
    TEST(test_scalar_lexer_valid),
    TEST(test_matrix_lexer_easy_valid),
    TEST(test_matrix_lexer_medium_valid)
};



uint run_lexer_tests(uint *total, uint *crashes) {
    uint total_lexer_tests = ARRAY_LEN(lexer_tests);
    
    int result;
    int crash_count = 0;
    int pass_count = 0;
    int signum = -1;
    for (uint i = 0; i < total_lexer_tests; i++) {
        result = run_in_sandbox(lexer_tests + i, &signum);

        switch (result) {
            case -1:
                break;
            case 0:
                print_success(lexer_tests + i); 
                pass_count++;
                break;
            case 1:
                /* No printing if tests fails, done in the ASSERT macro */
                break;
            case 2:
                print_crash(lexer_tests + i, signum); 
                crash_count++;
                break;
            default:
                break;
         }
    }

    if (crashes) { *crashes = crash_count; }
    if (total) { *total = total_lexer_tests; }

    return pass_count;
}
