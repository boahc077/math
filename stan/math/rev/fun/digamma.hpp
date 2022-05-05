#ifndef STAN_MATH_REV_FUN_DIGAMMA_HPP
#define STAN_MATH_REV_FUN_DIGAMMA_HPP

#include <stan/math/rev/meta.hpp>
#include <stan/math/rev/core.hpp>
#include <stan/math/prim/fun/trigamma.hpp>
#include <stan/math/prim/fun/digamma.hpp>

namespace stan {
namespace math {

/**
 * Return the derivative of the log gamma function
 * at the specified value.
 *
 * @param[in] a argument
 * @return derivative of log gamma function at argument
 */
inline var digamma(const var& a) {
  return make_callback_var(digamma(a.val()), [a](auto& vi) {
    a.adj() += vi.adj() * trigamma(a.val());
  });
}

/**
 * Return the elementwise derivative of the log gamma function
 * at the given input vector
 *
 * @tparam T a `var_value` with inner Eigen type
 * @param[in] a vector
 * @return elementwise derivative of log gamma function
 */
template <typename T, require_rev_matrix_t<T>* = nullptr>
inline auto digamma(const T& x) {
  auto x_arena = to_arena(x);
  return make_callback_rev_matrix<T>(
      x_arena.val()
          .array()
          .unaryExpr([](auto& x) { return digamma(x); })
          .matrix()
          .eval(),
      [x_arena](auto&& vi) mutable {
        x_arena.adj().array()
            += vi.adj().array() * x_arena.val().array().unaryExpr([](auto&& x) {
                 return trigamma(x);
               });
      });
}

}  // namespace math
}  // namespace stan
#endif
