#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <regex>
#include <array>

static const auto re = std::regex(R"((\d+)-(\d+),(\d+)-(\d+))");

struct range {
  uint64_t a, b;

  bool contains(const range& o) const {
    return o.a >= a && o.b <= b;
  }

  bool overlaps(const range& o) const {
    return (o.a <= a && a <= o.b)
      || (o.a <= b && b <= o.b);
  }
};

std::array<range, 2> parse_line(const std::string& line) {
  std::smatch match;
  if (!std::regex_match(line, match, re)) {
    throw std::runtime_error("Invalid line");
  }
  return {
    range{ std::stoull(match[1]), std::stoull(match[2]) },
    range{ std::stoull(match[3]), std::stoull(match[4]) }
  };
}

REGISTER_DAY("d04",
  [](std::istream& input) {
    uint64_t matching = 0;
    for (std::string line; std::getline(input, line);) {
      auto [l, r] = parse_line(line);
      if (l.contains(r) || r.contains(l)) {
        matching += 1;
      }
    }
    return std::to_string(matching);
  },
  [](std::istream& input) {
    uint64_t matching = 0;
    for (std::string line; std::getline(input, line);) {
      auto [l, r] = parse_line(line);
      if (l.overlaps(r) || r.overlaps(l)) {
        matching += 1;
      }
    }
    return std::to_string(matching);
  }
)

TEST(d04, overlaps) {
    auto [l, r] = parse_line("20-61,64-77");
    ASSERT_FALSE(l.overlaps(r));
    ASSERT_FALSE(r.overlaps(l));
}