#include <gtest/gtest.h>

#include "Transform2D.hpp"

namespace {

constexpr float PI = 3.14159265358979323846f;
constexpr float TOL = 1e-5f;

void expect_vec_near(const Vector2D &actual, float x, float y) {
  EXPECT_NEAR(actual.x, x, TOL);
  EXPECT_NEAR(actual.y, y, TOL);
}

void expect_affine_column(const Transform2D &t) {
  EXPECT_FLOAT_EQ(t.m[0][2], 0.0f);
  EXPECT_FLOAT_EQ(t.m[1][2], 0.0f);
  EXPECT_FLOAT_EQ(t.m[2][2], 1.0f);
}

} // namespace

TEST(Transform2D, DefaultIsIdentity) {
  const Transform2D t;
  for (int r = 0; r < 3; ++r)
    for (int c = 0; c < 3; ++c)
      EXPECT_FLOAT_EQ(t.m[r][c], r == c ? 1.0f : 0.0f);

  expect_vec_near(t.transform_point(Vector2D(3, -4)), 3, -4);
  expect_vec_near(t.transform_vector(Vector2D(3, -4)), 3, -4);
}

// --- Translation -----------------------------------------------------------

TEST(Transform2D, TranslationLivesInThirdRow) {
  const Transform2D t = Transform2D::translation(5, -2);
  EXPECT_FLOAT_EQ(t.m[2][0], 5.0f);
  EXPECT_FLOAT_EQ(t.m[2][1], -2.0f);
  expect_affine_column(t);
}

TEST(Transform2D, TranslationMovesPointsButNotVectors) {
  const Transform2D t = Transform2D::translation(5, -2);
  expect_vec_near(t.transform_point(Vector2D(1, 1)), 6, -1);
  expect_vec_near(t.transform_vector(Vector2D(1, 1)), 1, 1);
}

// --- Rotation --------------------------------------------------------------

TEST(Transform2D, RotationIsCounterClockwise) {
  const Transform2D r = Transform2D::rotation(PI / 2);
  expect_vec_near(r.transform_point(Vector2D(1, 0)), 0, 1);
  expect_vec_near(r.transform_point(Vector2D(0, 1)), -1, 0);
  expect_affine_column(r);
}

TEST(Transform2D, RotationPreservesLength) {
  const Transform2D r = Transform2D::rotation(0.73f);
  const Vector2D v(3, 4);
  EXPECT_NEAR(r.transform_vector(v).length(), 5.0f, TOL);
}

TEST(Transform2D, RotationZeroIsIdentity) {
  expect_vec_near(Transform2D::rotation(0).transform_point(Vector2D(2, 7)), 2,
                  7);
}

TEST(Transform2D, RotationsAddUp) {
  const Transform2D r = Transform2D::rotation(PI / 6) *
                        Transform2D::rotation(PI / 3);
  expect_vec_near(r.transform_point(Vector2D(1, 0)), 0, 1);
}

// --- Scale -----------------------------------------------------------------

TEST(Transform2D, Scale) {
  const Transform2D s = Transform2D::scale(2, -3);
  expect_vec_near(s.transform_point(Vector2D(1, 1)), 2, -3);
  expect_vec_near(s.transform_vector(Vector2D(1, 1)), 2, -3);
  expect_affine_column(s);
}

// --- Composition -----------------------------------------------------------

TEST(Transform2D, IdentityIsNeutral) {
  const Transform2D t = Transform2D::translation(1, 2) *
                        Transform2D::rotation(0.4f) *
                        Transform2D::scale(2, 3);
  const Transform2D left = Transform2D() * t;
  const Transform2D right = t * Transform2D();
  for (int r = 0; r < 3; ++r)
    for (int c = 0; c < 3; ++c) {
      EXPECT_NEAR(left.m[r][c], t.m[r][c], TOL);
      EXPECT_NEAR(right.m[r][c], t.m[r][c], TOL);
    }
}

TEST(Transform2D, ProductAppliesLeftFirst) {
  // Scale first, then translate: (1,1) -> (2,2) -> (12,2)
  const Transform2D st =
      Transform2D::scale(2, 2) * Transform2D::translation(10, 0);
  expect_vec_near(st.transform_point(Vector2D(1, 1)), 12, 2);

  // Translate first, then scale: (1,1) -> (11,1) -> (22,2)
  const Transform2D ts =
      Transform2D::translation(10, 0) * Transform2D::scale(2, 2);
  expect_vec_near(ts.transform_point(Vector2D(1, 1)), 22, 2);
}

TEST(Transform2D, ProductMatchesSequentialApplication) {
  const Transform2D a = Transform2D::rotation(0.9f);
  const Transform2D b = Transform2D::translation(-3, 4);
  const Transform2D c = Transform2D::scale(0.5f, 2.0f);
  const Vector2D p(1.5f, -2.5f);

  const Vector2D sequential =
      c.transform_point(b.transform_point(a.transform_point(p)));
  const Vector2D composed = (a * b * c).transform_point(p);
  expect_vec_near(composed, sequential.x, sequential.y);
  expect_affine_column(a * b * c);
}

TEST(Transform2D, ProductIsAssociative) {
  const Transform2D a = Transform2D::rotation(0.3f);
  const Transform2D b = Transform2D::translation(2, 1);
  const Transform2D c = Transform2D::scale(3, 0.5f);
  const Transform2D l = (a * b) * c;
  const Transform2D r = a * (b * c);
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      EXPECT_NEAR(l.m[i][j], r.m[i][j], TOL);
}

TEST(Transform2D, CompoundMultiply) {
  Transform2D t = Transform2D::scale(2, 2);
  Transform2D &ref = (t *= Transform2D::translation(10, 0));
  EXPECT_EQ(&ref, &t);
  expect_vec_near(t.transform_point(Vector2D(1, 1)), 12, 2);
}

TEST(Transform2D, VectorIgnoresTranslationInComposite) {
  const Transform2D t =
      Transform2D::rotation(PI / 2) * Transform2D::translation(100, 100);
  expect_vec_near(t.transform_vector(Vector2D(1, 0)), 0, 1);
}
