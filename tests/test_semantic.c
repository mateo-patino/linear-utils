#include "test_assert.h"
#include "test_helpers.h"
#include "test_semantic.h"
#include "types/token.h"
#include "types/matrix.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/semantic.h"
#include "errorprinter.h"

#include <assert.h>
#include <math.h>


/*
* Struct for arranging the objects that must remain allocated while an AST is
* semantically checked. AST nodes point into the tokens array.
*/
typedef struct {
    token_t *tokens;
    size_t token_count;
    ast_t *ast;
} semantic_fixture_t;


/*
* Creates an AST from a valid input string. The tokens and AST must be freed
* by calling free_semantic_fixture.
*/
static semantic_fixture_t create_semantic_fixture(const char *expr) {
    semantic_fixture_t fixture = { NULL, 0, NULL };
    tokens_status tok_status;
    parse_status parse_stat;

    fixture.tokens = create_tokens_from_string(expr, &fixture.token_count, &tok_status);
    assert(tok_status == TOKENS_OK);

    fixture.ast = create_ast_from_tokens(fixture.tokens, fixture.token_count, &parse_stat);
    assert(parse_stat == PARSE_OK);

    return fixture;
}


static void free_semantic_fixture(semantic_fixture_t *fixture) {
    if (!fixture) {
        return;
    }
    fully_free_ast(fixture->ast);
    fully_free_tokens(fixture->tokens, fixture->token_count);
}


static bool test_semantic_operand_types(void) {
    typedef struct {
        operator_type op;
        io_type left;
        io_type right;
        bool expected;
    } input_type_case_t;

    static const input_type_case_t cases[] = {
        { ADD, SEM_SCALAR, SEM_SCALAR, true },
        { ADD, SEM_MATRIX, SEM_MATRIX, true },
        { ADD, SEM_SCALAR, SEM_MATRIX, false },
        { SUB, SEM_SCALAR, SEM_SCALAR, true },
        { SUB, SEM_MATRIX, SEM_MATRIX, true },
        { SUB, SEM_MATRIX, SEM_SCALAR, false },
        { MUL, SEM_SCALAR, SEM_SCALAR, true },
        { MUL, SEM_MATRIX, SEM_MATRIX, true },
        { MUL, SEM_SCALAR, SEM_MATRIX, true },
        { MUL, SEM_MATRIX, SEM_SCALAR, true },
        { MUL, SEM_NULL, SEM_MATRIX, false },
        { DIV, SEM_SCALAR, SEM_SCALAR, true },
        { DIV, SEM_MATRIX, SEM_SCALAR, false },
        { DET, SEM_NULL, SEM_MATRIX, true },
        { DET, SEM_NULL, SEM_SCALAR, false },
        { RREF, SEM_NULL, SEM_MATRIX, true },
        { RREF, SEM_MATRIX, SEM_MATRIX, false },
        { INV, SEM_NULL, SEM_MATRIX, true },
        { INV, SEM_NULL, SEM_SCALAR, false },
        { NUM_OP, SEM_NULL, SEM_NULL, false }
    };

    for (size_t i = 0; i < ARRAY_LEN(cases); i++) {
        ASSERT_TRUE(are_valid_input_types(cases[i].op, cases[i].left, cases[i].right)
                    == cases[i].expected);
    }

    return true;
}


static bool test_valid_semantic_easy(void) {
    semantic_fixture_t fixture;
    semantic_status st;
    io_type output_type;

    clear_error();
    fixture = create_semantic_fixture("1 + 2");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_SCALAR);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("2x2 1 2 3 4 + 2x2 5 6 7 8");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_MATRIX);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("3 * 2x2 1 2 3 4");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_MATRIX);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("2x2 1 2 3 4 * 3");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_MATRIX);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("det 2x2 1 2 3 4");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_SCALAR);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("rref 2x3 1 2 3 4 5 6");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_MATRIX);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("inv 2x2 1 2 3 4");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_MATRIX);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    return true;
}


static bool test_valid_semantic_medium(void) {
    semantic_fixture_t fixture;
    semantic_status st;
    io_type output_type;

    clear_error();
    fixture = create_semantic_fixture("det ( inv 2x2 1 2 3 4 ) + 1");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_SCALAR);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("rref ( 2x2 1 2 3 4 * inv 2x2 1 0 0 1 )");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_MATRIX);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("( 2 * 2x2 1 2 3 4 ) + 2x2 5 6 7 8");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_MATRIX);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    return true;
}


static bool test_invalid_semantic_easy(void) {
    semantic_fixture_t fixture;
    semantic_status st;

    clear_error();
    fixture = create_semantic_fixture("1 + 2x2 1 2 3 4");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INCOMPATIBLE_OPERANDS);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("2x2 1 2 3 4 / 2");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INCOMPATIBLE_OPERANDS);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("2x2 1 2 3 4 + 1x1 1");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INCOMPATIBLE_DIMENSIONS);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("2x3 1 2 3 4 5 6 * 2x2 1 2 3 4");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INCOMPATIBLE_DIMENSIONS);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("det 2x3 1 2 3 4 5 6");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_NONSQUARE_MATRIX);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("inv 2x3 1 2 3 4 5 6");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_NONSQUARE_MATRIX);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    return true;
}


