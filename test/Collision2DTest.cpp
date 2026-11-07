#include <gtest/gtest.h>

#include "Collision2D.hpp"

namespace {

AABB box(float x0, float y0, float x1, float y1) {
  return AABB{Vector2D(x0, y0), Vector2D(x1, y1)};
}

} // namespace

// --- AABB::intersects ------------------------------------------------------

TEST(AABB, OverlappingBoxesIntersect) {
  EXPECT_TRUE(box(0, 0, 10, 10).intersects(box(5, 5, 15, 15)));
}

TEST(AABB, ContainedBoxIntersects) {
  EXPECT_TRUE(box(0, 0, 10, 10).intersects(box(2, 2, 3, 3)));
  EXPECT_TRUE(box(2, 2, 3, 3).intersects(box(0, 0, 10, 10)));
}

TEST(AABB, TouchingEdgesIntersect) {
  EXPECT_TRUE(box(0, 0, 10, 10).intersects(box(10, 0, 20, 10)));
  EXPECT_TRUE(box(0, 0, 10, 10).intersects(box(0, 10, 10, 20)));
  EXPECT_TRUE(box(0, 0, 10, 10).intersects(box(10, 10, 20, 20))); // corner
}

TEST(AABB, SeparatedBoxesDoNotIntersect) {
  EXPECT_FALSE(box(0, 0, 10, 10).intersects(box(11, 0, 20, 10))); // right
  EXPECT_FALSE(box(0, 0, 10, 10).intersects(box(-20, 0, -1, 10))); // left
  EXPECT_FALSE(box(0, 0, 10, 10).intersects(box(0, 11, 10, 20))); // below
  EXPECT_FALSE(box(0, 0, 10, 10).intersects(box(0, -20, 10, -1))); // above
}

TEST(AABB, OverlapOnOneAxisOnlyIsNotEnough) {
  EXPECT_FALSE(box(0, 0, 10, 10).intersects(box(5, 20, 15, 30)));
  EXPECT_FALSE(box(0, 0, 10, 10).intersects(box(20, 5, 30, 15)));
}

TEST(AABB, IntersectsIsSymmetric) {
  const AABB a = box(0, 0, 10, 10);
  const AABB b = box(9, -5, 12, 1);
  const AABB c = box(30, 30, 40, 40);
  EXPECT_EQ(a.intersects(b), b.intersects(a));
  EXPECT_EQ(a.intersects(c), c.intersects(a));
}

// --- Collision2D -----------------------------------------------------------

TEST(Collision2D, DefaultIsTileSizedSquare) {
  const Collision2D c;
  EXPECT_FLOAT_EQ(c.halfExtents.x, TILE_SIZE * 0.5f);
  EXPECT_FLOAT_EQ(c.halfExtents.y, TILE_SIZE * 0.5f);

  const AABB b = c.bounds(Vector2D(0, 0));
  EXPECT_FLOAT_EQ(b.max.x - b.min.x, TILE_SIZE);
  EXPECT_FLOAT_EQ(b.max.y - b.min.y, TILE_SIZE);
}

TEST(Collision2D, BoundsAreCentredOnPosition) {
  Collision2D c;
  c.halfExtents = Vector2D(10, 4);
  const AABB b = c.bounds(Vector2D(100, 50));
  EXPECT_FLOAT_EQ(b.min.x, 90.0f);
  EXPECT_FLOAT_EQ(b.min.y, 46.0f);
  EXPECT_FLOAT_EQ(b.max.x, 110.0f);
  EXPECT_FLOAT_EQ(b.max.y, 54.0f);
}

TEST(Collision2D, AdjacentTilesTouch) {
  const Collision2D c;
  const AABB a = c.bounds(Vector2D(32, 32));
  const AABB b = c.bounds(Vector2D(32 + TILE_SIZE, 32));
  const AABB far = c.bounds(Vector2D(32 + TILE_SIZE + 1, 32));
  EXPECT_TRUE(a.intersects(b));
  EXPECT_FALSE(a.intersects(far));
}
