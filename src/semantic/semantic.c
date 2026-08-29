#include "semantic.h"
#include "types/token.h"
#include "types/matrix.h"
#include "errorprinter.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>


/*
* An internal interface for setting a semantic status. Used during recursion by 
* is_semantically_valid_ast
*/
static semantic_status internal_semantic_status = SEMANTIC_OK;
static bool has_error_status = false; /* SEMANTIC_OK is the only NON-error status */

static void set_status(semantic_status status) {
    if (has_error_status) {
        return;
    }
    internal_semantic_status = status;
    if (status != SEMANTIC_OK) {
        has_error_status = true;
    }
}

static semantic_status get_status(void) {
    return internal_semantic_status;
}


static void clear_status(void) {
    internal_semantic_status = SEMANTIC_OK;
    has_error_status = false;
}


/* This function is a helper to set_type_error */
static const char *op_to_str(operator_type op) {
    switch (op) {
        case ADD:
            return "addition";
        case SUB:
            return "subtraction";
        case MUL:
            return "multiplication";
        case DIV:
            return "division";
        case DET:
            return "determinant";
        case RREF:
            return "RREF";
        case INV:
            return "inversion";
        case NUM_OP:
        default:
            return "?";
    }
}


/*
* Writes an error message to the global buffer concerning a type error.
*/
static void set_type_error(operator_type op, io_type left, io_type right) {
    if (has_error()) {
        return;
    }

    bool unary_type_error = false;
    const char *op_str = op_to_str(op);
    const char *left_str, *right_str;

    /* Note we assume the convention that unary operators have a NULL left child and
    * non-NULL right child */
    if (left != SEM_NULL) {
        left_str = left == SEM_SCALAR ? "scalar" : "matrix";
    }
    else {
        unary_type_error = true;
        assert(right != SEM_NULL);
    }
     
    right_str = right == SEM_SCALAR ? "scalar" : "matrix";

    if (unary_type_error) {
        set_error("Operand of type '%s' is incompatible with '%s'.",
                   right_str, op_str);
    }
    else {
        set_error("Operands of type '%s' and '%s' are incompatible with '%s'.",
                  left_str, right_str, op_str); 
    }
}


/*
* Helper to is_valid_ast_helper. It takes an operator type and two operand types
* and returns the expected output type of the operator if it were to operate
* on the two operand types.
*
* It returns SEM_SCALAR or SEM_MATRIX upon success and SEM_NULL upon failure.
*/
static io_type get_output_type(operator_type op, io_type left, io_type right) {
    switch (op) {

        case ADD:
        case SUB:
            if (left == SEM_SCALAR && right == SEM_SCALAR) {
                return SEM_SCALAR;
            }
            else if (left == SEM_MATRIX && right == SEM_MATRIX) {
                return SEM_MATRIX;
            }
            return SEM_NULL;

        case MUL:
            if (left == SEM_SCALAR && right == SEM_SCALAR) {
                return SEM_SCALAR;
            }
            else if (left == SEM_MATRIX && right == SEM_MATRIX) {
                return SEM_MATRIX;
            }
            else if ((left == SEM_SCALAR && right == SEM_MATRIX) || 
                    (left == SEM_MATRIX && right == SEM_SCALAR)) {
                return SEM_MATRIX;
            }
            return SEM_NULL;

        case DIV:
            if (left == SEM_SCALAR && right == SEM_SCALAR) {
                return SEM_SCALAR;
            }
            return SEM_NULL;

        case DET:
            if (left == SEM_NULL && right == SEM_MATRIX) {
                return SEM_SCALAR; 
            }
            return SEM_NULL;
        case RREF:
        case INV:
            /* Note we use the convention that unary operators have NULL left children
            * and non-NULL right children. */
            if (left == SEM_NULL && right == SEM_MATRIX) {
                return SEM_MATRIX;
            }
            return SEM_NULL;

        case NUM_OP:
        default:
            return SEM_NULL;
    }
}



