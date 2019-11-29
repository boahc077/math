#ifndef STAN_MATH_PRIM_MAT_FUN_LOG10_HPP
#define STAN_MATH_PRIM_MAT_FUN_LOG10_HPP

#include <stan/math/prim/mat/fun/Eigen.hpp>
#include <stan/math/prim/mat/vectorize/apply_scalar_unary.hpp>
#include <stan/math/prim/meta.hpp>
#include <cmath>

namespace stan {
namespace math {

/**
 * Structure to wrap log10() so it can be vectorized.
 * @param x Variable.
 * @tparam T Variable type.
 * @return Log base-10 of x.
 */
struct log10_fun {
  template <typename T>
  static inline T fun(const T& x) {
    using std::log10;
    return log10(x);
  }
};

/**
 * Vectorized version of log10().
 * @param x Container.
 * @tparam T Container type.
 * @return Log base-10 applied to each value in x.
 */
template <typename T, typename = require_not_eigen_vt<std::is_arithmetic, T>>
inline auto log10(const T& x) {
  return apply_scalar_unary<log10_fun, T>::apply(x);
}

/**
 * Version of log10() that accepts Eigen Matrix ar matrix expressions.
 * @tparam Derived derived type of x
 * @param x Matrix or matrix expression
 * @return Arc cosine of each variable in the container, in radians.
 */
template <typename Derived,
          typename = require_eigen_vt<std::is_arithmetic, Derived>>
inline auto log10(const Eigen::MatrixBase<Derived>& x) {
  return x.derived().array().log10().matrix();
}

}  // namespace math
}  // namespace stan

#endif
