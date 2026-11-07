#include "RigidBody2D.hpp"

void RigidBody2D::integrate(float dt) noexcept {
  // Semi-implicit Euler: update velocity first, then move with the new one.
  velocity += acceleration * dt;
  position += velocity * dt;
}
