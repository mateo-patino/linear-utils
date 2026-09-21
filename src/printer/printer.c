#include "printer/printer.h"
#include "errorprinter.h"

#include <stdio.h>
#include <math.h>


static bool print_scalar(FILE *stream, const scalar_t *scalar, int precision, bool add_newline);
static bool has_no_fractional_part(const scalar_t *scalar);

static bool has_no_fractional_part(const scalar_t *scalar) {
    return scalar && isfinite(*scalar) && *scalar == trunc(*scalar);
}


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
        set_error("Could not print object.");
    }

    return ok;
}


bool pretty_print_matrix(const matrix_t *matrix) {
    if (!matrix) {
        return false;
    }

    /* TODO */
    return true;
}


bool pretty_print_scalar(const scalar_t *scalar) {
    if (!scalar) {
        return false;
    }

    return print_scalar(stdout, scalar, SCALAR_PRECISION, true);
}