/*
* Returns true if `node` has exactly two operand children.
*/
static bool has_operand_children(const node_t *node) {
    if (!node || !node->left || !node->right) {
        return false;
    }
    const token_t *left_tok = node->left->token;
    const token_t *right_tok = node->right->token;
    if (!is_operand_token(left_tok) || !is_operand_token(right_tok)) {
        return false;
    }
    return true;
}


/*
* Returns true if node has exactly one operand child and it is the right child.
* The left child must be NULL.
*/
static bool has_operand_right_child(const node_t *node) {
    if (!node || !node->right || node->left != NULL) {
        return false;
    }
    const token_t *right_tok = node->right->token;
    if (!is_operand_token(right_tok)) {
        return false;
    }
    return true;
}


/*
* Calls a semantic checker for an operator `op` with operands 
* `left` and `right` for binary operartos and with single operand
* `right` for unary operators.
*
* It returns the semantic status code of the function called.
*/
static semantic_status run_semantic_operand_checks(operator_type op, const token_t *left, const token_t *right) {
    switch (op) {
        case ADD:
            return valid_add_operands(left, right);
        case SUB:
            return valid_sub_operands(left, right);
        case MUL:
            return valid_mul_operands(left, right);
        case DIV:
            return valid_div_operands(left, right);
        case DET:
            return valid_det_operand(right);
        case RREF:
            return valid_rref_operand(right);
        case INV:
            return valid_inv_operand(right);
        case NUM_OP:
        default:
            return SEMANTIC_NULL_ARGS;
    }
}


static bool set_operand_error(semantic_status stat, operator_type op, const token_t *left, const token_t *right) {
    if (has_error()) {
        return false;
    }

    const char *op_str = op_to_str(op);
    bool use_unary_version = false;

    const char *left_str, *right_str;

    /* Check if unary operand error case. `right` should never be NULL */
    assert(right != NULL);
    if (!left && right) {
        left_str = NULL;
        use_unary_version = true;
    }
    else {
        left_str = left->user_str;
    }
    right_str = right->user_str;

    switch (stat) {
        case SEMANTIC_OK:
            return false;

        case SEMANTIC_INCOMPATIBLE_OPERANDS:
            if (use_unary_version) {
                return set_error("'%s' is mathematically incompatible with '%s'.", 
                                   right_str, op_str); 
            }
            return set_error("'%s' and '%s' are mathematically incompatible with %s.", 
                            left_str, right_str, op_str);

        case SEMANTIC_FP_OVERFLOW:
            return set_error("'%s' operation results in floating-point overflow.", op_str);

        case SEMANTIC_INCOMPATIBLE_DIMENSIONS:
            if (use_unary_version) {
                return set_error("'%s' has incompatible dimensions for '%s'operation.", right_str, op_str);
            }
            return set_error("'%s' and '%s' have incompatible dimensions for '%s'.", left_str, right_str, op_str);

        case SEMANTIC_INFINITE_OR_NAN_SCALAR:
            return set_error("Infinite or undefined scalar.");

        case SEMANTIC_INFINITE_OR_NAN_ENTRY:
            return set_error("Infinite or undefined matrix entry.");

        case SEMANTIC_DIVISION_BY_ZERO:
            /* Should never need a unary version */
            return set_error("'%s' and '%s' produce division by zero.", left_str, right_str);
        
        case SEMANTIC_NONSQUARE_MATRIX:
            if (use_unary_version) {
                return set_error("'%s' operation expected a square matrix, got '%s'.",
                                  op_str, right_str);
            }
            return set_error("'%s' operation expected a square matrix, got non-square matrix.",
                              op_str);
        
        case SEMANTIC_EXPECTED_MATRIX:
            return set_error("'%s' operation expected a matrix.", op_str);
        
        case SEMANTIC_NULL_ARGS:
            return set_error("Expected dereferenceable pointer, got NULL.");
        
        default:
            return false;
    }
}


static bool is_valid_scalar_token(const token_t *tok) {
    if (!tok || tok->type != SCALAR || !tok->obj) {
        return false;
    }

    scalar_t val = *(scalar_t *)tok->obj;

    return !is_infinite_or_nan_scalar(val);
}


