#include <gtest/gtest.h>
#include <stdexcept>

#include "Vector2D.hpp"

// --- Construction ----------------------------------------------------------

TEST(Vector2D, DefaultIsZero) {
  const Vector2D v;
  EXPECT_FLOAT_EQ(v.x, 0.0f);
  EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST(Vector2D, ConstexprConstruction) {
  constexpr Vector2D v(3.0f, 4.0f);
  static_assert(v.length_squared() == 25.0f);
  static_assert(v.dot(Vector2D(1.0f, 0.0f)) == 3.0f);
  static_assert(Vector2D(1.0f, 0.0f).cross(Vector2D(0.0f, 1.0f)) == 1.0f);
}

// --- Length ----------------------------------------------------------------

TEST(Vector2D, Length) {
  const Vector2D v(3.0f, 4.0f);
  EXPECT_FLOAT_EQ(v.length_squared(), 25.0f);
  EXPECT_FLOAT_EQ(v.length(), 5.0f);
}

TEST(Vector2D, NormalizedHasUnitLengthAndSameDirection) {
  const Vector2D v(3.0f, 4.0f);
  const Vector2D n = v.normalized();
  EXPECT_NEAR(n.length(), 1.0f, EPSILON);
  EXPECT_FLOAT_EQ(n.x, 0.6f);
  EXPECT_FLOAT_EQ(n.y, 0.8f);
  // normalized() must not modify the original.
  EXPECT_FLOAT_EQ(v.x, 3.0f);
  EXPECT_FLOAT_EQ(v.y, 4.0f);
}

TEST(Vector2D, NormalizeInPlace) {
  Vector2D v(0.0f, -10.0f);
  v.normalize();
  EXPECT_FLOAT_EQ(v.x, 0.0f);
  EXPECT_FLOAT_EQ(v.y, -1.0f);
}

TEST(Vector2D, NormalizeZeroThrows) {
  Vector2D zero;
  EXPECT_THROW((void)zero.normalized(), std::invalid_argument);
  EXPECT_THROW(zero.normalize(), std::invalid_argument);
}

// --- Dot / cross -----------------------------------------------------------

TEST(Vector2D, Dot) {
  EXPECT_FLOAT_EQ(Vector2D(1, 2).dot(Vector2D(3, 4)), 11.0f);
  EXPECT_FLOAT_EQ(Vector2D(1, 0).dot(Vector2D(0, 1)), 0.0f);
}

TEST(Vector2D, CrossSignGivesOrientation) {
  // b is counter-clockwise from a -> positive.
  EXPECT_GT(Vector2D(1, 0).cross(Vector2D(0, 1)), 0.0f);
  EXPECT_LT(Vector2D(0, 1).cross(Vector2D(1, 0)), 0.0f);
  EXPECT_FLOAT_EQ(Vector2D(2, 4).cross(Vector2D(1, 2)), 0.0f); // parallel
}

// --- equals ----------------------------------------------------------------

TEST(Vector2D, EqualsSameVector) {
  EXPECT_TRUE(Vector2D(1, 2).equals(Vector2D(1, 2)));
  EXPECT_TRUE(Vector2D(1, 2).equals(Vector2D(1.000001f, 2.0f)));
}

TEST(Vector2D, EqualsIsFalseWhenRhsIsLarger) {
  // lhs - rhs is negative here; must still be "not equal".
  EXPECT_FALSE(Vector2D(0, 0).equals(Vector2D(100, 100)));
  EXPECT_FALSE(Vector2D(0, 0).equals(Vector2D(0, 1)));
}

TEST(Vector2D, EqualsIsSymmetric) {
  const Vector2D a(0, 0), b(5, -5);
  EXPECT_EQ(a.equals(b), b.equals(a));
}

TEST(Vector2D, EqualsCustomTolerance) {
  EXPECT_TRUE(Vector2D(1, 1).equals(Vector2D(1.05f, 0.95f), 0.1f));
  EXPECT_FALSE(Vector2D(1, 1).equals(Vector2D(1.2f, 1.0f), 0.1f));
}

// --- Arithmetic operators --------------------------------------------------

TEST(Vector2D, AddSub) {
  const Vector2D a(1, 2), b(3, 5);
  const Vector2D s = a + b;
  const Vector2D d = b - a;
  EXPECT_FLOAT_EQ(s.x, 4.0f);
  EXPECT_FLOAT_EQ(s.y, 7.0f);
  EXPECT_FLOAT_EQ(d.x, 2.0f);
  EXPECT_FLOAT_EQ(d.y, 3.0f);
}

TEST(Vector2D, ScalarMultiplyBothSides) {
  const Vector2D v(1, -2);
  const Vector2D r = v * 3.0f;
  const Vector2D l = 3.0f * v;
  EXPECT_FLOAT_EQ(r.x, 3.0f);
  EXPECT_FLOAT_EQ(r.y, -6.0f);
  EXPECT_FLOAT_EQ(l.x, r.x);
  EXPECT_FLOAT_EQ(l.y, r.y);
}

TEST(Vector2D, Divide) {
  const Vector2D v = Vector2D(4, -8) / 2.0f;
  EXPECT_FLOAT_EQ(v.x, 2.0f);
  EXPECT_FLOAT_EQ(v.y, -4.0f);
}

TEST(Vector2D, DivideByZeroThrows) {
  EXPECT_THROW((void)(Vector2D(1, 1) / 0.0f), std::invalid_argument);
}

TEST(Vector2D, CompoundAddSubMul) {
  Vector2D v(1, 1);
  v += Vector2D(2, 3);
  EXPECT_FLOAT_EQ(v.x, 3.0f);
  EXPECT_FLOAT_EQ(v.y, 4.0f);
  v -= Vector2D(1, 1);
  EXPECT_FLOAT_EQ(v.x, 2.0f);
  EXPECT_FLOAT_EQ(v.y, 3.0f);
  v *= 2.0f;
  EXPECT_FLOAT_EQ(v.x, 4.0f);
  EXPECT_FLOAT_EQ(v.y, 6.0f);
}

TEST(Vector2D, CompoundDivide) {
  Vector2D v(4, 6);
  v /= 2.0f;
  EXPECT_FLOAT_EQ(v.x, 2.0f);
  EXPECT_FLOAT_EQ(v.y, 3.0f);
}

TEST(Vector2D, CompoundDivideByZeroThrows) {
  Vector2D v(1, 1);
  EXPECT_THROW(v /= 0.0f, std::invalid_argument);
}

TEST(Vector2D, CompoundOperatorsReturnSelf) {
  Vector2D v(1, 1);
  EXPECT_EQ(&(v += Vector2D(1, 1)), &v);
  EXPECT_EQ(&(v -= Vector2D(1, 1)), &v);
  EXPECT_EQ(&(v *= 2.0f), &v);
  EXPECT_EQ(&(v /= 2.0f), &v);
}
