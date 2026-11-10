#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "Dungeon2D.hpp"
#include "TileMap.hpp"

namespace {

// A temporary file path, removed when the object goes out of scope.
class TempFile {
public:
  explicit TempFile(const std::string &name)
      : m_path(::testing::TempDir() + name) {}
  ~TempFile() { std::remove(m_path.c_str()); }
  const std::string &path() const { return m_path; }

  void write(const std::string &contents) const {
    std::ofstream(m_path, std::ios::binary) << contents;
  }

  std::string read() const {
    std::ifstream in(m_path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), {});
  }

private:
  std::string m_path;
};

// A hand-built 6x4 dungeon: one room and one corridor.
//   col: 012345
//   r0   ######
//   r1   #..###      room (1,1)-(2,2)
//   r2   #.....      corridor (3,2)-(5,2)
//   r3   ######
Dungeon2D smallDungeon() {
  Dungeon2D d(6, 5);
  d.height = 4;
  d.root = std::make_unique<BSPNode>();
  d.root->region = Rect{0, 0, 5, 3};
  d.root->room = Rect{1, 1, 2, 2};
  d.corridors.push_back(Corridor{3, 2, 5, 2});
  return d;
}

TileMap smallMap() {
  TileMap m;
  m.loadFromDungeon(smallDungeon());
  return m;
}

} // namespace

TEST(TileMap, DefaultIsEmpty) {
  const TileMap m;
  EXPECT_EQ(m.width(), 0);
  EXPECT_EQ(m.height(), 0);
}

// --- loadFromDungeon -------------------------------------------------------

TEST(TileMap, LoadFromDungeonRasterizesRoomsAndCorridors) {
  const TileMap m = smallMap();
  ASSERT_EQ(m.width(), 6);
  ASSERT_EQ(m.height(), 4);

  const int expected[4][6] = {
      {1, 1, 1, 1, 1, 1},
      {1, 0, 0, 1, 1, 1},
      {1, 0, 0, 0, 0, 0},
      {1, 1, 1, 1, 1, 1},
  };
  for (int r = 0; r < 4; ++r)
    for (int c = 0; c < 6; ++c) {
      EXPECT_EQ(m.at(c, r), expected[r][c]) << "col " << c << " row " << r;
      EXPECT_EQ(m.isWall(c, r), expected[r][c] == WALL_TILE);
      EXPECT_EQ(m.data()[r * 6 + c], expected[r][c]);
    }
}

TEST(TileMap, LoadFromDungeonWithoutTreeIsAllWall) {
  const Dungeon2D d(8, 6);
  TileMap m;
  m.loadFromDungeon(d);
  ASSERT_EQ(m.width(), 8);
  ASSERT_EQ(m.height(), 6);
  for (int i = 0; i < 8 * 6; ++i)
    EXPECT_EQ(m.data()[i], WALL_TILE);
}

TEST(TileMap, LoadFromGeneratedDungeonCarvesEveryRoom) {
  Dungeon2D d(60, 40);
  d.generate(5);
  TileMap m;
  m.loadFromDungeon(d);

  std::vector<const BSPNode *> stack{d.root.get()};
  while (!stack.empty()) {
    const BSPNode *n = stack.back();
    stack.pop_back();
    if (!n->isLeaf()) {
      stack.push_back(n->left.get());
      stack.push_back(n->right.get());
      continue;
    }
    for (int r = n->room.top; r <= n->room.bottom; ++r)
      for (int c = n->room.left; c <= n->room.right; ++c)
        EXPECT_FALSE(m.isWall(c, r));
  }
  for (const Corridor &k : d.corridors)
    for (int r = k.top; r <= k.bottom; ++r)
      for (int c = k.left; c <= k.right; ++c)
        EXPECT_FALSE(m.isWall(c, r));
}

// --- CSV -------------------------------------------------------------------

TEST(TileMap, SaveCSVExactFormat) {
  const TempFile f("tilemap_save.csv");
  ASSERT_TRUE(smallMap().saveCSV(f.path()));
  EXPECT_EQ(f.read(), "1,1,1,1,1,1\n"
                      "1,0,0,1,1,1\n"
                      "1,0,0,0,0,0\n"
                      "1,1,1,1,1,1\n");
}

TEST(TileMap, CSVRoundTrip) {
  Dungeon2D d(50, 30);
  d.generate(11);
  TileMap original;
  original.loadFromDungeon(d);

  const TempFile f("tilemap_roundtrip.csv");
  ASSERT_TRUE(original.saveCSV(f.path()));

  TileMap loaded;
  ASSERT_TRUE(loaded.loadCSV(f.path()));
  ASSERT_EQ(loaded.width(), original.width());
  ASSERT_EQ(loaded.height(), original.height());
  EXPECT_TRUE(std::equal(original.data(), original.data() + 50 * 30,
                         loaded.data()));
}

TEST(TileMap, LoadCSVWithoutFinalNewline) {
  const TempFile f("tilemap_nonl.csv");
  f.write("0,1\n1,0");
  TileMap m;
  ASSERT_TRUE(m.loadCSV(f.path()));
  EXPECT_EQ(m.width(), 2);
  EXPECT_EQ(m.height(), 2);
  EXPECT_EQ(m.at(1, 0), 1);
  EXPECT_EQ(m.at(1, 1), 0);
}

