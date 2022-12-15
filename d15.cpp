#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <regex>

namespace {

struct pos {
  int64_t x, y;

  auto operator<=>(const pos&) const = default;

  pos operator-(const pos& p) const {
    return pos{ x - p.x, y - p.y };
  }

  pos operator+(const pos& r) const {
    pos sent = *this;
    sent += r;
    return sent;
  }

  pos& operator+=(const pos& r) {
    x += r.x;
    y += r.y;
    return *this;
  }

  int64_t length() const {
    return std::abs(x) + std::abs(y);
  }

  friend std::ostream& operator<<(std::ostream& out, const pos& p) {
    return out << '(' << p.x << ", " << p.y << ')';
  }

  int64_t tuning_frequency() const {
    return x * 4000000 + y;
  }
};

struct finding {
  pos sensor;
  pos beacon;
  int64_t length;
};

finding parse_finding(const std::string& s) {
  static const auto re = std::regex(R"(Sensor at x=(-?\d+), y=(-?\d+): closest beacon is at x=(-?\d+), y=(-?\d+))");
  std::smatch match;
  if (!std::regex_match(s, match, re)) {
    throw std::runtime_error("invalid format");
  }
  auto sensor = pos{
    .x = string_view_to<int64_t>(match[1].str()),
    .y = string_view_to<int64_t>(match[2].str()),
  };
  auto beacon = pos{
    .x = string_view_to<int64_t>(match[3].str()),
    .y = string_view_to<int64_t>(match[4].str()),
  };
  return finding{
    .sensor = sensor,
    .beacon = beacon,
    .length = (beacon - sensor).length()
  };
}

std::vector<finding> parse_findings(std::istream& input) {
  std::vector<finding> findings;
  for (std::string line; std::getline(input, line) && !line.empty();) {
    findings.push_back(parse_finding(line));
  }
  return findings;
}

size_t part2(const std::vector<finding>& findings, int64_t range) {
  const auto test = [&findings, range](const pos& p) {
    return p.x >= 0 && p.x <= range
      && p.y >= 0 && p.y <= range
      && std::ranges::all_of(findings, [p](const finding& f) {
          return (p - f.sensor).length() > f.length;
         })
      ;
  };
  for (const finding& f : findings) {
    auto length = f.length + 1;
    // top right
    for (auto cur = pos{ 0, -length }; cur != pos{ length, 0 }; cur += pos{ 1, 1 }) {
      auto candidate = f.sensor + cur;
      if (test(candidate)) {
        return candidate.tuning_frequency();
      }
    }
    // bottom right
    for (auto cur = pos{ length, 0 }; cur != pos{ 0, length }; cur += pos{ -1, 1 }) {
      auto candidate = f.sensor + cur;
      if (test(candidate)) {
        return candidate.tuning_frequency();
      }
    }
    // bottom left
    for (auto cur = pos{ 0, length }; cur != pos{ -length, 0 }; cur += pos{ -1, -1 }) {
      auto candidate = f.sensor + cur;
      if (test(candidate)) {
        return candidate.tuning_frequency();
      }
    }
    // top left
    for (auto cur = pos{ -length, 0 }; cur != pos{ 0, -length }; cur += pos{ 1, -1 }) {
      auto candidate = f.sensor + cur;
      if (test(candidate)) {
        return candidate.tuning_frequency();
      }
    }
  }
  throw std::runtime_error{ "not found" };
}

size_t part1(const std::vector<finding>& findings, int64_t y) {
  std::set<int64_t> beacon_free;

  for (const finding& f : findings) {
    int64_t length = f.length - std::abs(y - f.sensor.y);
    if (length >= 0) {
      int64_t lower = f.sensor.x - length;
      int64_t higher = f.sensor.x + length;
      if (f.beacon.y == y) {
        if (lower == f.beacon.x) {
          ++lower;
        }
        if (higher == f.beacon.x) {
          --higher;
        }
      }
      for (int64_t x = lower; x <= higher; ++x) {
        beacon_free.insert(x);
      }
    }
  }

  return beacon_free.size();
}

REGISTER_DAY("d15",
  [](std::istream& input) {
    return std::to_string(part1(parse_findings(input), 2000000));
  },
  [](std::istream& input) {
    return std::to_string(part2(parse_findings(input), 4000000));
  }
)

TEST(d15, parsing) {
  auto f = parse_finding("Sensor at x=2, y=18: closest beacon is at x=-2, y=15");
  EXPECT_EQ(f.sensor.x, 2);
  EXPECT_EQ(f.sensor.y, 18);
  EXPECT_EQ(f.length, 7);
}

TEST(d15, length) {
  auto length = (pos{ 8, 7 } - pos{ 2, 10 }).length();
  EXPECT_EQ(length, 9);
}

TEST(d15, equality) {
  auto l = pos{ 2, 10 };
  auto r = l;
  EXPECT_EQ(l, r);
}

TEST(d15, part1) {
  {
    auto stream = std::istringstream(R"(
Sensor at x=8, y=7: closest beacon is at x=2, y=10
)");
    stream.get();

    auto res = part1(parse_findings(stream), 10);
    ASSERT_EQ(res, 12);
  }
  {
    auto stream = std::istringstream(R"(
Sensor at x=2, y=18: closest beacon is at x=-2, y=15
Sensor at x=9, y=16: closest beacon is at x=10, y=16
Sensor at x=13, y=2: closest beacon is at x=15, y=3
Sensor at x=12, y=14: closest beacon is at x=10, y=16
Sensor at x=10, y=20: closest beacon is at x=10, y=16
Sensor at x=14, y=17: closest beacon is at x=10, y=16
Sensor at x=8, y=7: closest beacon is at x=2, y=10
Sensor at x=2, y=0: closest beacon is at x=2, y=10
Sensor at x=0, y=11: closest beacon is at x=2, y=10
Sensor at x=20, y=14: closest beacon is at x=25, y=17
Sensor at x=17, y=20: closest beacon is at x=21, y=22
Sensor at x=16, y=7: closest beacon is at x=15, y=3
Sensor at x=14, y=3: closest beacon is at x=15, y=3
Sensor at x=20, y=1: closest beacon is at x=15, y=3
)");
    stream.get();

    auto res = part1(parse_findings(stream), 10);
    EXPECT_EQ(res, 26);
  }
}

TEST(d15, part2) {
  auto stream = std::istringstream(R"(
Sensor at x=2, y=18: closest beacon is at x=-2, y=15
Sensor at x=9, y=16: closest beacon is at x=10, y=16
Sensor at x=13, y=2: closest beacon is at x=15, y=3
Sensor at x=12, y=14: closest beacon is at x=10, y=16
Sensor at x=10, y=20: closest beacon is at x=10, y=16
Sensor at x=14, y=17: closest beacon is at x=10, y=16
Sensor at x=8, y=7: closest beacon is at x=2, y=10
Sensor at x=2, y=0: closest beacon is at x=2, y=10
Sensor at x=0, y=11: closest beacon is at x=2, y=10
Sensor at x=20, y=14: closest beacon is at x=25, y=17
Sensor at x=17, y=20: closest beacon is at x=21, y=22
Sensor at x=16, y=7: closest beacon is at x=15, y=3
Sensor at x=14, y=3: closest beacon is at x=15, y=3
Sensor at x=20, y=1: closest beacon is at x=15, y=3
)");
  stream.get();

  auto res = part2(parse_findings(stream), 20);
  EXPECT_EQ(res, 56000011);
}

}