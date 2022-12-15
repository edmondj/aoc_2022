#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <set>

namespace {

struct pos {
  int64_t x = 0;
  int64_t y = 0;

  auto operator<=>(const pos&) const = default;

  friend std::ostream& operator<<(std::ostream& out, const pos& p) {
    return out << '(' << p.x << ", " << p.y << ')';
  }
};

std::set<pos> pull_head(std::istream& input, size_t rope_len = 2) {
  auto rope = std::vector<pos>(rope_len, pos{ 0, 0 });
  auto sent = std::set<pos>{ rope.back() };

  for (std::string line; std::getline(input, line) && line.size() >= 3;) {
    char dir = line[0];
    for (int64_t ammount = std::stoll(line.substr(2)); ammount > 0; --ammount) {
      auto& head = rope.front();
      switch (dir) {
      case 'R': ++head.x; break;
      case 'U': --head.y; break;
      case 'L': --head.x; break;
      case 'D': ++head.y; break;
      }
      for (size_t i = 1; i < rope_len; ++i) {
        auto& prev = rope[i - 1];
        auto& cur = rope[i];
        int64_t x_diff = prev.x - cur.x;
        int64_t y_diff = prev.y - cur.y;
        if (std::abs(x_diff) > 1 || std::abs(y_diff) > 1) {
          cur.x += sign_of(x_diff);
          cur.y += sign_of(y_diff);
        }
      }
      sent.insert(rope.back());
    }
  }

  return sent;
}

REGISTER_DAY("d09",
  [](std::istream& input) {
    return std::to_string(pull_head(input, 2).size());
  },
  [](std::istream& input) {
    return std::to_string(pull_head(input, 10).size());
  }
)

TEST(d09, visit) {
  auto stream = std::istringstream(R"(
R 4
U 4
L 3
D 1
R 4
D 1
L 5
R 2
)");
  stream.get();
  auto visited = pull_head(stream);
  auto target = std::set<pos>{
    {0, 0}, {1, 0}, {2, 0}, {3, 0},
    {4, -1},
    {1, -2}, {2, -2}, {3, -2}, {4, -2},
    {3, -3}, {4, -3},
    {2, -4}, {3, -4}
  };

  EXPECT_EQ(visited.size(), 13);
  EXPECT_EQ(visited, target);
}

TEST(d09, angles) {
  {
    auto stream = std::istringstream(R"(
R 1
U 2
)");
    stream.get();
    auto visited = pull_head(stream);
    auto target = std::set<pos>{ {0, 0}, {1, -1} };
    EXPECT_EQ(visited, target);
  }
  {
    auto stream = std::istringstream(R"(
R 1
D 2
)");
    stream.get();
    auto visited = pull_head(stream);
    auto target = std::set<pos>{ {0, 0}, {1, 1} };
    EXPECT_EQ(visited, target);
  }
  {
    auto stream = std::istringstream(R"(
U 1
L 2
)");
    stream.get();
    auto visited = pull_head(stream);
    auto target = std::set<pos>{ {0, 0}, {-1, -1} };
    EXPECT_EQ(visited, target);
  }
  {
    auto stream = std::istringstream(R"(
U 1
R 2
)");
    stream.get();
    auto visited = pull_head(stream);
    auto target = std::set<pos>{ {0, 0}, {1, -1} };
    EXPECT_EQ(visited, target);
  }
  {
    auto stream = std::istringstream(R"(
L 1
U 2
)");
    stream.get();
    auto visited = pull_head(stream);
    auto target = std::set<pos>{ {0, 0}, {-1, -1} };
    EXPECT_EQ(visited, target);
  }
  {
    auto stream = std::istringstream(R"(
L 1
D 2
)");
    stream.get();
    auto visited = pull_head(stream);
    auto target = std::set<pos>{ {0, 0}, {-1, 1} };
    EXPECT_EQ(visited, target);
  }
  {
    auto stream = std::istringstream(R"(
D 1
R 2
)");
    stream.get();
    auto visited = pull_head(stream);
    auto target = std::set<pos>{ {0, 0}, {1, 1} };
    EXPECT_EQ(visited, target);
  }
  {
    auto stream = std::istringstream(R"(
D 1
L 2
)");
    stream.get();
    auto visited = pull_head(stream);
    auto target = std::set<pos>{ {0, 0}, {-1, 1} };
    EXPECT_EQ(visited, target);
  }
}

}