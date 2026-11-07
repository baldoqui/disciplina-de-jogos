#include "Collision2D.hpp"

bool AABB::intersects(const AABB &other) const noexcept {
  return min.x <= other.max.x && max.x >= other.min.x &&
         min.y <= other.max.y && max.y >= other.min.y;
}

AABB Collision2D::bounds(const Vector2D &position) const noexcept {
  return AABB{position - halfExtents, position + halfExtents};
}
