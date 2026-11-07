#include <algorithm>
#include <limits>
#include <random>
#include <stdexcept>

#include "Dungeon2D.hpp"

// Rect bounds are INCLUSIVE: a Rect covers columns left..right and rows
// top..bottom, so its size is (right - left + 1) x (bottom - top + 1).

namespace {
constexpr int kMinRoom = 5;      // smallest room side, in tiles
constexpr int kRoomPadding = 1;  // wall tiles kept between a room and its region
constexpr int kMinLeaf = kMinRoom + 2 * kRoomPadding; // smallest leaf region side
constexpr int kMaxLeaf = 24;     // leaves larger than this are always split
constexpr int kCorridorHalf = 1; // corridor width = 2 * kCorridorHalf + 1 = 3

int rectWidth(const Rect &r) { return r.right - r.left + 1; }
int rectHeight(const Rect &r) { return r.bottom - r.top + 1; }

int randInt(std::mt19937 &rng, int lo, int hi) {
  return std::uniform_int_distribution<int>(lo, hi)(rng);
}

void split(BSPNode &node, std::mt19937 &rng) {
  const int w = rectWidth(node.region);
  const int h = rectHeight(node.region);
  const bool canSplitX = w >= 2 * kMinLeaf;
  const bool canSplitY = h >= 2 * kMinLeaf;

  if (!canSplitX && !canSplitY)
    return;

  // Small enough regions sometimes stay whole, giving varied room sizes.
  if (w <= kMaxLeaf && h <= kMaxLeaf && randInt(rng, 0, 3) == 0)
    return;

  bool splitX;
  if (canSplitX && canSplitY) {
    // Prefer cutting across the longer side to avoid thin regions.
    if (w > h * 5 / 4)
      splitX = true;
    else if (h > w * 5 / 4)
      splitX = false;
    else
      splitX = randInt(rng, 0, 1) == 0;
  } else {
    splitX = canSplitX;
  }

  const Rect &r = node.region;
  node.left = std::make_unique<BSPNode>();
  node.right = std::make_unique<BSPNode>();

  if (splitX) {
    const int cut = randInt(rng, r.left + kMinLeaf - 1, r.right - kMinLeaf);
    node.left->region = Rect{r.left, r.top, cut, r.bottom};
    node.right->region = Rect{cut + 1, r.top, r.right, r.bottom};
  } else {
    const int cut = randInt(rng, r.top + kMinLeaf - 1, r.bottom - kMinLeaf);
    node.left->region = Rect{r.left, r.top, r.right, cut};
    node.right->region = Rect{r.left, cut + 1, r.right, r.bottom};
  }

  split(*node.left, rng);
  split(*node.right, rng);
}

// Chooses one side's [lo, hi] room span inside a region span [lo, hi].
void placeSpan(int regionLo, int regionHi, std::mt19937 &rng, int &lo,
               int &hi) {
  const int size = regionHi - regionLo + 1;
  // A region too small for padding (only possible for the root) gets none.
  const int pad = size >= kMinLeaf ? kRoomPadding : 0;
  const int available = size - 2 * pad;
  const int roomSize = randInt(rng, kMinRoom, available);
  lo = randInt(rng, regionLo + pad, regionHi - pad - roomSize + 1);
  hi = lo + roomSize - 1;
}

void carveRooms(BSPNode &node, std::mt19937 &rng) {
  if (node.isLeaf()) {
    placeSpan(node.region.left, node.region.right, rng, node.room.left,
              node.room.right);
    placeSpan(node.region.top, node.region.bottom, rng, node.room.top,
              node.room.bottom);
    return;
  }
  carveRooms(*node.left, rng);
  carveRooms(*node.right, rng);
}

void collectRooms(const BSPNode &node, std::vector<const Rect *> &out) {
  if (node.isLeaf()) {
    out.push_back(&node.room);
    return;
  }
  collectRooms(*node.left, out);
  collectRooms(*node.right, out);
}

int centerX(const Rect &r) { return (r.left + r.right) / 2; }
int centerY(const Rect &r) { return (r.top + r.bottom) / 2; }

Corridor horizontal(int x0, int x1, int y) {
  return Corridor{std::min(x0, x1) - kCorridorHalf, y - kCorridorHalf,
                  std::max(x0, x1) + kCorridorHalf, y + kCorridorHalf};
}

Corridor vertical(int x, int y0, int y1) {
  return Corridor{x - kCorridorHalf, std::min(y0, y1) - kCorridorHalf,
                  x + kCorridorHalf, std::max(y0, y1) + kCorridorHalf};
}

// Joins the two subtrees of every internal node with an L-shaped corridor
// between their closest pair of rooms. Since each subtree is connected by
// induction, the whole dungeon ends up connected.
void connect(const BSPNode &node, std::mt19937 &rng,
             std::vector<Corridor> &corridors) {
  if (node.isLeaf())
    return;

  connect(*node.left, rng, corridors);
  connect(*node.right, rng, corridors);

  std::vector<const Rect *> roomsA, roomsB;
  collectRooms(*node.left, roomsA);
  collectRooms(*node.right, roomsB);

  const Rect *a = nullptr;
  const Rect *b = nullptr;
  long best = std::numeric_limits<long>::max();
  for (const Rect *ra : roomsA) {
    for (const Rect *rb : roomsB) {
      const long dx = centerX(*ra) - centerX(*rb);
      const long dy = centerY(*ra) - centerY(*rb);
      const long d = dx * dx + dy * dy;
      if (d < best) {
        best = d;
        a = ra;
        b = rb;
      }
    }
  }

  // Room centres are at least 2 tiles from the room edges (rooms are >= 5
  // wide), so a 3-wide corridor ending at a centre lies inside the room.
  const int ax = centerX(*a), ay = centerY(*a);
  const int bx = centerX(*b), by = centerY(*b);

  if (randInt(rng, 0, 1) == 0) {
    corridors.push_back(horizontal(ax, bx, ay));
    corridors.push_back(vertical(bx, ay, by));
  } else {
    corridors.push_back(vertical(ax, ay, by));
    corridors.push_back(horizontal(ax, bx, by));
  }
}
} // namespace

Dungeon2D::Dungeon2D(int widthTiles, int heightTiles)
    : width(widthTiles), height(heightTiles) {
  if (widthTiles < kMinRoom || heightTiles < kMinRoom) {
    throw std::invalid_argument("Dungeon2D: width and height must be at least "
                                "5 tiles (the minimum room size)");
  }
}

void Dungeon2D::generate(unsigned seed) {
  std::mt19937 rng(seed);

  corridors.clear();
  root = std::make_unique<BSPNode>();
  root->region = Rect{0, 0, width - 1, height - 1};

  split(*root, rng);
  carveRooms(*root, rng);
  connect(*root, rng, corridors);
}
