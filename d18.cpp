#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>

namespace d18 {
  struct pos_t {
    int x, y, z;

    auto operator<=>(const pos_t&) const = default;

    pos_t operator+(const pos_t& r) const {
      return { x + r.x, y + r.y, z + r.z };
    }
  };

  pos_t parse_pos(std::string_view s) {
    static const auto delim = std::string{ "," };
    union {
      pos_t sent;
      int arr[3];
    };
    int i = 0;
    for (auto&& subrange : s | std::views::split(delim)) {
      arr[i++] = string_view_to<int>(std::string_view{ subrange.begin(), subrange.end() });
    }
    return sent;
  }

  size_t count_free_side(std::istream& input, bool ignore_pockets) {
    static constexpr pos_t sides[] = {
      { 1, 0, 0 },
      { -1, 0, 0 },
      { 0, 1, 0 },
      { 0, -1, 0 },
      { 0, 0, 1 },
      { 0, 0, -1 },
    };
    std::set<pos_t> cubes;
    std::map<pos_t, size_t> free_space;

    for (std::string line; std::getline(input, line);) {
      pos_t cube = parse_pos(line);
      free_space.erase(cube);
      for (const pos_t& side : sides) {
        if (!cubes.contains(cube + side)) {
          auto [it, inserted] = free_space.insert({ cube + side, 1 });
          if (!inserted) {
            ++it->second;
          }
        }
      }
      cubes.insert(cube);
    }
    if (ignore_pockets) {
      return ranges::reduce(free_space | std::views::values);
    }
    pos_t min = { -1, -1, -1 };
    pos_t max = { 25, 25, 25 };
    size_t free = 0;
    auto seen = std::set<pos_t>{ min };
    auto to_see = std::vector<pos_t>{ min };
    while (!to_see.empty()) {
      auto new_to_see = std::vector<pos_t>{};
      for (const pos_t& cur : to_see) {
        for (pos_t s : sides | std::views::transform([&cur](const pos_t& s) { return cur + s; })) {
          if (s.x >= min.x && s.y >= min.y && s.z >= min.z
            && s.x <= max.x && s.y <= max.y && s.z <= max.z
            && !cubes.contains(s) && seen.insert(s).second) {
            auto found = free_space.find(s);
            if (found != free_space.end()) {
              free += found->second;
            }
            new_to_see.push_back(s);
          }
        }
      }
      to_see = std::move(new_to_see);
    }
    return free;
  }

  REGISTER_DAY("d18",
    [](std::istream& in) {
      return std::to_string(count_free_side(in, true));
    },
    [](std::istream& in) {
      return std::to_string(count_free_side(in, false));
    }
  )

  TEST(d18, basic) {
    auto stream = std::istringstream(R"(
1,1,1
2,1,1
)");
    stream.get();
    auto res = count_free_side(stream, true);
    ASSERT_EQ(res, 10);
  }

  TEST(d18, part1) {
    auto stream = std::istringstream(R"(
2,2,2
1,2,2
3,2,2
2,1,2
2,3,2
2,2,1
2,2,3
2,2,4
2,2,6
1,2,5
3,2,5
2,1,5
2,3,5
)");
    stream.get();
    auto res = count_free_side(stream, true);
    ASSERT_EQ(res, 64);
  }

  TEST(d18, part2) {
    auto stream = std::istringstream(R"(
2,2,2
1,2,2
3,2,2
2,1,2
2,3,2
2,2,1
2,2,3
2,2,4
2,2,6
1,2,5
3,2,5
2,1,5
2,3,5
)");
    stream.get();
    auto res = count_free_side(stream, false);
    ASSERT_EQ(res, 58);
  }
}