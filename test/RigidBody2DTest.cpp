#include <gtest/gtest.h>

#include "RigidBody2D.hpp"

namespace {

constexpr float TOL = 1e-4f;

void expect_vec_near(const Vector2D &actual, float x, float y) {
  EXPECT_NEAR(actual.x, x, TOL);
  EXPECT_NEAR(actual.y, y, TOL);
}

} // namespace

TEST(RigidBody2D, DefaultIsAtRest) {
  RigidBody2D body;
  body.integrate(0.1f);
  expect_vec_near(body.position, 0, 0);
  expect_vec_near(body.velocity, 0, 0);
  expect_vec_near(body.acceleration, 0, 0);
}

TEST(RigidBody2D, ConstantVelocityMovesLinearly) {
  RigidBody2D body;
  body.position = Vector2D(10, 20);
  body.velocity = Vector2D(100, -50);
  body.integrate(0.5f);
  expect_vec_near(body.position, 60, -5);
  expect_vec_near(body.velocity, 100, -50);
}

TEST(RigidBody2D, UsesUpdatedVelocityForPosition) {
  // Semi-implicit Euler: v = 0 + 10*1 = 10, then x = 0 + 10*1 = 10.
  // (Explicit Euler would leave x at 0 after the first step.)
  RigidBody2D body;
  body.acceleration = Vector2D(10, 0);
  body.integrate(1.0f);
  expect_vec_near(body.velocity, 10, 0);
  expect_vec_near(body.position, 10, 0);
}

TEST(RigidBody2D, MultipleSteps) {
  // With constant a and step dt, after n steps:
  //   v_n = n*a*dt,  x_n = a*dt^2 * n(n+1)/2.
  RigidBody2D body;
  body.acceleration = Vector2D(0, 2);
  const float dt = 0.25f;
  const int n = 8;
  for (int i = 0; i < n; ++i)
    body.integrate(dt);
  expect_vec_near(body.velocity, 0, n * 2 * dt);
  expect_vec_near(body.position, 0, 2 * dt * dt * n * (n + 1) / 2.0f);
}

TEST(RigidBody2D, AccelerationIsNotModified) {
  RigidBody2D body;
  body.acceleration = Vector2D(3, -4);
  body.integrate(0.1f);
  expect_vec_near(body.acceleration, 3, -4);
}

TEST(RigidBody2D, IsReproducible) {
  RigidBody2D a, b;
  a.velocity = b.velocity = Vector2D(1.5f, -2.0f);
  a.acceleration = b.acceleration = Vector2D(0.3f, 9.8f);
  for (int i = 0; i < 100; ++i) {
    a.integrate(1.0f / 60.0f);
    b.integrate(1.0f / 60.0f);
  }
  EXPECT_FLOAT_EQ(a.position.x, b.position.x);
  EXPECT_FLOAT_EQ(a.position.y, b.position.y);
}
