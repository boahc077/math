#ifndef STAN_MATH_PRIM_FUN_HYPERGEOMETRIC_PFQ_HPP
#define STAN_MATH_PRIM_FUN_HYPERGEOMETRIC_PFQ_HPP

#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/err/check_not_nan.hpp>
#include <stan/math/prim/err/check_finite.hpp>
#include <boost/math/special_functions/hypergeometric_pFq.hpp>

namespace stan {
namespace math {

/**
 * Returns the generalised hypergeometric function applied to the
 * input arguments:
 * \f$_pF_q(a_1,...,a_p;b_1,...,b_q;z)\f$
 *
 * This function is not intended to be exposed to end users, only
 * used for p & q values that are stable with the grad_pFq
 * implementation.
 *
 * See 'grad_pFq.hpp' for the derivatives wrt each parameter
 *
 * @tparam ArithVec1 An Eigen vector with arithmetic scalar type
 * @tparam ArithVec2 An Eigen vector with arithmetic scalar type
 * @tparam Arithmetic An arithmetic scalar
 * @param[in] a Vector of 'a' arguments to function
 * @param[in] b Vector of 'b' arguments to function
 * @param[in] z Scalar z argument
 * @return Generalised hypergeometric function
 */
template <typename ArithVec1, typename ArithVec2, typename Arithmetic,
          require_all_eigen_st<std::is_arithmetic, ArithVec1, ArithVec2>* = nullptr,
          require_arithmetic_t<Arithmetic>* = nullptr>
return_type_t<ArithVec1, ArithVec2, Arithmetic> hypergeometric_pFq(const ArithVec1& a, const ArithVec2& b,
                                             const Arithmetic& z) {
  plain_type_t<ArithVec1> a_ref = a;
  plain_type_t<ArithVec2> b_ref = b;
  check_finite("hypergeometric_pFq", "a", a_ref);
  check_finite("hypergeometric_pFq", "b", b_ref);
  check_finite("hypergeometric_pFq", "z", z);

  check_not_nan("hypergeometric_pFq", "a", a_ref);
  check_not_nan("hypergeometric_pFq", "b", b_ref);
  check_not_nan("hypergeometric_pFq", "z", z);

  bool condition_1 = (a_ref.size() > (b_ref.size() + 1)) && (z != 0);
  bool condition_2 = (a_ref.size() == (b_ref.size() + 1)) && (std::fabs(z) > 1);

  if (condition_1 || condition_2) {
    std::stringstream msg;
    msg << "hypergeometric function pFq does not meet convergence "
        << "conditions with given arguments. "
        << "a: " << a_ref << ", b: " << b_ref << ", "
        << ", z: " << z;
    throw std::domain_error(msg.str());
  }

  return boost::math::hypergeometric_pFq(
      std::vector<double>(a_ref.data(), a_ref.data() + a_ref.size()),
      std::vector<double>(b_ref.data(), b_ref.data() + b_ref.size()), z);
}
}  // namespace math
}  // namespace stan
#endif
