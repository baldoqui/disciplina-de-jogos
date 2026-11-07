#include <gtest/gtest.h>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "Dungeon2D.hpp"
#include "TileMap.hpp"

// Rect bounds are inclusive: size = right - left + 1.

namespace {

int rectWidth(const Rect &r) { return r.right - r.left + 1; }
int rectHeight(const Rect &r) { return r.bottom - r.top + 1; }

bool contains(const Rect &outer, const Rect &inner) {
  return inner.left >= outer.left && inner.right <= outer.right &&
         inner.top >= outer.top && inner.bottom <= outer.bottom;
}

bool overlaps(const Rect &a, const Rect &b) {
  return a.left <= b.right && b.left <= a.right && a.top <= b.bottom &&
         b.top <= a.bottom;
}

void collectLeaves(const BSPNode &node, std::vector<const BSPNode *> &out) {
  if (node.isLeaf()) {
    out.push_back(&node);
    return;
  }
  collectLeaves(*node.left, out);
  collectLeaves(*node.right, out);
}

// Checks that every internal node is split into two children that exactly
// tile its region.
void checkTree(const BSPNode &node) {
  ASSERT_EQ(static_cast<bool>(node.left), static_cast<bool>(node.right));
  if (node.isLeaf())
    return;
  const Rect &p = node.region;
  const Rect &a = node.left->region;
  const Rect &b = node.right->region;
  EXPECT_TRUE(contains(p, a));
  EXPECT_TRUE(contains(p, b));
  EXPECT_FALSE(overlaps(a, b));
  EXPECT_EQ(rectWidth(a) * rectHeight(a) + rectWidth(b) * rectHeight(b),
            rectWidth(p) * rectHeight(p));
  checkTree(*node.left);
  checkTree(*node.right);
}

// Number of EMPTY cells reachable (4-neighbour) from the first empty cell.
int reachableEmpty(const TileMap &map) {
  const int w = map.width(), h = map.height();
  std::vector<char> seen(static_cast<std::size_t>(w) * h, 0);
  std::vector<int> stack;
  for (int i = 0; i < w * h && stack.empty(); ++i)
    if (map.data()[i] == EMPTY_TILE)
      stack.push_back(i);

  int count = 0;
  while (!stack.empty()) {
    const int i = stack.back();
    stack.pop_back();
    if (seen[i] || map.data()[i] != EMPTY_TILE)
      continue;
    seen[i] = 1;
    ++count;
    const int c = i % w, r = i / w;
    if (c > 0) stack.push_back(i - 1);
    if (c < w - 1) stack.push_back(i + 1);
    if (r > 0) stack.push_back(i - w);
    if (r < h - 1) stack.push_back(i + w);
  }
  return count;
}

struct Size {
  int w, h;
};

const Size kSizes[] = {{5, 5}, {6, 9}, {20, 14}, {40, 30}, {80, 60}, {120, 20}};
const unsigned kSeeds[] = {0u, 1u, 7u, 42u, 1234u, 99999u};

} // namespace

TEST(Dungeon2D, ConstructorStoresSize) {
  const Dungeon2D d(40, 30);
  EXPECT_EQ(d.width, 40);
  EXPECT_EQ(d.height, 30);
  EXPECT_EQ(d.root, nullptr);
  EXPECT_TRUE(d.corridors.empty());
}

TEST(Dungeon2D, ConstructorRejectsTooSmall) {
  EXPECT_THROW(Dungeon2D(4, 10), std::invalid_argument);
  EXPECT_THROW(Dungeon2D(10, 4), std::invalid_argument);
  EXPECT_THROW(Dungeon2D(0, 0), std::invalid_argument);
  EXPECT_THROW(Dungeon2D(-5, 10), std::invalid_argument);
}

TEST(Dungeon2D, RootCoversWholeArea) {
  Dungeon2D d(40, 30);
  d.generate(1);
  ASSERT_NE(d.root, nullptr);
  EXPECT_EQ(d.root->region.left, 0);
  EXPECT_EQ(d.root->region.top, 0);
  EXPECT_EQ(d.root->region.right, 39);
  EXPECT_EQ(d.root->region.bottom, 29);
}

TEST(Dungeon2D, LargeDungeonIsPartitioned) {
  Dungeon2D d(80, 60);
  d.generate(3);
  std::vector<const BSPNode *> leaves;
  collectLeaves(*d.root, leaves);
  EXPECT_GT(leaves.size(), 1u);
  EXPECT_FALSE(d.corridors.empty());
}

