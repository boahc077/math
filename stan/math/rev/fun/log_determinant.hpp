#ifndef STAN_MATH_REV_FUN_LOG_DETERMINANT_HPP
#define STAN_MATH_REV_FUN_LOG_DETERMINANT_HPP

#include <stan/math/rev/meta.hpp>
#include <stan/math/rev/core.hpp>
#include <stan/math/rev/fun/typedefs.hpp>
#include <stan/math/rev/functor/reverse_pass_callback.hpp>
#include <stan/math/rev/core/arena_matrix.hpp>
#include <stan/math/prim/err.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/math/prim/fun/typedefs.hpp>

namespace stan {
namespace math {

template <typename T, require_eigen_vt<is_var, T>* = nullptr>
inline var log_determinant(const T& m) {
  check_square("determinant", "m", m);

  if (m.size() == 0) {
    return 0.0;
  }

  arena_t<T> arena_m = m;

  auto m_lu = arena_m.val().partialPivLu();

  auto arena_m_inv_transpose = to_arena(m_lu.inverse().transpose());

  var log_det = std::log(std::abs(m_lu.determinant()));

  reverse_pass_callback([arena_m, log_det, arena_m_inv_transpose]() mutable {
    arena_m.adj() += log_det.adj() * arena_m_inv_transpose;
  });

  return log_det;
}

}  // namespace math
}  // namespace stan
#endif
