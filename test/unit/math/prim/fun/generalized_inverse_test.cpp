#include <stan/math/prim.hpp>
#include <test/unit/util.hpp>
#include <gtest/gtest.h>

TEST(MathMatrixPrim, Zero) {
  stan::math::matrix_d m0(0, 0);
  stan::math::matrix_d ginv = stan::math::generalized_inverse(m0);
  EXPECT_EQ(0, ginv.rows());
  EXPECT_EQ(0, ginv.cols());
}

TEST(MathMatrixPrim, Equal1) {
  using stan::math::generalized_inverse;

  stan::math::matrix_d m1(2, 3);
  m1 << 1, 3, 5, 2, 4, 6;

  stan::math::matrix_d m2(2, 2);
  m2 << 1, 0, 0, 1;

  stan::math::matrix_d m3 = m1 * generalized_inverse(m1);
  EXPECT_MATRIX_NEAR(m2, m3, 1e-9);
}

TEST(MathMatrixPrim, Equal2) {
  using stan::math::generalized_inverse;

  stan::math::matrix_d m1(3, 2);
  m1 << 1, 2, 2, 4, 1, 2;

  stan::math::matrix_d m2(3, 3);
  m2 << 1.0 / 6.0, 1.0 / 3.0, 1.0 / 6.0, 
        1.0 / 3.0, 2.0 / 3.0, 1.0 / 3.0,
        1.0 / 6.0, 1.0 / 3.0, 1.0 / 6.0;

  stan::math::matrix_d m3 = m1 * generalized_inverse(m1);
  EXPECT_MATRIX_NEAR(m2, m3, 1e-9);
} 