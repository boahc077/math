#ifndef TEST_UNIT_MATH_REV_FUNCTOR_ODE_FIXTURE_HPP
#define TEST_UNIT_MATH_REV_FUNCTOR_ODE_FIXTURE_HPP

#include <stan/math/rev.hpp>
#include <test/prob/utility.hpp>
#include <gtest/gtest.h>
#include <test/unit/util.hpp>
#include <type_traits>
#include <iostream>
#include <sstream>
#include <vector>
#include <limits>
#include <string>


inline void print_stack(std::ostream& o) {
  o << "STACK, size=" << stan::math::ChainableStack::instance_->var_stack_.size()
    << std::endl;
  // TODO(carpenter): this shouldn't need to be cast any more
  for (size_t i = 0; i < stan::math::ChainableStack::instance_->var_stack_.size(); ++i)
    o << i << "  " << stan::math::ChainableStack::instance_->var_stack_[i] << "  "
      << (static_cast<stan::math::vari*>(stan::math::ChainableStack::instance_->var_stack_[i]))->val_
      << " : "
      << (static_cast<stan::math::vari*>(stan::math::ChainableStack::instance_->var_stack_[i]))->adj_
      << std::endl;
  o << "STACK NOCHAIN, size=" << stan::math::ChainableStack::instance_->var_nochain_stack_.size()
    << std::endl;
  // TODO(carpenter): this shouldn't need to be cast any more
  for (size_t i = 0; i < stan::math::ChainableStack::instance_->var_nochain_stack_.size(); ++i)
    o << i << "  " << stan::math::ChainableStack::instance_->var_nochain_stack_[i] << "  "
      << (static_cast<stan::math::vari*>(stan::math::ChainableStack::instance_->var_nochain_stack_[i]))->val_
      << " : "
      << (static_cast<stan::math::vari*>(stan::math::ChainableStack::instance_->var_nochain_stack_[i]))->adj_
      << std::endl;
}


template <class ode_problem_type>
struct ODETestFixture : public ::testing::Test {
  /**
   * test ODE solver pass
   */
  void test_good() {
    ode_problem_type& ode = static_cast<ode_problem_type&>(*this);
    ASSERT_NO_THROW(ode.apply_solver());
  }

  /**
   * test ODE solution against analytical solution
   *
   * @param ode_sol solver functor that takes a <code>vector</code>
   * parameter variable that returns solution <code>vector</code>.
   * @param analy_sol analytical solution functor that returns
   * solution <code>vector</code>.
   * @param tol comparison tolerance
   * @param x parameter <code>vector</code> fed into <code>ode_sol</code>
   * @param t time at which analyitical solution is evaluated
   * @param args parameters pack required by <code>analy_sol</code>.
   */
  template <typename F_ode, typename F_sol, typename... T_args>
  void test_analytical(F_ode const& ode_sol, F_sol const& analy_sol, double tol,
                       Eigen::VectorXd const& x, double t,
                       const T_args&... args) {
    ode_problem_type& ode = static_cast<ode_problem_type&>(*this);
    auto sol = ode_sol(x);
    int n = sol.size();
    auto sol_0 = analy_sol(t, args...);
    for (int i = 0; i < n; ++i) {
      EXPECT_NEAR(stan::math::value_of(sol[i]), sol_0[i], tol);
    }
  }

