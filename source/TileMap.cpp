#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>

#include "Dungeon2D.hpp"
#include "TileMap.hpp"

namespace {
// Sets every cell of an inclusive Rect to value, clipped to the map bounds.
void fillRect(std::vector<int> &cells, int width, int height, const Rect &r,
              int value) {
  const int left = std::max(r.left, 0);
  const int top = std::max(r.top, 0);
  const int right = std::min(r.right, width - 1);
  const int bottom = std::min(r.bottom, height - 1);

  for (int row = top; row <= bottom; ++row)
    for (int col = left; col <= right; ++col)
      cells[row * width + col] = value;
}

void carveLeaves(const BSPNode &node, std::vector<int> &cells, int width,
                 int height) {
  if (node.isLeaf()) {
    fillRect(cells, width, height, node.room, EMPTY_TILE);
    return;
  }
  carveLeaves(*node.left, cells, width, height);
  carveLeaves(*node.right, cells, width, height);
}

// Parses a whole token as a base-10 int, as written by saveCSV(): an optional
// '-' followed by digits only (no spaces, '+' or trailing characters).
bool parseInt(const std::string &token, int &out) {
  if (token.empty() ||
      !(token[0] == '-' || std::isdigit(static_cast<unsigned char>(token[0]))))
    return false;
  std::size_t used = 0;
  try {
    out = std::stoi(token, &used);
  } catch (const std::exception &) {
    return false;
  }
  return used == token.size();
}
} // namespace

void TileMap::loadFromDungeon(const Dungeon2D &dungeon) {
  m_width = dungeon.width;
  m_height = dungeon.height;
  m_cells.assign(static_cast<std::size_t>(m_width) * m_height, WALL_TILE);

  if (dungeon.root)
    carveLeaves(*dungeon.root, m_cells, m_width, m_height);

  for (const Corridor &c : dungeon.corridors)
    fillRect(m_cells, m_width, m_height, c, EMPTY_TILE);
}

bool TileMap::saveCSV(const std::string &path) const {
  std::ofstream out(path);
  if (!out)
    return false;

  for (int row = 0; row < m_height; ++row) {
    for (int col = 0; col < m_width; ++col) {
      if (col > 0)
        out << ',';
      out << m_cells[row * m_width + col];
    }
    out << '\n';
  }

  return static_cast<bool>(out);
}

bool TileMap::loadCSV(const std::string &path) {
  std::ifstream in(path);
  if (!in)
    return false;

  std::vector<int> cells;
  int width = 0;
  int height = 0;
  std::string line;

  while (std::getline(in, line)) {
    // Files saved on Windows end each line with "\r\n".
    if (!line.empty() && line.back() == '\r')
      line.pop_back();

    std::vector<int> row;
    std::string token;
    std::istringstream fields(line);
    while (std::getline(fields, token, ',')) {
      int value = 0;
      if (!parseInt(token, value))
        return false;
      row.push_back(value);
    }
    // getline drops a final empty field, so a trailing comma must be caught
    // explicitly.
    if (row.empty() || line.back() == ',')
      return false;

    if (height == 0)
      width = static_cast<int>(row.size());
    else if (static_cast<int>(row.size()) != width)
      return false;

    cells.insert(cells.end(), row.begin(), row.end());
    ++height;
  }

  if (in.bad() || height == 0)
    return false;

  m_width = width;
  m_height = height;
  m_cells = std::move(cells);
  return true;
}

int TileMap::width() const noexcept { return m_width; }

int TileMap::height() const noexcept { return m_height; }

int TileMap::at(int col, int row) const {
  assert(col >= 0 && col < m_width && row >= 0 && row < m_height);
  return m_cells[row * m_width + col];
}

bool TileMap::isWall(int col, int row) const {
  return at(col, row) == WALL_TILE;
}

const int *TileMap::data() const noexcept { return m_cells.data(); }

void TileMap::toIndices(const Vector2D &world, int &col,
                        int &row) const noexcept {
  col = static_cast<int>(std::floor(world.x / TILE_SIZE));
  row = static_cast<int>(std::floor(world.y / TILE_SIZE));
}

Vector2D TileMap::toWorld(int col, int row) const noexcept {
  return Vector2D((static_cast<float>(col) + 0.5f) * TILE_SIZE,
                  (static_cast<float>(row) + 0.5f) * TILE_SIZE);
}

void TileMap::randomEmptyCell(int &col, int &row) const {
  const auto emptyCount =
      std::count(m_cells.begin(), m_cells.end(), EMPTY_TILE);
  if (emptyCount == 0)
    throw std::logic_error("TileMap::randomEmptyCell: the map has no empty cell");

  static std::mt19937 rng{std::random_device{}()};
  auto pick = std::uniform_int_distribution<long>(0, emptyCount - 1)(rng);

  for (std::size_t i = 0; i < m_cells.size(); ++i) {
    if (m_cells[i] != EMPTY_TILE)
      continue;
    if (pick-- == 0) {
      col = static_cast<int>(i) % m_width;
      row = static_cast<int>(i) / m_width;
      return;
    }
  }
}