TEST(Dungeon2D, SameSeedSameDungeon) {
  Dungeon2D a(60, 40), b(60, 40);
  a.generate(42);
  b.generate(42);

  std::vector<const BSPNode *> la, lb;
  collectLeaves(*a.root, la);
  collectLeaves(*b.root, lb);
  ASSERT_EQ(la.size(), lb.size());
  for (std::size_t i = 0; i < la.size(); ++i) {
    EXPECT_EQ(la[i]->room.left, lb[i]->room.left);
    EXPECT_EQ(la[i]->room.top, lb[i]->room.top);
    EXPECT_EQ(la[i]->room.right, lb[i]->room.right);
    EXPECT_EQ(la[i]->room.bottom, lb[i]->room.bottom);
  }
  ASSERT_EQ(a.corridors.size(), b.corridors.size());
  for (std::size_t i = 0; i < a.corridors.size(); ++i) {
    EXPECT_EQ(a.corridors[i].left, b.corridors[i].left);
    EXPECT_EQ(a.corridors[i].top, b.corridors[i].top);
    EXPECT_EQ(a.corridors[i].right, b.corridors[i].right);
    EXPECT_EQ(a.corridors[i].bottom, b.corridors[i].bottom);
  }
}

TEST(Dungeon2D, RegenerateReplacesPreviousGeometry) {
  Dungeon2D d(60, 40);
  d.generate(1);
  d.generate(2);
  Dungeon2D fresh(60, 40);
  fresh.generate(2);
  EXPECT_EQ(d.corridors.size(), fresh.corridors.size());
}

TEST(Dungeon2D, DifferentSeedsUsuallyDiffer) {
  TileMap a, b;
  Dungeon2D d(60, 40);
  d.generate(1);
  a.loadFromDungeon(d);
  d.generate(2);
  b.loadFromDungeon(d);
  EXPECT_FALSE(std::equal(a.data(), a.data() + 60 * 40, b.data()));
}

// --- Geometric constraints, over many sizes and seeds ----------------------

TEST(Dungeon2D, TreeRegionsPartitionParent) {
  for (const Size &s : kSizes)
    for (unsigned seed : kSeeds) {
      Dungeon2D d(s.w, s.h);
      d.generate(seed);
      checkTree(*d.root);
    }
}

TEST(Dungeon2D, RoomsAreAtLeast5x5AndInsideTheirRegion) {
  for (const Size &s : kSizes)
    for (unsigned seed : kSeeds) {
      Dungeon2D d(s.w, s.h);
      d.generate(seed);
      std::vector<const BSPNode *> leaves;
      collectLeaves(*d.root, leaves);
      for (const BSPNode *leaf : leaves) {
        EXPECT_GE(rectWidth(leaf->room), 5);
        EXPECT_GE(rectHeight(leaf->room), 5);
        EXPECT_TRUE(contains(leaf->region, leaf->room));
      }
    }
}

TEST(Dungeon2D, RoomsDoNotOverlap) {
  for (unsigned seed : kSeeds) {
    Dungeon2D d(80, 60);
    d.generate(seed);
    std::vector<const BSPNode *> leaves;
    collectLeaves(*d.root, leaves);
    for (std::size_t i = 0; i < leaves.size(); ++i)
      for (std::size_t j = i + 1; j < leaves.size(); ++j)
        EXPECT_FALSE(overlaps(leaves[i]->room, leaves[j]->room));
  }
}

TEST(Dungeon2D, CorridorsAre3WideAndInBounds) {
  for (const Size &s : kSizes)
    for (unsigned seed : kSeeds) {
      Dungeon2D d(s.w, s.h);
      d.generate(seed);
      for (const Corridor &c : d.corridors) {
        EXPECT_EQ(std::min(rectWidth(c), rectHeight(c)), 3);
        EXPECT_TRUE(contains(d.root->region, c));
      }
    }
}

TEST(Dungeon2D, AllFloorIsConnected) {
  for (const Size &s : kSizes)
    for (unsigned seed : kSeeds) {
      Dungeon2D d(s.w, s.h);
      d.generate(seed);
      TileMap map;
      map.loadFromDungeon(d);
      const int empty = static_cast<int>(
          std::count(map.data(), map.data() + s.w * s.h, EMPTY_TILE));
      EXPECT_GT(empty, 0);
      EXPECT_EQ(reachableEmpty(map), empty)
          << "size " << s.w << "x" << s.h << ", seed " << seed;
    }
}