  /**
   * test ODE solution as well as sensitivity solution
   * against analytical solution.
   *
   *
   * @param ode_sol solver functor that takes a <code>vector</code>
   * parameter variable that returns solution <code>vector</code>.
   * @param analy_sol analytical solution functor that returns
   * solution <code>vector</code>.
   * @param analy_grad_sol analytical sensitivity solution functor
   * that returns sensitivity <code>matrix</code>, with column i
   * corresponding to sensitivity of state i w.r.t parameters
   * <code>vector</code> x.
   * @param tol comparison tolerance
   * @param x parameter <code>vector</code> fed into <code>ode_sol</code>
   * @param t time at which analyitical solution is evaluated
   * @param args parameters pack required by <code>analy_sol</code>.
   */
  template <typename F_ode, typename F_sol, typename F_grad_sol,
            typename... T_args>
  void test_analytical(F_ode const& ode_sol, F_sol const& analy_sol,
                       F_grad_sol const& analy_grad_sol, double tol,
                       Eigen::VectorXd const& x, double t,
                       const T_args&... args) {
    ode_problem_type& ode = static_cast<ode_problem_type&>(*this);

    auto sol_0 = analy_sol(t, args...);
    Eigen::Matrix<var, -1, 1> x_var(stan::math::to_var(x));
    Eigen::Matrix<stan::math::var, -1, 1> sol = ode_sol(x_var);
    EXPECT_TRUE(sol.size() == sol_0.size());
    for (auto i = 0; i < sol.size(); ++i) {
      EXPECT_NEAR(sol[i].val(), sol_0[i], tol)
          << "ODE solution failed for state i, i = " << i;
    }

    Eigen::Matrix<double, -1, -1> grad_0(analy_grad_sol(t, args...));
    for (auto i = 0; i < sol.size(); ++i) {
      stan::math::set_zero_all_adjoints();
      sol[i].grad();
      for (auto j = 0; j < x.size(); ++j) {
        EXPECT_NEAR(x_var[j].adj(), grad_0(j, i), tol)
            << "ODE sensitivity solution failed for state i and parameter j, "
               "(i, j) = ("
            << i << ", " << j << ").";
      }
    }
  }

  /**
   * Gradient wrt to certain param using central difference
   *
   * @param param_index index to param of which sensitivity is seeked
   * @param h finite difference step/perturbation.
   *
   * @return gradient
   */
  std::vector<Eigen::VectorXd> fd_param(const size_t& param_index,
                                        const double& h) {
    std::stringstream msgs;
    ode_problem_type& ode = static_cast<ode_problem_type&>(*this);

    auto param = ode.param();
    auto init = ode.init();
    param[param_index] += h;
    std::vector<Eigen::VectorXd> res_ub = ode.apply_solver(init, param);
    param[param_index] -= 2 * h;
    std::vector<Eigen::VectorXd> res_lb = ode.apply_solver(init, param);

    std::vector<Eigen::VectorXd> results(ode.ts.size());

    for (size_t i = 0; i < ode.ts.size(); ++i) {
      results[i] = (res_ub[i] - res_lb[i]) / (2.0 * h);
    }
    return results;
  }

  /**
   * Gradient wrt to certain param using central difference
   *
   * @param param_index index to param of which sensitivity is seeked
   * @param h finite difference step/perturbation.
   *
   * @return gradient
   */
  std::vector<Eigen::VectorXd> fd_init(const size_t& param_index,
                                       const double& h) {
    std::stringstream msgs;
    ode_problem_type& ode = static_cast<ode_problem_type&>(*this);

    auto param = ode.param();
    auto init = ode.init();
    init[param_index] += h;
    std::vector<Eigen::VectorXd> res_ub = ode.apply_solver(init, param);
    init[param_index] -= 2 * h;
    std::vector<Eigen::VectorXd> res_lb = ode.apply_solver(init, param);

    std::vector<Eigen::VectorXd> results(ode.ts.size());

    for (size_t i = 0; i < ode.ts.size(); ++i) {
      results[i] = (res_ub[i] - res_lb[i]) / (2.0 * h);
    }
    return results;
  }

