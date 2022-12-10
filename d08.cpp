#include "days.hpp"
#include "utils.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <vector>

using forest = std::vector<std::string>;

forest parse_forest(std::istream& input) {
  forest sent;
  for (std::string line; std::getline(input, line);) {
    sent.push_back(std::move(line));
  }
  return sent;
}

using visibility = std::vector<std::vector<bool>>;
using pos = std::pair<size_t, size_t>;

template<std::ranges::input_range Input>
auto filter_visible(Input&& i, const forest& f) {
  return i
    | std::views::filter([tallest = std::optional<char>{}, &f](const pos& p) mutable {
        char tree = f[p.second][p.first];
        bool visible = !tallest || tallest < tree;
        if (visible) {
          tallest = tree;
        }
        return visible;
      })
    ;
}

visibility map_visibility(const forest& f) {
  size_t height = f.size();
  size_t width = f[0].size();
  visibility v;
  v.resize(height, std::vector<bool>(width, false));

  for (size_t y = 0; y < height; ++y) {
    auto left_to_right = std::views::iota(size_t{ 0 }, width)
      | std::views::transform([y](size_t x) {
          return pos{ x, y };
        })
    ;
    for (const auto [vx, vy] : filter_visible(left_to_right, f)) {
      v[vy][vx] = true;
    }

    auto right_to_left = std::views::iota(size_t{ 0 }, width)
      | std::views::reverse
      | std::views::transform([y](size_t x) {
          return pos{ x, y };
        })
    ;
    for (const auto [vx, vy] : filter_visible(right_to_left, f)) {
      v[vy][vx] = true;
    }
  }

  for (size_t x = 0; x < width; ++x) {
    auto top_to_bottom = std::views::iota(size_t{ 0 }, height)
      | std::views::transform([x](size_t y) {
          return pos{ x, y };
        })
    ;
    for (const auto [vx, vy] : filter_visible(top_to_bottom, f)) {
      v[vy][vx] = true;
    }

    auto bottom_to_top = std::views::iota(size_t{ 0 }, height)
      | std::views::reverse
      | std::views::transform([x](size_t y) {
          return pos{ x, y };
        })
    ;
    for (const auto [vx, vy] : filter_visible(bottom_to_top, f)) {
      v[vy][vx] = true;
    }
  }
  return v;
}

size_t count_visible(const visibility& v) {
  return ranges::reduce(v | std::views::transform([](const std::vector<bool>& row) { return std::ranges::count(row, true); }));
}

using scenic_score = std::vector<std::vector<size_t>>;

scenic_score map_scenic_score(const forest& f) {
  size_t height = f.size();
  size_t width = f[0].size();
  auto res = scenic_score(height, std::vector<size_t>(width, 0));
  const auto count_all = [](auto&& input) { return std::ranges::count_if(input, [](const auto&) { return true; }); };

  for (size_t x = 1; x < width - 1; ++x) {
    for (size_t y = 1; y < height - 1; ++y) {
      char reference = f[y][x];

      auto until_ref = [reference, &f](std::ranges::input_range auto&& input) {
        auto e = std::ranges::find_if(input, [&](const pos& p) { return f[p.second][p.first] >= reference; });
        if (e != std::ranges::end(input)) {
          ++e;
        }
        return std::ranges::distance(input.begin(), e);
      };

      auto top_score = until_ref(std::views::iota(size_t{ 0 }, y) | std::views::reverse | std::views::transform([x](size_t y) { return pos{ x, y }; }));
      auto left_score = until_ref(std::views::iota(size_t{ 0 }, x) | std::views::reverse | std::views::transform([y](size_t x) { return pos{ x, y }; }));
      auto down_score = until_ref(std::views::iota(y + 1, height) | std::views::transform([x](size_t y) { return pos{ x, y }; }));
      auto right_score = until_ref(std::views::iota(x + 1, width) | std::views::transform([y](size_t x) { return pos{x, y}; }));
      res[y][x] = static_cast<size_t>(top_score * left_score * down_score * right_score);
    }
  }
  return res;
}

REGISTER_DAY("d08",
  [](std::istream& input) {
    return std::to_string(count_visible(map_visibility(parse_forest(input))));
  },
  [](std::istream& input) {
    auto scores = map_scenic_score(parse_forest(input));
    size_t m = std::ranges::max(scores | std::views::transform([](const auto & row) { return std::ranges::max(row); }));
    return std::to_string(m);
  }
)

#include <iostream>

TEST(d08, visible) {
  auto f = forest{
    {"30373"},
    {"25512"},
    {"65332"},
    {"33549"},
    {"35390"}
  };
  auto vis = map_visibility(f);

  for (const auto& row : vis) {
    for (bool v : row) {
      std::cout << (v ? '+' : '.');
    }
    std::cout << std::endl;
  }

  EXPECT_TRUE(vis[1][1]);
  EXPECT_TRUE(vis[1][2]);
  EXPECT_TRUE(vis[2][1]);
  EXPECT_TRUE(vis[2][3]);
  EXPECT_TRUE(vis[3][2]);
  EXPECT_EQ(count_visible(vis), 21);
}

TEST(d08, sccenic_score) {
  auto f = forest{
    {"30373"},
    {"25512"},
    {"65332"},
    {"33549"},
    {"35390"}
  };
  auto score = map_scenic_score(f);

  for (const auto& row : score) {
    for (auto s : row) {
      std::cout << s << ' ';
    }
    std::cout << std::endl;
  }

  EXPECT_EQ(score[1][2], 4);
  EXPECT_EQ(score[3][2], 8);
}