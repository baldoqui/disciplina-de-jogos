#include "Transform2D.hpp"
#include <cmath>

Transform2D Transform2D::translation(float tx, float ty) noexcept {
  Transform2D res;
  res.m[2][0] = tx;
  res.m[2][1] = ty;
  return res;
}

Transform2D Transform2D::rotation(float angle_rad) noexcept {
  Transform2D res;
  const float c = std::cos(angle_rad);
  const float s = std::sin(angle_rad);

  res.m[0][0] = c;
  res.m[0][1] = s;
  res.m[1][0] = -s;
  res.m[1][1] = c;

  return res;
}

Transform2D Transform2D::scale(float sx, float sy) noexcept {
  Transform2D res;
  res.m[0][0] = sx;
  res.m[1][1] = sy;
  return res;
}

Transform2D Transform2D::operator*(const Transform2D &rhs) const noexcept {
  Transform2D res;

  for (int r = 0; r < 3; ++r) {
    for (int c = 0; c < 3; ++c) {
      res.m[r][c] =
          m[r][0] * rhs.m[0][c] + m[r][1] * rhs.m[1][c] + m[r][2] * rhs.m[2][c];
    }
  }

  return res;
}

Transform2D &Transform2D::operator*=(const Transform2D &rhs) noexcept {
  *this = *this * rhs;
  return *this;
}

Vector2D Transform2D::transform_point(const Vector2D &point) const noexcept {
  // [point.x, point.y, 1.0f]
  // Result = p * M
  // res.x = point.x * m[0][0] + point.y * m[1][0] + 1.0f * m[2][0]
  // res.y = point.x * m[0][1] + point.y * m[1][1] + 1.0f * m[2][1]

  return Vector2D(point.x * m[0][0] + point.y * m[1][0] + m[2][0],
                  point.x * m[0][1] + point.y * m[1][1] + m[2][1]);
}

Vector2D
Transform2D::transform_vector(const Vector2D &direction) const noexcept {
  // [direction.x, direction.y, 0.0f]

  return Vector2D(direction.x * m[0][0] + direction.y * m[1][0],
                  direction.x * m[0][1] + direction.y * m[1][1]);
}
