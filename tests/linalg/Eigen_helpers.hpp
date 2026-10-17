#ifndef EIGEN_HELPERS_HPP
#define EIGEN_HELPERS_HPP

#include "linalg/view.h"
#include <linalg/eigen/Eigen/Core>


/*
* Returns an Eigen::MatrixXd from a matrixv_t struct.
*/
Eigen::MatrixXd get_Eigen_matrix(const matrixv_t *matrix);


#endif
