#include "printer/printer.h"
#include "errorprinter.h"

#include <stdio.h>


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

    /* TODO: add some way to check if the mantissa is completely zero, in which case we print as an integer */
    fprintf(stdout, PRISCALAR "\n", SCALAR_PRECISION, *scalar);

    return true;
}

