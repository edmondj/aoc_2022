#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>

using dist = int64_t;

struct pos {
  dist x, y;
  auto operator<=>(const pos&) const = default;

  pos operator+(const pos& r) const {
    return pos{ x + r.x, y + r.y };
  }

  pos& operator+=(const pos& r) {
    *this = *this + r;
    return *this;
  }
};

struct topography {
  std::set<pos> blocks;
  dist depth = 0;

  bool drop_sand(bool with_abyss = true) {
    static constexpr auto candidates = { pos{0, 1}, pos{-1, 1}, pos{1, 1} };
    static constexpr auto start = pos{ 500, 0 };

    if (blocks.contains(start)) {
      return false;
    }

    auto sand = start;
    while (sand.y < depth) {
      bool moved = false;
      for (const pos& c : candidates) {
        if (!blocks.contains(sand + c)) {
          sand += c;
          moved = true;
          break;
        }
      }
      if (!moved) {
        blocks.insert(sand);
        return true;
      }
    }
    if (with_abyss) {
      return false;
    }
    blocks.insert(sand);
    return true;
  }
};

pos parse_pos(std::string_view s) {
  auto found = s.find(',');
  if (found == std::string_view::npos) {
    throw std::runtime_error("invalid format");
  }
  return pos{
    .x = string_view_to<dist>(s.substr(0, found)),
    .y = string_view_to<dist>(s.substr(found + 1))
  };
}

topography parse_topography(std::istream& input) {
  topography sent;
  for (std::string line; std::getline(input, line);) {
    std::optional<pos> prev;
    for (const pos& p : std::string_view(line)
      | std::views::split(std::string_view(" -> "))
      | std::views::transform([](auto&& subrange) {
          return parse_pos(std::string_view(subrange.begin(), subrange.end()));
        })
      ) {
      if (prev) {
        for (pos cur = *prev; cur != p; cur += pos{ sign_of(p.x - cur.x), sign_of(p.y - cur.y) }) {
          sent.blocks.insert(cur);
          sent.depth = std::max(sent.depth, cur.y);
        }
      }
      sent.blocks.insert(p);
      sent.depth = std::max(sent.depth, p.y);
      prev = p;
    }
  }
  return sent;
}

REGISTER_DAY("d14",
  [](std::istream& input) {
    auto t = parse_topography(input);
    size_t sand_count = 0;
    while (t.drop_sand()) {
      ++sand_count;
    }
    return std::to_string(sand_count);
  },
  [](std::istream& input) {
    auto t = parse_topography(input);
    t.depth += 1;
    size_t sand_count = 0;
    while (t.drop_sand(false)) {
      ++sand_count;
    }
    return std::to_string(sand_count);
  }
)

TEST(d14, parse) {
  auto input = std::istringstream(R"(
498,4 -> 498,6 -> 496,6
503,4 -> 502,4 -> 502,9 -> 494,9
)");
  input.get();
  auto topo = parse_topography(input);
  EXPECT_EQ(topo.depth, 9);
  EXPECT_EQ(topo.blocks.size(), 20);
}


TEST(d14, part2) {
    auto input = std::istringstream(R"(
498,4 -> 498,6 -> 496,6
503,4 -> 502,4 -> 502,9 -> 494,9
)");
    input.get();
    auto topo = parse_topography(input);

    size_t i = 0;
    ++topo.depth;
    while (topo.drop_sand(false)) {
      ++i;
    }
    EXPECT_EQ(i, 93);
}