TEST(TileMap, LoadCSVWithWindowsLineEndings) {
  const TempFile f("tilemap_crlf.csv");
  f.write("0,1\r\n1,0\r\n");
  TileMap m;
  ASSERT_TRUE(m.loadCSV(f.path()));
  EXPECT_EQ(m.width(), 2);
  EXPECT_EQ(m.height(), 2);
  EXPECT_EQ(m.at(1, 0), 1);
  EXPECT_EQ(m.at(1, 1), 0);
}

TEST(TileMap, SaveCSVFailsOnBadPath) {
  EXPECT_FALSE(smallMap().saveCSV("/nonexistent-dir/x/y.csv"));
}

TEST(TileMap, LoadCSVFailsOnMissingFile) {
  TileMap m = smallMap();
  EXPECT_FALSE(m.loadCSV("/nonexistent-dir/x/y.csv"));
  EXPECT_EQ(m.width(), 6);
}

TEST(TileMap, LoadCSVRejectsMalformedAndLeavesMapUnchanged) {
  const char *bad[] = {
      "",               // empty file
      "1,0,\n0,1,\n",   // trailing comma
      "1,0\n0\n",       // ragged rows
      "1, 0\n0,1\n",    // space
      "1,,0\n",         // empty field
      "1,x\n",          // not a number
      "1,0\n\n0,1\n",   // blank line
      "1;0\n",          // wrong separator
      "1,0\r\r\n",      // stray carriage return
  };
  int i = 0;
  for (const char *contents : bad) {
    const TempFile f("tilemap_bad" + std::to_string(i++) + ".csv");
    f.write(contents);
    TileMap m = smallMap();
    EXPECT_FALSE(m.loadCSV(f.path())) << "accepted: \"" << contents << "\"";
    EXPECT_EQ(m.width(), 6);
    EXPECT_EQ(m.height(), 4);
    EXPECT_EQ(m.at(1, 1), EMPTY_TILE);
  }
}

// --- Coordinate translation ------------------------------------------------

TEST(TileMap, ToIndicesFloorsWorldPosition) {
  const TileMap m;
  int col = -1, row = -1;

  m.toIndices(Vector2D(0, 0), col, row);
  EXPECT_EQ(col, 0);
  EXPECT_EQ(row, 0);

  m.toIndices(Vector2D(63.9f, 64.0f), col, row);
  EXPECT_EQ(col, 0);
  EXPECT_EQ(row, 1);

  m.toIndices(Vector2D(3 * TILE_SIZE + 10, 7 * TILE_SIZE + 63), col, row);
  EXPECT_EQ(col, 3);
  EXPECT_EQ(row, 7);

  m.toIndices(Vector2D(-0.1f, -64.5f), col, row); // floor, not truncation
  EXPECT_EQ(col, -1);
  EXPECT_EQ(row, -2);
}

TEST(TileMap, ToWorldGivesCellCentre) {
  const TileMap m;
  const Vector2D p = m.toWorld(0, 0);
  EXPECT_FLOAT_EQ(p.x, 32.0f);
  EXPECT_FLOAT_EQ(p.y, 32.0f);
  const Vector2D q = m.toWorld(3, 5);
  EXPECT_FLOAT_EQ(q.x, 3.5f * TILE_SIZE);
  EXPECT_FLOAT_EQ(q.y, 5.5f * TILE_SIZE);
}

TEST(TileMap, ToIndicesInvertsToWorld) {
  const TileMap m;
  for (int r = -3; r < 10; ++r)
    for (int c = -3; c < 10; ++c) {
      int col, row;
      m.toIndices(m.toWorld(c, r), col, row);
      EXPECT_EQ(col, c);
      EXPECT_EQ(row, r);
    }
}

// --- randomEmptyCell -------------------------------------------------------

TEST(TileMap, RandomEmptyCellIsAlwaysEmpty) {
  Dungeon2D d(40, 30);
  d.generate(8);
  TileMap m;
  m.loadFromDungeon(d);
  for (int i = 0; i < 500; ++i) {
    int col = -1, row = -1;
    m.randomEmptyCell(col, row);
    ASSERT_GE(col, 0);
    ASSERT_LT(col, m.width());
    ASSERT_GE(row, 0);
    ASSERT_LT(row, m.height());
    EXPECT_EQ(m.at(col, row), EMPTY_TILE);
  }
}

TEST(TileMap, RandomEmptyCellReachesEveryEmptyCell) {
  const TileMap m = smallMap(); // 7 empty cells
  std::vector<int> hits(6 * 4, 0);
  for (int i = 0; i < 2000; ++i) {
    int col, row;
    m.randomEmptyCell(col, row);
    ++hits[row * 6 + col];
  }
  for (int i = 0; i < 6 * 4; ++i) {
    if (m.data()[i] == EMPTY_TILE)
      EXPECT_GT(hits[i], 0) << "cell " << i << " never chosen";
    else
      EXPECT_EQ(hits[i], 0);
  }
}
