#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <regex>
#include <vector>
#include <stack>
#include <algorithm>

struct state {
  std::vector<std::vector<char>> stacks;

  auto operator<=>(const state&) const = default;
};

state parse_state(std::istream& input) {
  state sent;
  for (std::string line; std::getline(input, line) && line.size() >= 2;) {
    if (line[1] == '1') {
      continue;
    }
    for (size_t stack_idx = 0, char_idx = 1; char_idx < line.size(); ++stack_idx, char_idx += 4) {
      if (sent.stacks.size() <= stack_idx) {
        sent.stacks.resize(stack_idx + 1);
      }
      if (char crate = line[char_idx]; crate != ' ') {
        sent.stacks[stack_idx].push_back(line[char_idx]);
      }
    }
  }
  // Stacks are reverse
  for (auto& stack : sent.stacks) {
    std::ranges::reverse(stack);
  }
  return sent;
}

struct move {
  size_t ammount, from, to;
};

move parse_move(const std::string& line) {
  static const auto re = std::regex(R"(move (\d+) from (\d+) to (\d+))");
  std::smatch matches;
  if (!std::regex_match(line, matches, re)) {
    throw std::runtime_error("Invalid instruction");
  }
  return move{
    .ammount = std::stoull(matches[1]),
    .from = std::stoull(matches[2]),
    .to = std::stoull(matches[3]),
  };
}

template<typename Crane>
std::string solve(std::istream& input, Crane&& crane) {
  state s = parse_state(input);
  for (std::string line; std::getline(input, line);) {
    move m = parse_move(line);
    auto& from = s.stacks[m.from - 1];
    auto& to = s.stacks[m.to - 1];
    crane(from, to, m.ammount);
  }
  std::string res;
  for (const auto& stack : s.stacks) {
    res += stack.back();
  }
  return res;
}

void old_crane(std::vector<char>& from, std::vector<char>& to, size_t ammount) {
  std::ranges::copy(from | std::views::reverse | std::views::take(ammount), std::back_inserter(to));
  from.resize(from.size() - ammount);
}

void new_crane(std::vector<char>& from, std::vector<char>& to, size_t ammount) {
  std::ranges::copy(from | std::views::drop(from.size() - ammount), std::back_inserter(to));
  from.resize(from.size() - ammount);
}

REGISTER_DAY("d05",
  [](std::istream& input) {
    return solve(input, &old_crane);
  },
  [](std::istream& input) {
    return solve(input, &new_crane);
  }
)

TEST(d05, parse) {
  auto stream = std::istringstream(
    R"(
    [D]
[N] [C]
[Z] [M] [P]
 1   2   3

)");
  stream.get(); // skip \n put there for readability
  auto target = state{{
      {'Z', 'N'},
      {'M', 'C', 'D'},
      {'P'}
  }};
  ASSERT_EQ(parse_state(stream), target);
}

TEST(d05, part1) {
  auto stream = std::istringstream(
    R"(
    [D]
[N] [C]
[Z] [M] [P]
 1   2   3

move 1 from 2 to 1
move 3 from 1 to 3
move 2 from 2 to 1
move 1 from 1 to 2
)");
  stream.get(); // skip \n put there for readability
  ASSERT_EQ(solve(stream, &old_crane), "CMZ");
}