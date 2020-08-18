#ifndef STAN_MATH_PRIM_PROB_EXP_MOD_NORMAL_LCDF_HPP
#define STAN_MATH_PRIM_PROB_EXP_MOD_NORMAL_LCDF_HPP

#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/err.hpp>
#include <stan/math/prim/fun/constants.hpp>
#include <stan/math/prim/fun/erf.hpp>
#include <stan/math/prim/fun/exp.hpp>
#include <stan/math/prim/fun/inv.hpp>
#include <stan/math/prim/fun/is_inf.hpp>
#include <stan/math/prim/fun/log.hpp>
#include <stan/math/prim/fun/max_size.hpp>
#include <stan/math/prim/fun/size.hpp>
#include <stan/math/prim/fun/size_zero.hpp>
#include <stan/math/prim/fun/square.hpp>
#include <stan/math/prim/fun/value_of.hpp>
#include <stan/math/prim/functor/operands_and_partials.hpp>
#include <cmath>

namespace stan {
namespace math {

template <typename T_y, typename T_loc, typename T_scale, typename T_inv_scale>
return_type_t<T_y, T_loc, T_scale, T_inv_scale> exp_mod_normal_lcdf(
    const T_y& y, const T_loc& mu, const T_scale& sigma,
    const T_inv_scale& lambda) {
  using T_partials_return = partials_return_t<T_y, T_loc, T_scale, T_inv_scale>;
  using T_y_ref = ref_type_if_t<is_constant<T_y>::value, T_y>;
  using T_mu_ref = ref_type_if_t<is_constant<T_loc>::value, T_loc>;
  using T_sigma_ref = ref_type_if_t<is_constant<T_scale>::value, T_scale>;
  using T_lambda_ref
      = ref_type_if_t<is_constant<T_inv_scale>::value, T_inv_scale>;
  static const char* function = "exp_mod_normal_lcdf";
  check_consistent_sizes(function, "Random variable", y, "Location parameter",
                         mu, "Scale parameter", sigma, "Inv_scale paramter",
                         lambda);
  T_y_ref y_ref = y;
  T_mu_ref mu_ref = mu;
  T_sigma_ref sigma_ref = sigma;
  T_lambda_ref lambda_ref = lambda;

  const auto& y_col = as_column_vector_or_scalar(y_ref);
  const auto& mu_col = as_column_vector_or_scalar(mu_ref);
  const auto& sigma_col = as_column_vector_or_scalar(sigma_ref);
  const auto& lambda_col = as_column_vector_or_scalar(lambda_ref);

  const auto& y_arr = as_array_or_scalar(y_col);
  const auto& mu_arr = as_array_or_scalar(mu_col);
  const auto& sigma_arr = as_array_or_scalar(sigma_col);
  const auto& lambda_arr = as_array_or_scalar(lambda_col);

  ref_type_t<decltype(value_of(y_arr))> y_val = value_of(y_arr);
  ref_type_t<decltype(value_of(mu_arr))> mu_val = value_of(mu_arr);
  ref_type_t<decltype(value_of(sigma_arr))> sigma_val = value_of(sigma_arr);
  ref_type_t<decltype(value_of(lambda_arr))> lambda_val = value_of(lambda_arr);

  check_not_nan(function, "Random variable", y_val);
  check_finite(function, "Location parameter", mu_val);
  check_positive_finite(function, "Scale parameter", sigma_val);
  check_positive_finite(function, "Inv_scale parameter", lambda_val);

  if (size_zero(y, mu, sigma, lambda)) {
    return 0;
  }

  operands_and_partials<T_y_ref, T_mu_ref, T_sigma_ref, T_lambda_ref>
      ops_partials(y_ref, mu_ref, sigma_ref, lambda_ref);

  scalar_seq_view<decltype(y_val)> y_vec(y_val);
  for (size_t n = 0, size_y = stan::math::size(y); n < size_y; n++) {
    if (is_inf(y_vec[n])) {
      return ops_partials.build(y_vec[n] < 0 ? negative_infinity() : 0);
    }
  }

  const auto& inv_sigma
      = to_ref_if<!is_constant_all<T_y, T_loc, T_scale>::value>(inv(sigma_val));
  const auto& diff = to_ref(y_val - mu_val);
  const auto& v = to_ref(lambda_val * sigma_val);
  const auto& scaled_diff = to_ref(diff * INV_SQRT_TWO * inv_sigma);
  const auto& scaled_diff_diff
      = to_ref_if<!is_constant_all<T_y, T_loc, T_scale, T_inv_scale>::value>(
          scaled_diff - v * INV_SQRT_TWO);
  const auto& erf_calc = to_ref(0.5 * (1 + erf(scaled_diff_diff)));

  // we can not have this call as a rvalue in next expression until square is
  // returning expressions.
  const auto& v2 = square(v);
  const auto& exp_term
      = to_ref_if<!is_constant_all<T_y, T_loc, T_scale, T_inv_scale>::value>(
          exp(0.5 * v2 - lambda_val * diff));
  const auto& cdf_n
      = to_ref(0.5 + 0.5 * erf(scaled_diff) - exp_term * erf_calc);

  T_partials_return cdf_log = sum(log(cdf_n));

  if (!is_constant_all<T_y, T_loc, T_scale, T_inv_scale>::value) {
    const auto& exp_term_2
        = to_ref_if<(!is_constant_all<T_y, T_loc, T_scale>::value
                     && !is_constant_all<T_inv_scale>::value)>(
            exp(-square(scaled_diff_diff)));
    if (!is_constant_all<T_y, T_loc, T_scale>::value) {
      constexpr bool need_deriv_refs = !is_constant_all<T_y, T_loc>::value
                                       && !is_constant_all<T_scale>::value;
      const auto& deriv_1
          = to_ref_if<need_deriv_refs>(lambda_val * exp_term * erf_calc);
      const auto& deriv_2 = to_ref_if<need_deriv_refs>(
          INV_SQRT_TWO_PI * exp_term * exp_term_2 * inv_sigma);
      // another case where we need a variable instead of a rvalue
      const auto& exp_sq_scaled_diff = exp(-square(scaled_diff));
      const auto& deriv_3 = to_ref_if<need_deriv_refs>(
          INV_SQRT_TWO_PI * exp_sq_scaled_diff * inv_sigma);
      if (!is_constant_all<T_y, T_loc>::value) {
        const auto& deriv = to_ref_if<(!is_constant_all<T_loc>::value
                                       && !is_constant_all<T_y>::value)>(
            (deriv_1 - deriv_2 + deriv_3) / cdf_n);
        if (!is_constant_all<T_loc>::value) {
          ops_partials.edge2_.partials_ = -deriv;
        }
        if (!is_constant_all<T_y>::value) {
          ops_partials.edge1_.partials_ = std::move(deriv);
        }
      }
      if (!is_constant_all<T_scale>::value) {
        //        std::cout << "inv_sigma: " << std::endl << inv_sigma <<
        //        std::endl << std::endl; std::cout << "exp_term: " << std::endl
        //        << exp_term << std::endl << std::endl; std::cout <<
        //        "exp_term_2: " << std::endl << exp_term_2 << std::endl <<
        //        std::endl; std::cout << "scaled_diff_diff: " << std::endl <<
        //        scaled_diff_diff << std::endl << std::endl; std::cout <<
        //        "scaled_diff: " << std::endl << scaled_diff << std::endl <<
        //        std::endl; std::cout << "deriv_1: " << std::endl << deriv_1 <<
        //        std::endl << std::endl; std::cout << "deriv_2: " << std::endl
        //        << deriv_2 << std::endl << std::endl; std::cout <<
        //        "exp_sq_scaled_diff: " << std::endl << exp_sq_scaled_diff <<
        //        std::endl << std::endl; std::cout << "deriv_3: " << std::endl
        //        << deriv_3 << std::endl << std::endl; std::cout << "v: " <<
        //        std::endl << v << std::endl << std::endl; std::cout <<
        //        "scaled_diff: " << std::endl << scaled_diff << std::endl <<
        //        std::endl; std::cout << "cdf_n: " << std::endl << cdf_n <<
        //        std::endl << std::endl; std::cout << "edge3_.partials_: " <<
        //        std::endl << (-((deriv_1 - deriv_2) * v
        //                                                            + (deriv_3
        //                                                            - deriv_2)
        //                                                            *
        //                                                            scaled_diff
        //                                                            *
        //                                                            SQRT_TWO)
        //                                                          / cdf_n) <<
        //                                                          std::endl <<
        //                                                          std::endl;
        ops_partials.edge3_.partials_
            = -((deriv_1 - deriv_2) * v
                + (deriv_3 - deriv_2) * scaled_diff * SQRT_TWO)
              / cdf_n;
        //        std::cout << "edge3_.partials_: " << std::endl <<
        //        ops_partials.edge3_.partials_[0] << std::endl << std::endl;
        //        std::cout << "inv_sigma: " << std::endl << inv_sigma <<
        //        std::endl << std::endl; std::cout << "exp_term: " << std::endl
        //        << exp_term << std::endl << std::endl; std::cout <<
        //        "exp_term_2: " << std::endl << exp_term_2 << std::endl <<
        //        std::endl; std::cout << "scaled_diff_diff: " << std::endl <<
        //        scaled_diff_diff << std::endl << std::endl; std::cout <<
        //        "scaled_diff: " << std::endl << scaled_diff << std::endl <<
        //        std::endl; std::cout <<
        //        "#############################################################################################################################"<<std::endl;
      }
    }
    if (!is_constant_all<T_inv_scale>::value) {
      ops_partials.edge4_.partials_
          = exp_term
            * (INV_SQRT_TWO_PI * sigma_val * exp_term_2
               - (v * sigma_val - diff) * erf_calc)
            / cdf_n;
    }
  }

  //  operands_and_partials<T_y, T_loc, T_scale, T_inv_scale> ops_partials(
  //      y, mu, sigma, lambda);

  //  scalar_seq_view<T_y> y_vec(y);
  //  scalar_seq_view<T_loc> mu_vec(mu);
  //  scalar_seq_view<T_scale> sigma_vec(sigma);
  //  scalar_seq_view<T_inv_scale> lambda_vec(lambda);
  //  size_t N = max_size(y, mu, sigma, lambda);

  //  for (size_t n = 0, size_y = stan::math::size(y); n < size_y; n++) {
  //    if (is_inf(y_vec[n])) {
  //      return ops_partials.build(y_vec[n] < 0 ? negative_infinity() : 0);
  //    }
  //  }

  //  for (size_t n = 0; n < N; n++) {
  //    const T_partials_return y_dbl = value_of(y_vec[n]);
  //    const T_partials_return mu_dbl = value_of(mu_vec[n]);
  //    const T_partials_return sigma_dbl = value_of(sigma_vec[n]);
  //    const T_partials_return lambda_dbl = value_of(lambda_vec[n]);
  //    const T_partials_return inv_sigma = inv(sigma_dbl);
  //    const T_partials_return diff = y_dbl - mu_dbl;
  //    const T_partials_return u = lambda_dbl * diff;
  //    const T_partials_return v = lambda_dbl * sigma_dbl;
  //    const T_partials_return scaled_diff = diff * INV_SQRT_TWO * inv_sigma;
  //    const T_partials_return scaled_diff_diff = scaled_diff - v *
  //    INV_SQRT_TWO; const T_partials_return erf_calc = 0.5 * (1 +
  //    erf(scaled_diff_diff)); const T_partials_return exp_term = exp(0.5 *
  //    square(v) - u); const T_partials_return exp_term_2 =
  //    exp(-square(scaled_diff_diff));

  //    const T_partials_return deriv_1 = lambda_dbl * exp_term * erf_calc;
  //    const T_partials_return deriv_2
  //        = INV_SQRT_TWO_PI * exp_term * exp_term_2 * inv_sigma;
  //    const T_partials_return deriv_3
  //        = INV_SQRT_TWO_PI * exp(-square(scaled_diff)) * inv_sigma;

  //    const T_partials_return cdf_n
  //        = 0.5 + 0.5 * erf(scaled_diff) - exp_term * erf_calc;

  //    cdf_log += log(cdf_n);

  //    if (!is_constant_all<T_y>::value) {
  //      ops_partials.edge1_.partials_[n] += (deriv_1 - deriv_2 + deriv_3) /
  //      cdf_n;
  //    }
  //    if (!is_constant_all<T_loc>::value) {
  //      ops_partials.edge2_.partials_[n] -= (deriv_1 - deriv_2 + deriv_3) /
  //      cdf_n;
  //    }
  //    if (!is_constant_all<T_scale>::value) {
  //      ops_partials.edge3_.partials_[n]
  //          -= ((deriv_1 - deriv_2) * v
  //              + (deriv_3 - deriv_2) * scaled_diff * SQRT_TWO)
  //             / cdf_n;
  //    }
  //    if (!is_constant_all<T_inv_scale>::value) {
  //      ops_partials.edge4_.partials_[n]
  //          += exp_term
  //             * (INV_SQRT_TWO_PI * sigma_dbl * exp_term_2
  //                - (v * sigma_dbl - diff) * erf_calc)
  //             / cdf_n;
  //    }
  //  }
  return ops_partials.build(cdf_log);
}

}  // namespace math
}  // namespace stan
#endif
