#ifndef PRINTER_H
#define PRINTER_H


#include "types/matrix.h"

#include <stdio.h>

/*
* Types of data that the printer can display to the screen.
* See `printout_t` struct to understand how it's to be interpreted.
*/
typedef enum {
    SCALAR_PRINTOUT,
    MATRIX_PRINTOUT
} printout_type;


/*
* printout_t structs serve as wrappers around scalar_t and matrix_t
* pointers, as well as char pointers.
*
* If `obj` points to a scalar_t, type will contain SCALAR_PRINTOUT. If
* `obj` points to a matrix_t, type will contain MATRIX_PRINTOUT.
*/
typedef struct {
    printout_type type;
    void *obj;
} printout_t;


/*
* Frees the `obj` member of a printout_t struct
*/
void free_printout_obj(printout_t *pout);


/*
* Pretty print `pout->obj` according to its type.
*/
bool pretty_print(const printout_t *pout);


/*
* Pretty print a matrix_t token to stdout.
*
* Entries in the matrix are right-alignede within each column. The width of
* any column is determined by the maximum string width of any entry in that
* column.
*/
bool pretty_print_matrix(FILE *stream, const matrix_t *matrix);

/*
* Pretty print a scalar_t token to stdout.
*/
bool pretty_print_scalar(FILE *stream, const scalar_t *scalar);


#endif
