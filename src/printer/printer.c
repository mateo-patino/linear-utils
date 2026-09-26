#include "printer/printer.h"
#include "errorprinter.h"

#include <stdio.h>
#include <math.h>


static bool print_scalar(FILE *stream, const scalar_t *scalar, int precision, bool add_newline);
static bool has_no_fractional_part(const scalar_t *scalar);
static size_t get_max_column_width(const matrix_t *matrix);


/*
* Prints a the value pointed at by `scalar` to `stream`. 
*
* It prints a newline character if `add_newline` is true, and the scalar is printed 
* with `precision` decimal points if its fractional part is not zero. If the fractional
* part is zero, the value is printed as an integer.
*/
static bool print_scalar(FILE *stream, const scalar_t *scalar, int precision, bool add_newline) {
    if (!stream || !scalar) {
        return false;
    }

    if (has_no_fractional_part(scalar)) {
        fprintf(stream, "%.0f", *scalar);
    }
    else {
        fprintf(stream, PRISCALAR, precision, *scalar);
    }

    if (add_newline) {
        fprintf(stream, "\n");
    }

    return true;
}


/*
* Returns true if the value at `scalar` has a no fractional part
*/
static bool has_no_fractional_part(const scalar_t *scalar) {
    return scalar && isfinite(*scalar) && *scalar == trunc(*scalar);
}


/*
* Computes the maximum column width required to display all 
* entries in a matrix.
*
* It returns the maximum column width found upon success and SIZE__MAX 
* if an error occurs.
*/
static size_t get_max_column_width(const matrix_t *matrix) {
    if (!matrix || !matrix->data) {
        return 0;
    }

    const scalar_t *data = matrix->data;
    unsigned int nentry = matrix->nrow * matrix->ncol;

    size_t max_len = 0;
    int len = 0;
    for (unsigned int i = 0; i < nentry; i++) {

        if (has_no_fractional_part(data + i)) {
            len = snprintf(NULL, 0, "%.0f", data[i]); 
        }
        else {
            len = snprintf(NULL, 0, PRISCALAR, SCALAR_PRECISION, data[i]);
        }

        if (len < 0) {
            set_error("Could not get maximum column width.");
            return SIZE_MAX;
        }

        max_len = (size_t)len > max_len ? (size_t)len : max_len;
    }

    return max_len; 
}


bool pretty_print(const printout_t *pout) {
    if (!pout) {
        return false;
    }
    
    bool ok = false;
    if (pout->type == SCALAR_PRINTOUT) {
        ok = pretty_print_scalar((const scalar_t *)pout->obj);
    }
    else if (pout->type == MATRIX_PRINTOUT) {
        ok = pretty_print_matrix((const matrix_t *)pout->obj);
    }

    if (!ok) {
        set_error("Could not print output.");
    }

    return ok;
}


bool pretty_print_matrix(const matrix_t *matrix) {
    if (!matrix) {
        return false;
    } 

    size_t col_width = get_max_column_width(matrix);
    if (col_width == SIZE_MAX) {
        set_error("Could not print output.");
        return false;
    }



    return true;
}


bool pretty_print_scalar(const scalar_t *scalar) {
    if (!scalar) {
        return false;
    }

    return print_scalar(stdout, scalar, SCALAR_PRECISION, true);
}