static bool is_valid_matrix_token(const token_t *tok) {
    if (!tok || tok->type != MATRIX || !tok->obj) {
        return false;
    }
    const matrix_t *mat = (const matrix_t *)tok->obj;
    return has_finite_entries(mat);
}


/*
* This function recursively traverses an AST rooted at `node` and perform two kinds
* of semantic checks on every parent-child-child node triplet: 
*
* 1) do the children nodes have mathematically valid types? For example, if an 
* operator is ADD (add), its children must be either both scalars or both matrices. We check
* for this by recursively determining the output type (matrix or scalar) that each subtree
* in the AST produces and propagating the result upwards.
*
* NEEDSWORK: currently, we use io_type enum values to represent matrix and scalar types. It would
* be great if we could recursively check matrix dimensions as well. For example, the current semantic
* analysis code does not detect an error in this expression "4 mul 2x2 1 1 1 1 + 1x1 1". There is
* obviously a dimension mismatch between the 2x2 and the 1x1 matrices. The current recursive routine 
* only propagates type information up each subtree but it does not carry any matrix information, so the 
* '+' operator sees it's adding two matrices and is happy. It would be useful if the '+' operator knew
* that adding a 2x2 matrix to a 1x1 matrix is impossible, and so an error is raised. This feature
* would likely require replacing the io_type enum with a struct that packs type and matrix dimension info
* together and propagating structs through the recursion.
* 
* 2) For the deepest operator nodes (the nodes that are parents to the user-provided operands), 
* are the user-provided operands mathematically valid? In particular, do they have the right
* types, matrix dimensions, are all entries or scalars finite, etc.
*
*/
static io_type is_valid_ast_helper(const node_t *node) {
    if (!node) {
        return SEM_NULL;
    }

    const token_t *token = node->token;
    assert(token != NULL && token->obj != NULL);

    /*
    * Check if node points to a scalar or matrix. These would be operands directly provided
    * by the user. is_valid_scalar_token and is_valid_matrix_token only check that these operands
    * have finite (non-INFINITY and non-NAN) values, not whether they are mathematically valid
    * for subsequent operators up the tree. Whether they're the right type is checked in "TYPE 
    * CHECKS" and whether they have the right dimensions, do not result in a foreseeable overflow, 
    * etc. is done in OPERAND CHECKS.
    *
    */
    if (token->type == SCALAR) {

        /* Check the scalar is not NaN or inf */
        if (!is_valid_scalar_token(token)) {
            set_error("Infinite or undefined scalar '%s'", token->user_str);
            set_status(SEMANTIC_INFINITE_OR_NAN_SCALAR);
            return SEM_NULL;
        }

        return SEM_SCALAR;
    }
    else if (token->type == MATRIX) {

        /* Check the matrix's entries are not NaN or inf */
        if (!is_valid_matrix_token(token)) {
            set_error("Infinite or undefined matrix entry in '%s'", token->user_str);
            set_status(SEMANTIC_INFINITE_OR_NAN_ENTRY);
            return SEM_NULL;
        }

        return SEM_MATRIX;
    }

    /* If node is not a scalar or a matrix, it must be an operator, so recurse */ 
    assert(token->type == OPERATOR);
    io_type left_type = is_valid_ast_helper(node->left);
    io_type right_type = is_valid_ast_helper(node->right);


    /* 
    * TYPE CHECKS: Check left and right types match the expected input types. Write to
    * global error buffer if not. 
    */
    operator_type op = *(operator_type *)token->obj;
    if (!are_valid_input_types(op, left_type, right_type)) {
        set_type_error(op, left_type, right_type);
        set_status(SEMANTIC_INCOMPATIBLE_OPERANDS);
    }

    /* 
    * OPERAND CHECKS: check if `node` is at the second-to-deepest level 
    * (i.e. both of its children are operands or one is NULL and the other is not), 
    * and if so, do operand checks. We only do these checks at the bottom of the 
    * tree because the operands provided by the user are immediately available 
    * without any linear algebra computations.
    */
    semantic_status stat;
    const token_t *left_token, *right_token;
    if (has_operand_children(node)) {

        left_token = node->left->token;
        right_token = node->right->token;
        stat = run_semantic_operand_checks(op, left_token, right_token);

        if (stat != SEMANTIC_OK && !has_error()) {
            set_operand_error(stat, op, left_token, right_token);
            set_status(stat);
        }
    }
    /* Unary operator, so left child is NULL and only operand is in the right child */
    else if (has_operand_right_child(node)) {

        right_token = node->right->token;
        stat = run_semantic_operand_checks(op, NULL, right_token);

        if (stat != SEMANTIC_OK && !has_error()) {
            set_operand_error(stat, op, NULL, right_token);
            set_status(stat);
        }
    }

    /* Complete tree traversal regardless of error, if any */
    return get_output_type(op, left_type, right_type); 
}


