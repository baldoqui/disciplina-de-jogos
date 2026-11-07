#include <gtest/gtest.h>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "Dungeon2D.hpp"
#include "PathFind.hpp"
#include "TileMap.hpp"

namespace {

constexpr int W = WALL_TILE;
constexpr int E = EMPTY_TILE;

Vector2D centre(int col, int row) {
  return Vector2D((col + 0.5f) * TILE_SIZE, (row + 0.5f) * TILE_SIZE);
}

int colOf(const Vector2D &p) { return static_cast<int>(std::floor(p.x / TILE_SIZE)); }
int rowOf(const Vector2D &p) { return static_cast<int>(std::floor(p.y / TILE_SIZE)); }

// Checks that consecutive way points are 8-neighbours, lie on empty cells and
// never cut a wall corner.
void expectValidPath(const std::vector<Vector2D> &path, const int *grid,
                     int width) {
  for (std::size_t i = 0; i < path.size(); ++i) {
    const int c = colOf(path[i]), r = rowOf(path[i]);
    EXPECT_EQ(grid[r * width + c], EMPTY_TILE) << "way point " << i;
    EXPECT_TRUE(path[i].equals(centre(c, r))) << "way point " << i;
    if (i == 0)
      continue;
    const int pc = colOf(path[i - 1]), pr = rowOf(path[i - 1]);
    const int dx = c - pc, dy = r - pr;
    EXPECT_LE(std::abs(dx), 1);
    EXPECT_LE(std::abs(dy), 1);
    EXPECT_FALSE(dx == 0 && dy == 0);
    if (dx != 0 && dy != 0) {
      EXPECT_EQ(grid[pr * width + c], EMPTY_TILE) << "corner cut at " << i;
      EXPECT_EQ(grid[r * width + pc], EMPTY_TILE) << "corner cut at " << i;
    }
  }
}

} // namespace

TEST(PathFind, SameCellReturnsThatCentre) {
  const int grid[] = {E, E, E, E};
  PathFind pf(grid, 2, 2);
  auto path = pf.findPath(Vector2D(10, 10), Vector2D(50, 60));
  ASSERT_EQ(path.size(), 1u);
  EXPECT_TRUE(path[0].equals(centre(0, 0)));
}

TEST(PathFind, StraightLine) {
  const int grid[] = {E, E, E, E, E};
  PathFind pf(grid, 5, 1);
  auto path = pf.findPath(centre(0, 0), centre(4, 0));
  ASSERT_EQ(path.size(), 5u);
  for (int i = 0; i < 5; ++i)
    EXPECT_TRUE(path[i].equals(centre(i, 0)));
}

TEST(PathFind, UsesDiagonalsInOpenSpace) {
  std::vector<int> grid(5 * 5, E);
  PathFind pf(grid.data(), 5, 5);
  auto path = pf.findPath(centre(0, 0), centre(4, 4));
  ASSERT_EQ(path.size(), 5u);
  for (int i = 0; i < 5; ++i)
    EXPECT_TRUE(path[i].equals(centre(i, i)));
}

TEST(PathFind, DoesNotCutCorners) {
  // Diagonal (0,0)->(1,1) is blocked by the wall at (1,0).
  const int grid[] = {
      E, W,
      E, E,
  };
  PathFind pf(grid, 2, 2);
  auto path = pf.findPath(centre(0, 0), centre(1, 1));
  ASSERT_EQ(path.size(), 3u);
  EXPECT_TRUE(path[1].equals(centre(0, 1)));
  expectValidPath(path, grid, 2);
}

TEST(PathFind, NoDiagonalSqueezeBetweenWalls) {
  const int grid[] = {
      E, W,
      W, E,
  };
  PathFind pf(grid, 2, 2);
  EXPECT_TRUE(pf.findPath(centre(0, 0), centre(1, 1)).empty());
}

TEST(PathFind, GoesAroundWall) {
  const int grid[] = {
      E, E, E, E, E,
      E, W, W, W, E,
      E, E, E, W, E,
  };
  PathFind pf(grid, 5, 3);
  auto path = pf.findPath(centre(2, 2), centre(4, 2));
  ASSERT_FALSE(path.empty());
  EXPECT_TRUE(path.front().equals(centre(2, 2)));
  EXPECT_TRUE(path.back().equals(centre(4, 2)));
  expectValidPath(path, grid, 5);
  // Every diagonal shortcut touches a wall corner, so the only route hugs the
  // border: (2,2) (1,2) (0,2) (0,1) (0,0) (1,0) (2,0) (3,0) (4,0) (4,1) (4,2).
  EXPECT_EQ(path.size(), 11u);
}

TEST(PathFind, UnreachableGoalIsEmpty) {
  const int grid[] = {E, W, E};
  PathFind pf(grid, 3, 1);
  EXPECT_TRUE(pf.findPath(centre(0, 0), centre(2, 0)).empty());
}

TEST(PathFind, InvalidEndpointsAreEmpty) {
  const int grid[] = {E, W, E, E};
  PathFind pf(grid, 4, 1);
  EXPECT_TRUE(pf.findPath(centre(1, 0), centre(2, 0)).empty());   // start on wall
  EXPECT_TRUE(pf.findPath(centre(2, 0), centre(1, 0)).empty());   // goal on wall
  EXPECT_TRUE(pf.findPath(Vector2D(-1, 10), centre(2, 0)).empty()); // out of range
  EXPECT_TRUE(pf.findPath(centre(2, 0), centre(4, 0)).empty());
  EXPECT_TRUE(pf.findPath(centre(2, 0), centre(0, 1)).empty());
}

TEST(PathFind, ConnectsAllRoomsOfGeneratedDungeon) {
  Dungeon2D dungeon(60, 40);
  dungeon.generate(42);
  TileMap map;
  map.loadFromDungeon(dungeon);
  PathFind pf(map.data(), map.width(), map.height());

  std::vector<Vector2D> centres;
  std::vector<const BSPNode *> stack{dungeon.root.get()};
  while (!stack.empty()) {
    const BSPNode *n = stack.back();
    stack.pop_back();
    if (n->isLeaf()) {
      centres.push_back(centre((n->room.left + n->room.right) / 2,
                               (n->room.top + n->room.bottom) / 2));
    } else {
      stack.push_back(n->left.get());
      stack.push_back(n->right.get());
    }
  }
  ASSERT_GE(centres.size(), 2u);

  for (std::size_t i = 1; i < centres.size(); ++i) {
    auto path = pf.findPath(centres[0], centres[i]);
    ASSERT_FALSE(path.empty()) << "room " << i << " unreachable";
    EXPECT_TRUE(path.front().equals(centres[0]));
    EXPECT_TRUE(path.back().equals(centres[i]));
    expectValidPath(path, map.data(), map.width());
  }
}
