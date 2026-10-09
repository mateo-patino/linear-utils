#include "linalg/test_la_helpers.hpp"


/* C inteface */
#include "linalg/view.h"
#include "linalg/scalar.h"

/*
* Initialize a view with dimensions (nrow, ncol) to default values.
*/
matrix_view::matrix_view(size_t nrow, size_t ncol) {

    /* Entries are initialized to zero */
    data_.resize(nrow * ncol);

    view_.data = data_.data();
    view_.nrow = nrow;
    view_.ncol = ncol;
    view_.row_stride = ncol;
    view_.column_stride = 1;
}


/*
* Initializes a view with dimensions (nrow, ncol) to the values in `entries`
* Accordingly, `entries` must have nrow * ncol entries.
*
* The values from `entries` are deep copied.
*/
matrix_view::matrix_view(size_t nrow, size_t ncol, std::vector<scalar>& entries) {  

    /* Automatically performs a deep copy of the std::vector's data */
    data_ = entries;
    
    view_.data = data_.data();
    view_.nrow = nrow;
    view_.ncol = ncol;
    view_.row_stride = ncol;
    view_.column_stride = 1;
}


/* 
* Returns a mutable pointer to the internal matrixv_t struct
*/
const matrixv_t *matrix_view::get_view() const {
    return &view_;
}


/*
* Returns a mutable pointer to the internal matrixv_t struct
*/
matrixv_t *matrix_view::get_view() {
    return &view_;
}
