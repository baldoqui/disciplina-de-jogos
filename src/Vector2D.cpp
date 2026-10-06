#include <stdexcept>

#include "Vector2D.hpp"

Vector2D Vector2D::normalized() const {
  const float len = length();

  if (len <= EPSILON)
    throw std::invalid_argument(
        "Cannot normalize a zero or near-zero length vector.");

  return Vector2D(x / len, y / len);
}

void Vector2D::normalize() {
  const Vector2D norm_vec = normalized();

  x = norm_vec.x;
  y = norm_vec.y;
}

bool Vector2D::equals(const Vector2D &rhs, float tolerance) const noexcept {
  const Vector2D dist_vec = *this - rhs;

  if (std::abs(dist_vec.x) < tolerance && std::abs(dist_vec.y) < tolerance)
    return true;
  else
    return false;
}

Vector2D Vector2D::operator+(const Vector2D &rhs) const noexcept {
  return Vector2D(x + rhs.x, y + rhs.y);
}

Vector2D Vector2D::operator-(const Vector2D &rhs) const noexcept {
  return Vector2D(x - rhs.x, y - rhs.y);
}

Vector2D Vector2D::operator*(float scalar) const noexcept {
  return Vector2D(x * scalar, y * scalar);
}

Vector2D Vector2D::operator/(float scalar) const {
  if (std::abs(scalar) <= EPSILON)
    throw std::invalid_argument(
        "Vector2D division by zero or near-zero scalar.");

  return Vector2D(x / scalar, y / scalar);
}

Vector2D &Vector2D::operator+=(const Vector2D &rhs) noexcept {
  *this = *this + rhs;
  return *this;
}

Vector2D &Vector2D::operator-=(const Vector2D &rhs) noexcept {
  *this = *this - rhs;
  return *this;
}

Vector2D &Vector2D::operator*=(float scalar) noexcept {
  *this = *this * scalar;
  return *this;
}

Vector2D &Vector2D::operator/=(float scalar) {
  *this = *this / scalar;
  return *this;
}

Vector2D operator*(float scalar, const Vector2D &vec) noexcept {
  return vec * scalar;
}
