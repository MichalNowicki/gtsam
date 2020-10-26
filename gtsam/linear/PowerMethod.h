/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010-2019, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file   PowerMethod.h
 * @date   Sept 2020
 * @author Jing Wu
 * @brief  Power method for fast eigenvalue and eigenvector
 * computation
 */

#pragma once

#include <gtsam/base/Matrix.h>
#include <gtsam/base/Vector.h>

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <random>
#include <vector>

namespace gtsam {

using Sparse = Eigen::SparseMatrix<double>;

/* ************************************************************************* */
/// MINIMUM EIGENVALUE COMPUTATIONS

// Template argument Operator just needs multiplication operator
template <class Operator>
class PowerMethod {
 protected:
  // Const reference to an externally-held matrix whose minimum-eigenvalue we
  // want to compute
  const Operator &A_;

  const int dim_;  // dimension of Matrix A

  size_t nrIterations_;  // number of iterations

  double ritzValue_;   // Ritz eigenvalue
  Vector ritzVector_;  // Ritz eigenvector

 public:
  // Constructor
  explicit PowerMethod(const Operator &A,
                       const boost::optional<Vector> initial = boost::none)
      : A_(A), dim_(A.rows()), nrIterations_(0) {
    Vector x0;
    x0 = initial ? initial.get() : Vector::Random(dim_);
    x0.normalize();

    // initialize Ritz eigen value
    ritzValue_ = 0.0;

    // initialize Ritz eigen vector
    ritzVector_ = Vector::Zero(dim_);
    ritzVector_ = powerIteration(x0);
  }

  // Run power iteration on the vector, and return A * x / || A * x ||
  Vector powerIteration(const Vector &x) const {
    Vector y = A_ * x;
    y.normalize();
    return y;
  }

  // Run power iteration on the vector, and return A * x / || A * x ||
  Vector powerIteration() const { return powerIteration(ritzVector_); }

  // After Perform power iteration on a single Ritz value, check if the Ritz
  // residual for the current Ritz pair is less than the required convergence
  // tol, return true if yes, else false
  bool converged(double tol) const {
    const Vector x = ritzVector_;
    // store the Ritz eigen value
    const double ritzValue = x.dot(A_ * x);
    const double error = (A_ * x - ritzValue * x).norm();
    return error < tol;
  }

  // Return the number of iterations
  size_t nrIterations() const { return nrIterations_; }

  // Start the power/accelerated iteration, after performing the
  // power/accelerated iteration, calculate the ritz error, repeat this
  // operation until the ritz error converge. If converged return true, else
  // false.
  bool compute(size_t maxIterations, double tol) {
    // Starting
    bool isConverged = false;

    for (size_t i = 0; i < maxIterations && !isConverged; i++) {
      ++nrIterations_;
      // update the ritzVector after power iteration
      ritzVector_ = powerIteration();
      // update the ritzValue 
      ritzValue_ = ritzVector_.dot(A_ * ritzVector_);
      isConverged = converged(tol);
    }

    return isConverged;
  }

  // Return the eigenvalue
  double eigenvalue() const { return ritzValue_; }

  // Return the eigenvector
  Vector eigenvector() const { return ritzVector_; }
};

}  // namespace gtsam
