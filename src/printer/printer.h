#ifndef PRINTER_H
#define PRINTER_H


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


#endif
