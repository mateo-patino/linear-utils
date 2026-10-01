#include "printer/printer.h"
#include "errorprinter.h"

#include <stdio.h>
#include <math.h>
#include <limits.h>


static bool print_scalar(FILE *stream, const scalar_t *scalar, int precision, bool add_newline);
static bool has_no_fractional_part(const scalar_t *scalar);
static size_t get_column_width(const matrix_t *matrix, unsigned int j);
static size_t get_scalar_strlen(const scalar_t *scalar);



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
static size_t get_column_width(const matrix_t *matrix, unsigned int j) {
    if (!matrix || !matrix->data) {
        return 0;
    }

    const scalar_t *data = matrix->data;
    const scalar_t *entry = NULL;

    size_t max_len = 0, entry_strlen = 0;
    unsigned int nrow = matrix->nrow;

    for (unsigned int i = 0; i < nrow; i++) {
        entry = &data[i * nrow + j];
        entry_strlen = get_scalar_strlen(entry); /* Could fail, but error handling here is hardly useful */

        max_len = entry_strlen > max_len ? entry_strlen : max_len;
    }

    return max_len; 
}


/*
* Returns the string length of a scalar upon success and SIZE_MAX
* if snprintf fails.
*/
static size_t get_scalar_strlen(const scalar_t *scalar) {
    if (!scalar) {
        return SIZE_MAX;
    }
    
    int len = has_no_fractional_part(scalar) ? snprintf(NULL, 0, "%.0f", *scalar) : snprintf(NULL, 0, PRISCALAR, SCALAR_PRECISION, *scalar);
    if (len < 0) {
        return SIZE_MAX;
    }

    return (size_t)len;
}


bool pretty_print(const printout_t *pout) {
    if (!pout) {
        return false;
    }
    
    bool ok = false;
    if (pout->type == SCALAR_PRINTOUT) {
        ok = pretty_print_scalar(stdout, (const scalar_t *)pout->obj);
    }
    else if (pout->type == MATRIX_PRINTOUT) {
        ok = pretty_print_matrix(stdout, (const matrix_t *)pout->obj);
    }

    if (!ok) {
        set_error("Could not print output.");
    }

    return ok;
}


bool pretty_print_matrix(FILE *stream, const matrix_t *matrix) {
    if (!matrix) {
        return false;
    } 

    const scalar_t *data = matrix->data, *entry = NULL;
    unsigned int nrow = matrix->nrow, ncol = matrix->ncol;
    size_t padding = 0;

    for (unsigned int i = 0; i < nrow; i++) { 
        fprintf(stream, "| ");

        for (unsigned int j = 0; j < ncol; j++) {
            entry = &data[i * ncol + j];

            /* Print leading whitespaces to right-align value inside column */
            padding = get_column_width(matrix, j) - get_scalar_strlen(entry);
            for (size_t k = 0; k < padding; k++) {
                fputc(' ', stream);
            }

            print_scalar(stream, entry, SCALAR_PRECISION, false);

            fputc(' ', stream);
        }

        fprintf(stream, "|\n");
    }

    /*
    * NEEDSWORK: In the routine above, every time we move to the next column, we
    * compute its width. It would be a faster design to precompute the column widths
    * and store them outside the for loop so that the padding loop can query the 
    * width that corresponds to column j. It's a small optimization but nevertheless
    * possible.
    */

    return true;
}


bool pretty_print_scalar(FILE *stream, const scalar_t *scalar) {
    if (!scalar) {
        return false;
    }

    return print_scalar(stream, scalar, SCALAR_PRECISION, true);
}