/* This is the only function in this module (and its helper) that shall set 
* errors to the global buffer. It will do so via set_type_error and set_operand_error. */
semantic_status is_semantically_valid_ast(const ast_t *ast, io_type *output_type) {
    if (!ast || !ast->root) {
        return SEMANTIC_NULL_ARGS;
    }

    /* Clear status from previous calls */
    clear_status();

    io_type final_output_type = is_valid_ast_helper(ast->root); 
    if (output_type) { *output_type = final_output_type; }

    return get_status();
}


bool are_valid_input_types(operator_type op, io_type left, io_type right) {

    switch (op) {

        /* Add, subtract, and multiply can be done with scalars and matrices */
        case ADD:
        case SUB:
            return (left == SEM_SCALAR && right == SEM_SCALAR) ||
                   (left == SEM_MATRIX && right == SEM_MATRIX);
        case MUL:
            return (left == right && left == SEM_SCALAR) ||
                   (left == right && left == SEM_MATRIX) ||
                   ((left == SEM_SCALAR && right == SEM_MATRIX) ||
                    (left == SEM_MATRIX && right == SEM_SCALAR));

        /* Division can only be done with scalars */
        case DIV:
            return left == SEM_SCALAR && right == SEM_SCALAR;

        /* Determinants, rref, and inverses can only be done with matrices */
        case DET:
        case RREF:
        case INV:
            return left == SEM_NULL && right == SEM_MATRIX;

        /* These shouldn't happen */
        case NUM_OP:
        default:
            return false;
    }
}


bool is_scalar_add_overflow(scalar_t a, scalar_t b) {
    return isinf(a + b);
}


bool is_scalar_mul_overflow(scalar_t a, scalar_t b) {
    return isinf(a * b);
}


/* Does not catch NaNs, only overflows (infinities) */
bool is_scalar_div_overflow(scalar_t a, scalar_t b) {
    return isinf(a / b);
}


/* Does catch NaNs and infinities, which are both ways different systems
* represent division by zero */
bool is_division_by_zero(scalar_t a, scalar_t b) {
    return !isfinite(a / b);
}


bool is_infinite_or_nan_scalar(scalar_t a) {
    return !isfinite(a);
}


bool has_finite_entries(const matrix_t *a) {
    if (!a) {
        return false;
    }   

    scalar_t *entry = a->data;
    for (unsigned int i = 0; i < a->nrow * a->ncol; i++) {
        if (!isfinite(entry[i])) {
            return false;
        }
    }
    return true;
}


