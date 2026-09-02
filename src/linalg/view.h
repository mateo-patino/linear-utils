#ifndef VIEW_H
#define VIEW_H

#include "linalg/scalar.h"

#include <stdlib.h>


/*
* The core matrix structure in this library is the  matrix view
* struct `matrixv_t`. It encompasses all the information required
* to read data from and write data to a matrix's `data` array.
*/
typedef struct {
    scalar *data;
    size_t ncol;
    size_t nrow;
    size_t column_stride;
    size_t row_stride;
} matrixv_t;


#endif
