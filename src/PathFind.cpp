#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <queue>

#include "PathFind.hpp"

namespace {
constexpr float kOrthogonalCost = 1.0f;
constexpr float kDiagonalCost = 1.41421356f;

// Octile distance: exact cost between two cells on an empty 8-connected grid,
// so it never overestimates and A* returns a shortest path.
float heuristic(int col, int row, int goalCol, int goalRow) {
  const int dx = std::abs(col - goalCol);
  const int dy = std::abs(row - goalRow);
  return kOrthogonalCost * static_cast<float>(std::max(dx, dy)) +
         (kDiagonalCost - kOrthogonalCost) *
             static_cast<float>(std::min(dx, dy));
}

struct OpenEntry {
  float f;
  int index;
  bool operator>(const OpenEntry &other) const { return f > other.f; }
};
} // namespace

PathFind::PathFind(const int *grid, int width, int height)
    : m_grid(grid), m_width(width), m_height(height) {}

std::vector<Vector2D> PathFind::findPath(const Vector2D &startWorld,
                                         const Vector2D &goalWorld) const {
  if (!m_grid || m_width <= 0 || m_height <= 0)
    return {};

  const auto walkable = [this](int col, int row) {
    return col >= 0 && col < m_width && row >= 0 && row < m_height &&
           m_grid[row * m_width + col] == EMPTY_TILE;
  };

  const int startCol = static_cast<int>(std::floor(startWorld.x / TILE_SIZE));
  const int startRow = static_cast<int>(std::floor(startWorld.y / TILE_SIZE));
  const int goalCol = static_cast<int>(std::floor(goalWorld.x / TILE_SIZE));
  const int goalRow = static_cast<int>(std::floor(goalWorld.y / TILE_SIZE));

  if (!walkable(startCol, startRow) || !walkable(goalCol, goalRow))
    return {};

  const int cellCount = m_width * m_height;
  const int startIndex = startRow * m_width + startCol;
  const int goalIndex = goalRow * m_width + goalCol;

  std::vector<float> g(cellCount, std::numeric_limits<float>::infinity());
  std::vector<int> parent(cellCount, -1);
  std::vector<bool> closed(cellCount, false);
  std::priority_queue<OpenEntry, std::vector<OpenEntry>, std::greater<>> open;

  g[startIndex] = 0.0f;
  open.push({heuristic(startCol, startRow, goalCol, goalRow), startIndex});

  while (!open.empty()) {
    const int current = open.top().index;
    open.pop();
    // The queue may hold stale duplicates of a cell already expanded.
    if (closed[current])
      continue;
    closed[current] = true;
    if (current == goalIndex)
      break;

    const int col = current % m_width;
    const int row = current / m_width;

    for (int dy = -1; dy <= 1; ++dy) {
      for (int dx = -1; dx <= 1; ++dx) {
        if (dx == 0 && dy == 0)
          continue;
        const int nCol = col + dx;
        const int nRow = row + dy;
        if (!walkable(nCol, nRow))
          continue;
        const bool diagonal = dx != 0 && dy != 0;
        // No corner cutting: both orthogonal neighbours must be free.
        if (diagonal && (!walkable(col + dx, row) || !walkable(col, row + dy)))
          continue;

        const int next = nRow * m_width + nCol;
        if (closed[next])
          continue;
        const float tentative =
            g[current] + (diagonal ? kDiagonalCost : kOrthogonalCost);
        if (tentative < g[next]) {
          g[next] = tentative;
          parent[next] = current;
          open.push({tentative + heuristic(nCol, nRow, goalCol, goalRow), next});
        }
      }
    }
  }

  if (!closed[goalIndex])
    return {};

  std::vector<Vector2D> path;
  for (int index = goalIndex; index != -1; index = parent[index]) {
    const int col = index % m_width;
    const int row = index / m_width;
    path.emplace_back((static_cast<float>(col) + 0.5f) * TILE_SIZE,
                      (static_cast<float>(row) + 0.5f) * TILE_SIZE);
  }
  std::reverse(path.begin(), path.end());
  return path;
}
