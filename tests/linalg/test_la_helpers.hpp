#ifndef TEST_LA_HELPERS_HPP
#define TEST_LA_HELPERS_HPP

/* C interfaces */
#include "linalg/view.h"
#include "linalg/scalar.h"

#include <vector>


/*
* This class is a small wrapper around the C matrix view (matrixv_t) struct.
* 
* The matrixv_t struct is stored in each class object, so it does not need
* to be freed manually. The internal scalar *data pointer of each struct is managed
* with a std::vector that frees this pointer automatically when the wrapper's
* destructor runs, so the view's data doesn't need to be manually freed.
*/
class matrix_view {

    private:
        matrixv_t view_;
        std::vector<scalar> data_;

    public:

        matrix_view(size_t nrow, size_t ncol);
        matrix_view(size_t nrow, size_t ncol, std::vector<scalar>& entries);

        const matrixv_t *get_view() const;
        matrixv_t *get_view();
};


#endif