  /**
   * Test AD against finite diff when param
   * is <code>var</code>.
   *
   * Require <code>apply_solver(T1&& init, T2&& param)</code> from child fixture
   * for finite diff grad calculation. The call should return ODE data results
   * with when <code>T1</code> and <code>T2</code> are data.
   *
   * @param diff finite diff stepsize
   * @param tol double value test tolerance
   */
  void test_fd_dv(double diff, double tol) {
    std::stringstream msgs;
    ode_problem_type& ode = static_cast<ode_problem_type&>(*this);

    int n = ode.param().size();
    std::cout << "finite diff..." << std::endl;
    std::vector<std::vector<Eigen::VectorXd>> fd_res(n);
    for (size_t i = 0; i < n; ++i) {
      fd_res[i] = fd_param(i, diff);
    }
    std::cout << "finite diff...done" << std::endl;
    std::vector<double> grads_eff;

    stan::math::nested_rev_autodiff nested;
    auto theta_v = stan::math::to_var(ode.param());

    std::cout << "AD..." << std::endl;
    std::vector<Eigen::Matrix<stan::math::var, -1, 1>> ode_res
        = ode.apply_solver(ode.init(), theta_v);

    for (size_t i = 0; i < ode.ts.size(); i++) {
      for (size_t j = 0; j < ode_res[0].size(); j++) {
        std::cout << "i = " << i << ": j = " << j << std::endl;
        grads_eff.clear();
        //ode_res[i][j].grad(theta_v, grads_eff);
        //ode_res[i][j].grad();
        //print_stack(std::cout);
        stan::math::grad(ode_res[i][j].vi_);
        print_stack(std::cout);
        std::cout << "i = " << i << ": j = " << j << " grad done." << std::endl;

        /*
        for (size_t k = 0; k < n; k++)
          EXPECT_NEAR(grads_eff[k], fd_res[k][i][j], tol)
              << "Gradient of ODE solver failed with initial positions"
              << " known and parameters unknown at time index " << i
              << ", equation index " << j << ", and parameter index: "
          << k;
        */
        for (size_t k = 0; k < n; k++)
          EXPECT_NEAR(theta_v[k].adj(), fd_res[k][i][j], tol)
              << "Gradient of ODE solver failed with initial positions"
              << " known and parameters unknown at time index " << i
              << ", equation index " << j << ", and parameter index: " << k;

        //stan::math::set_zero_all_adjoints();
        nested.set_zero_all_adjoints();
      }
    }
    std::cout << "AD...done" << std::endl;
  }

  /**
   * Test AD against finite diff when initial condition
   * is <code>var</code>.
   *
   * Require <code>apply_solver(T1&& init, T2&& param)</code> from child fixture
   * for finite diff grad calculation. The call should return ODE data results
   * with when <code>T1</code> and <code>T2</code> are data.
   *
   * @param diff finite diff stepsize
   * @param tol double value test tolerance
   */
  void test_fd_vd(double diff, double tol) {
    std::stringstream msgs;
    ode_problem_type& ode = static_cast<ode_problem_type&>(*this);

    Eigen::Matrix<stan::math::var, -1, 1> y0_v = stan::math::to_var(ode.init());
    int n = y0_v.size();
    std::vector<std::vector<Eigen::VectorXd>> fd_res(n);
    for (size_t i = 0; i < n; ++i) {
      fd_res[i] = fd_init(i, diff);
    }
    std::vector<double> grads_eff;

    std::vector<Eigen::Matrix<stan::math::var, -1, 1>> ode_res
        = ode.apply_solver(y0_v, ode.param());

    std::vector<stan::math::var> y_vec(to_array_1d(y0_v));

    for (size_t i = 0; i < ode.ts.size(); i++) {
      for (size_t j = 0; j < n; j++) {
        grads_eff.clear();
        ode_res[i][j].grad(y_vec, grads_eff);

        for (size_t k = 0; k < n; k++)
          EXPECT_NEAR(grads_eff[k], fd_res[k][i][j], tol)
              << "Gradient of ode solver failed with initial positions"
              << " unknown and parameters known at time index " << i
              << ", equation index " << j << ", and parameter index: " << k;

        stan::math::set_zero_all_adjoints();
      }
    }
  }

