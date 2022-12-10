#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <ranges>
#include <optional>

class signal_generator {
public:
  using difference_type = ptrdiff_t;
  struct value_type {
    int64_t cycle;
    int64_t x;
  };
  struct sentinel_t {};

  signal_generator(std::istream& input)
    : input(&input) {
    get_next_instruction();
  }

  const value_type& operator*() const {
    return state;
  }

  signal_generator& operator++() {
    ++state.cycle;
    if (--instruction_length == 0) {
      get_next_instruction();
    }
    return *this;
  }

  int operator++(int) {
    throw std::runtime_error("unreachable");
  }

  bool operator==(sentinel_t) const { return instruction_length < 0; }

private:

  void get_next_instruction() {
    state.x += add;
    std::string line;
    if (std::getline(*input, line)) {
      if (line.starts_with("addx ")) {
        add = std::stoull(line.substr(5));
        instruction_length = 2;
      } else {
        add = 0;
        instruction_length = 1;
      }
    }
  }

  std::istream* input;
  value_type state = { 1, 1 };
  int64_t add = 0;
  int64_t instruction_length = -1;
};

static_assert(std::input_iterator<signal_generator>);

auto generate_signal(std::istream& input) {
  return std::ranges::subrange(signal_generator(input), signal_generator::sentinel_t{});
}

auto interpreted_signal(std::istream& input) {
  return generate_signal(input) | std::views::drop(2)
    | std::views::filter([](const signal_generator::value_type& p) {
        return p.cycle >= 20 && (p.cycle - 20) % 40 == 0;
      })
    | std::views::transform([](const signal_generator::value_type& p) {
        return p.cycle * p.x;
      })
    ;
}

REGISTER_DAY("d10",
  [](std::istream& input) {
    return std::to_string(ranges::reduce(interpreted_signal(input)));
  },
  [](std::istream& input) {
    std::ostringstream res;
    for (const auto& [cycle, x] : generate_signal(input) | std::views::take(40 * 6)) {
      if (cycle % 40 == 1) {
        res << '\n';
      }
      if (std::abs((cycle - 1) % 40 - x) <= 1) {
        res << '#';
      } else {
        res << '.';
      }
    }
    return res.str();
  }
)

TEST(d10, basic) {
  auto input = std::istringstream(R"(
noop
addx 3
addx -5
)");
  input.get();
  auto res = std::vector<int64_t>{};
  for (const auto& s : generate_signal(input)) {
    res.push_back(s.x);
  }
  auto target = std::vector<int64_t>{ 1, 1, 1, 4, 4, -1 };
  EXPECT_EQ(res, target);
}

TEST(d10, part1) {
  auto input = std::istringstream(R"(
addx 15
addx -11
addx 6
addx -3
addx 5
addx -1
addx -8
addx 13
addx 4
noop
addx -1
addx 5
addx -1
addx 5
addx -1
addx 5
addx -1
addx 5
addx -1
addx -35
addx 1
addx 24
addx -19
addx 1
addx 16
addx -11
noop
noop
addx 21
addx -15
noop
noop
addx -3
addx 9
addx 1
addx -3
addx 8
addx 1
addx 5
noop
noop
noop
noop
noop
addx -36
noop
addx 1
addx 7
noop
noop
noop
addx 2
addx 6
noop
noop
noop
noop
noop
addx 1
noop
noop
addx 7
addx 1
noop
addx -13
addx 13
addx 7
noop
addx 1
addx -33
noop
noop
noop
addx 2
noop
noop
noop
addx 8
noop
addx -1
addx 2
addx 1
noop
addx 17
addx -9
addx 1
addx 1
addx -3
addx 11
noop
noop
addx 1
noop
addx 1
noop
noop
addx -13
addx -19
addx 1
addx 3
addx 26
addx -30
addx 12
addx -1
addx 3
addx 1
noop
noop
noop
addx -9
addx 18
addx 1
addx 2
noop
noop
addx 9
noop
noop
noop
addx -1
addx 2
addx -37
addx 1
addx 3
noop
addx 15
addx -21
addx 22
addx -6
addx 1
noop
addx 2
addx 1
noop
addx -10
noop
noop
addx 20
addx 1
addx 2
addx 2
addx -6
addx -11
noop
noop
noop
)");
  input.get();
  auto res = std::vector<int64_t>{};
  for (int64_t x : interpreted_signal(input)) {
    res.push_back(x);
  }
  auto target = std::vector<int64_t>{ 420, 1140, 1800, 2940, 2880, 3960 };
  EXPECT_EQ(res, target);
}