semantic_status valid_add_operands(const token_t *a, const token_t *b) {
    if (!a || !b) {
        return SEMANTIC_NULL_ARGS;
    }

    if (a->type == SCALAR && b->type == SCALAR) {
        const scalar_t *sca = (const scalar_t *)a->obj;
        const scalar_t *lar = (const scalar_t *)b->obj;

        if (!sca || !lar) {
            return SEMANTIC_NULL_ARGS;
        }
        else if (is_infinite_or_nan_scalar(*sca) || is_infinite_or_nan_scalar(*lar)) {
            return SEMANTIC_INFINITE_OR_NAN_SCALAR;
        }
        else if (is_scalar_add_overflow(*sca, *lar)) {
            return SEMANTIC_FP_OVERFLOW;
        }

        return SEMANTIC_OK; 
    }
    else if (a->type == MATRIX && b->type == MATRIX) {
        const matrix_t *mat = (const matrix_t *)a->obj;
        const matrix_t *rix = (const matrix_t *)b->obj;

        if (!mat || !rix) {
            return SEMANTIC_NULL_ARGS;
        }
        else if (!have_equal_dimensions(mat, rix)) {
            return SEMANTIC_INCOMPATIBLE_DIMENSIONS;
        }
        /*
         * Below we check that each entry in the matrices is not infinite. Note that entry-wise addition
         * can result in overflow even if all entries are finite, but this overflow is detected inside 
         * the algebra module during calculation.
         */
        else if (!has_finite_entries(mat) || !has_finite_entries(rix)) {
            return SEMANTIC_INFINITE_OR_NAN_ENTRY;
        }

        return SEMANTIC_OK; 
    }
    
    return SEMANTIC_INCOMPATIBLE_OPERANDS;    
}


semantic_status valid_sub_operands(const token_t *a, const token_t *b) {
    /* 
    * Addition and subtraction have the same conditions to be valid operations. is_add_overflow and 
    * has_finite_entries use isinf, which checks for negative and positive infinity, so the entire 
    * valid_add_operands logic can be reused here. 
    */
    return valid_add_operands(a, b);
}


semantic_status valid_mul_operands(const token_t *first, const token_t *second) {
    if (!first || !second) { 
        return SEMANTIC_NULL_ARGS;
    }

    /* Scalar times a scalar */
    if (first->type == SCALAR && second->type == SCALAR) { 
        const scalar_t *sca = (const scalar_t *)first->obj;
        const scalar_t *lar = (const scalar_t *)second->obj;

        if (!sca || !lar) {
            return SEMANTIC_NULL_ARGS;
        }
        else if (is_infinite_or_nan_scalar(*sca) || is_infinite_or_nan_scalar(*lar)) {
            return SEMANTIC_INFINITE_OR_NAN_SCALAR;
        }
        else if (is_scalar_mul_overflow(*sca, *lar)) {
            return SEMANTIC_FP_OVERFLOW;
        }

        return SEMANTIC_OK;
    }
    /* Matrix multiplication */
    else if (first->type == MATRIX && second->type == MATRIX) {
        const matrix_t *mat = (const matrix_t *)first->obj;
        const matrix_t *rix = (const matrix_t *)second->obj;

        if (!mat || !rix) {
            return SEMANTIC_NULL_ARGS;
        }
        /*
        * Like in addition, below we ONLY check that entries are finite. Matrix multiplication
        * can still produce overflow, but we let the algebra library detect that error so as 
        * avoid any long matrix math in this semantics layer.
        */
        else if (!has_finite_entries(mat) || !has_finite_entries(rix)) {
            return SEMANTIC_INFINITE_OR_NAN_ENTRY;
        }
        /*
        * NOTE: this function assumes the order of matrix multiplication is mat * rix.
        * Hence, we check that the number of columns in mat equals the number of rows in 
        * rix.
        */
        else if (mat->ncol != rix->nrow) {
            return SEMANTIC_INCOMPATIBLE_DIMENSIONS;
        }

        return SEMANTIC_OK;
    }

    /* Scalar multiplication. NEEDSWORK: condense these two else-ifs into one? */
    const scalar_t *scalar;
    const matrix_t *matrix;
    if (first->type == SCALAR && second->type == MATRIX) {
       scalar = (scalar_t *)first->obj;
       matrix = (matrix_t *)second->obj;
       goto CHECK_SCALAR_MULTIPLICATION;
    }
    else if (first->type == MATRIX && second->type == SCALAR) {
        scalar = (scalar_t *)second->obj;
        matrix = (matrix_t *)first->obj;
        goto CHECK_SCALAR_MULTIPLICATION;
    }

    /* If we reached this, first and second are not valid operands*/
    return SEMANTIC_INCOMPATIBLE_OPERANDS;

CHECK_SCALAR_MULTIPLICATION:
    if (is_infinite_or_nan_scalar(*scalar)) {
        return SEMANTIC_INFINITE_OR_NAN_SCALAR;
    }
    else if (!has_finite_entries(matrix)) {
        return SEMANTIC_INFINITE_OR_NAN_ENTRY;
    }
    return SEMANTIC_OK;
}