  /**
   * Test AD against finite diff when both initial condition & param
   * are <code>var</code>.
   *
   * Require <code>apply_solver(T1&& init, T2&& param)</code> from child fixture
   * for finite diff grad calculation. The call should return ODE data results
   * with when <code>T1</code> and <code>T2</code> are data.
   *
   * @param diff finite diff stepsize
   * @param tol double value test tolerance
   */
  void test_fd_vv(double diff, double tol) {
    std::stringstream msgs;
    ode_problem_type& ode = static_cast<ode_problem_type&>(*this);

    int n = ode.dim();
    int m = ode.param_size();
    std::vector<std::vector<Eigen::VectorXd>> fd_res_y(n);
    for (size_t i = 0; i < n; ++i) {
      fd_res_y[i] = fd_init(i, diff);
    }

    std::vector<std::vector<Eigen::VectorXd>> fd_res_p(m);
    for (size_t i = 0; i < m; ++i) {
      fd_res_p[i] = fd_param(i, diff);
    }

    std::vector<stan::math::var> vars(
        stan::math::to_array_1d(stan::math::to_var(ode.init())));
    auto theta = ode.param();
    for (int i = 0; i < m; ++i) {
      vars.push_back(theta[i]);
    }
    Eigen::Matrix<stan::math::var, -1, 1> yv(n);
    for (int i = 0; i < n; ++i) {
      yv(i) = vars[i];
    }
    std::vector<stan::math::var> theta_v(vars.begin() + n, vars.end());

    std::vector<Eigen::Matrix<stan::math::var, -1, 1>> ode_res
        = ode.apply_solver(yv, theta_v);

    std::vector<double> grads_eff;
    for (size_t i = 0; i < ode.ts.size(); i++) {
      for (size_t j = 0; j < n; j++) {
        grads_eff.clear();
        ode_res[i][j].grad(vars, grads_eff);

        for (size_t k = 0; k < m; k++) {
          EXPECT_NEAR(grads_eff[k + n], fd_res_p[k][i][j], tol)
              << "Gradient of ode solver failed with initial positions"
              << " unknown and parameters unknown for param at time index " << i
              << ", equation index " << j << ", and parameter index: " << k;
        }
        for (size_t k = 0; k < n; k++) {
          EXPECT_NEAR(grads_eff[k], fd_res_y[k][i][j], tol)
              << "Gradient of ode solver failed with initial positions"
              << " unknown and parameters known for initial position at time "
                 "index "
              << i << ", equation index " << j
              << ", and parameter index: " << k;
        }

        stan::math::set_zero_all_adjoints();
      }
    }
  }

  /**
   * Test AD when <code>ts</code> is <code>var</code>.
   *
   * require <code>apply_solver()</code> from child fixture for ODE
   * solution when time step is <code>var</code>, and <code>eval_rhs</code>
   * to calculate RHS.
   */
  void test_ts_ad() {
    ode_problem_type& ode = static_cast<ode_problem_type&>(*this);
    std::vector<double> g;
    std::vector<Eigen::Matrix<stan::math::var, -1, 1>> res = ode.apply_solver();
    size_t nt = res.size();
    for (auto i = 0; i < nt; ++i) {
      Eigen::VectorXd res_d = stan::math::value_of(res[i]);
      for (auto j = 0; j < ode.dim(); ++j) {
        g.clear();
        res[i][j].grad();
        for (auto k = 0; k < nt; ++k) {
          if (k != i) {
            EXPECT_FLOAT_EQ(ode.ts[k].adj(), 0.0);
          } else {
            double ts_ad
                = stan::math::value_of(ode.eval_rhs(ode.ts[i].val(), res_d)[j]);
            EXPECT_FLOAT_EQ(ode.ts[k].adj(), ts_ad);
          }
        }
        stan::math::set_zero_all_adjoints();
      }
    }
  }
};

#endif