static bool test_invalid_semantic_medium(void) {
    scalar_t scalar_max = SCALAR_MAX;
    scalar_t scalar_two = 2;
    scalar_t scalar_nan = NAN;
    scalar_t matrix_entries[] = { 1, INFINITY };
    matrix_t matrix = { 1, 2, matrix_entries };

    token_t max_token = { SCALAR, &scalar_max, NULL };
    token_t two_token = { SCALAR, &scalar_two, NULL };
    token_t nan_token = { SCALAR, &scalar_nan, NULL };
    token_t matrix_token = { MATRIX, &matrix, NULL };

    ASSERT_TRUE(valid_add_operands(&max_token, &max_token) == SEMANTIC_FP_OVERFLOW);
    ASSERT_TRUE(valid_mul_operands(&max_token, &two_token) == SEMANTIC_FP_OVERFLOW);
    ASSERT_TRUE(valid_add_operands(&nan_token, &two_token) == SEMANTIC_INFINITE_OR_NAN_SCALAR);
    ASSERT_TRUE(valid_rref_operand(&matrix_token) == SEMANTIC_INFINITE_OR_NAN_ENTRY);
    ASSERT_TRUE(valid_mul_operands(&two_token, &matrix_token) == SEMANTIC_INFINITE_OR_NAN_ENTRY);
    
    /*
     * NEEDSWORK: these tests above can be replaced with simpler versions using create_semantic_fixture
     * from a string. Writing NAN or INF in the input string produces scalar tokens with NAN or INFINITY 
     * values. Many new tests can be added using this lexer feature now too.
     */

    semantic_fixture_t fixture;
    semantic_status st;
    
    clear_error();
    fixture = create_semantic_fixture("INF");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INFINITE_OR_NAN_SCALAR);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("NAN");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INFINITE_OR_NAN_SCALAR);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("1 + 2 + INF");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INFINITE_OR_NAN_SCALAR);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("1 + 2 + NAN");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INFINITE_OR_NAN_SCALAR);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("1 + 2 + det INF");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INFINITE_OR_NAN_SCALAR);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);
    
    clear_error();
    fixture = create_semantic_fixture("1 + 2 + det NAN");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INFINITE_OR_NAN_SCALAR);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("rref ( inv NAN ) + 1 + 2");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INFINITE_OR_NAN_SCALAR);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);


    return true;
}


/* Test that a semantic error deep inside the tree is handled correctly */
static bool test_invalid_semantic_hard(void) {
    semantic_fixture_t fixture;
    semantic_status st;

    /* Should raise a type error */
    clear_error();
    fixture = create_semantic_fixture("inv ( det 100 mul 2x2 1 2 3 4 )");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INCOMPATIBLE_OPERANDS);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    /* Should raise a type error */
    clear_error();
    fixture = create_semantic_fixture("2 mul 2x2 1 2 3 4 + ( 4 mul 2x2 1 2 3 4 ) + ( 8 mul rref ( inv ( 1 + 2x2 0 1 1 0 ) ) )");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INCOMPATIBLE_OPERANDS);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    /* Should raise an operand error */
    clear_error();
    fixture = create_semantic_fixture("det ( 10 mul ( 2x2 1 2 3 4 mul 2x2 5 6 7 8 ) + 100 mul ( inv 2x2 1 1 1 1 + rref ( 2x2 1 2 3 4 + 1x1 1 ) ) )");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INCOMPATIBLE_DIMENSIONS);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    /* Should raise an operand error */
    clear_error();
    fixture = create_semantic_fixture("det ( inv ( 2x2 4 7 2 5 ) mul ( 3x3 1 2 0 3 4 5 6 7 9 + 3x3 9 0 1 8 2 3 7 6 4 ) - NAN mul ( det ( rref ( 2x2 6 1 4 3 ) ) ) )");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INFINITE_OR_NAN_SCALAR);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    return true;
}


static bool test_semantic_status_reset(void) {
    semantic_fixture_t fixture;
    semantic_status st;
    io_type output_type;

    clear_error();
    fixture = create_semantic_fixture("1 + 2x2 1 2 3 4");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_INCOMPATIBLE_OPERANDS);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("1 + 1");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_SCALAR);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("inv 2x3 1 2 3 4 5 6");
    st = is_semantically_valid_ast(fixture.ast, NULL);
    ASSERT_TRUE(st == SEMANTIC_NONSQUARE_MATRIX);
    ASSERT_TRUE(has_error() == true);
    free_semantic_fixture(&fixture);

    clear_error();
    fixture = create_semantic_fixture("det 2x2 1 0 0 1");
    st = is_semantically_valid_ast(fixture.ast, &output_type);
    ASSERT_TRUE(st == SEMANTIC_OK);
    ASSERT_TRUE(output_type == SEM_SCALAR);
    ASSERT_TRUE(has_error() == false);
    free_semantic_fixture(&fixture);

    return true;
}


static const test_case_t semantic_tests[] = {
    TEST(test_semantic_operand_types),
    TEST(test_valid_semantic_easy),
    TEST(test_valid_semantic_medium),
    TEST(test_invalid_semantic_easy),
    TEST(test_invalid_semantic_medium),
    TEST(test_invalid_semantic_hard),
    TEST(test_semantic_status_reset)
};


uint run_semantic_tests(uint *total, uint *crashes) {
    uint total_semantic_tests = ARRAY_LEN(semantic_tests);

    int result;
    int crash_count = 0;
    int pass_count = 0;
    int signum = -1;
    for (uint i = 0; i < total_semantic_tests; i++) {
        result = run_in_sandbox(semantic_tests + i, &signum);

        switch (result) {
            case -1:
                break;
            case 0:
                print_success(semantic_tests + i);
                pass_count++;
                break;
            case 1:
                /* No printing if tests fails, done in the ASSERT macro */
                break;
            case 2:
                print_crash(semantic_tests + i, signum);
                crash_count++;
                break;
            default:
                break;
        }
    }

    if (crashes) { *crashes = crash_count; }
    if (total) { *total = total_semantic_tests; }

    return pass_count;
}