semantic_status valid_div_operands(const token_t *a, const token_t *b) {
    if (!a || !b) {
        return SEMANTIC_NULL_ARGS;
    }

    if (a->type != SCALAR || b->type != SCALAR) {
        return SEMANTIC_INCOMPATIBLE_OPERANDS;
    }

    const scalar_t *sca = (const scalar_t *)a->obj;
    const scalar_t *lar = (const scalar_t *)b->obj;

    if (!sca || !lar) {
        return SEMANTIC_NULL_ARGS;
    }
    else if (is_infinite_or_nan_scalar(*sca) || is_infinite_or_nan_scalar(*lar)) {
        return SEMANTIC_INFINITE_OR_NAN_SCALAR;  
    }
    /*
     * NOTE: this function assumes that the order of divison is sca / lar.
     * Below we check that sca / lar does not produce positive or negative infinity.
     */
    else if (is_scalar_div_overflow(*sca, *lar)) {
        return SEMANTIC_FP_OVERFLOW;
    }
    /* 
    * Below we check that sca / lar does not produce infinity or NaN, which are both
    * ways in which floating-point division-by-zero is represented in different systems.
    */
    else if (is_division_by_zero(*sca, *lar)) {
        return SEMANTIC_DIVISION_BY_ZERO;
    }

    return SEMANTIC_OK;
} 


semantic_status valid_det_operand(const token_t *a) {
    if (!a) {
        return SEMANTIC_NULL_ARGS;
    } 

    if (a->type != MATRIX) {
        return SEMANTIC_EXPECTED_MATRIX;
    }

    const matrix_t *mat = (const matrix_t *)a->obj;

    if (!mat) {
        return SEMANTIC_NULL_ARGS;
    }
    else if (!has_finite_entries(mat)) {
        return SEMANTIC_INFINITE_OR_NAN_ENTRY;
    }
    /* Determinants are only defined for square matrices */
    else if (mat->ncol != mat->nrow) {
        return SEMANTIC_NONSQUARE_MATRIX;
    }

    return SEMANTIC_OK;
}


semantic_status valid_rref_operand(const token_t *a) {
    if (!a) {
        return SEMANTIC_NULL_ARGS;
    }

    if (a->type != MATRIX) {
        return SEMANTIC_EXPECTED_MATRIX;
    }

    const matrix_t *mat = (const matrix_t *)a->obj;

    if (!mat) {
        return SEMANTIC_NULL_ARGS;
    }

    return has_finite_entries(mat) ? SEMANTIC_OK : SEMANTIC_INFINITE_OR_NAN_ENTRY;
}


semantic_status valid_inv_operand(const token_t *a) {
    if (!a) {
        return SEMANTIC_NULL_ARGS;
    }

    if (a->type != MATRIX) {
        return SEMANTIC_EXPECTED_MATRIX;
    }
   
    const matrix_t *mat = (const matrix_t *)a->obj;
    if (!mat) {
        return SEMANTIC_NULL_ARGS;
    }

    /*
    * Only square matrices can be inverted. Generalized (one-sided) inverses that arise
    * from inverting a non-square matrix are not currently supported.
    */
    if (mat->ncol != mat->nrow) {
        return SEMANTIC_NONSQUARE_MATRIX;
    }
    else if (!has_finite_entries(mat)) {
        return SEMANTIC_INFINITE_OR_NAN_ENTRY;
    }

    /* It is tempting to check the determinant of the matrix, but this implies calling
    * the algebra library, and this module shall only perform high-level checks and avoid
    * any heavy math. If the matrix is not invertible, the algebra functions will detect it
    * when their time comes */

    return SEMANTIC_OK;
}
