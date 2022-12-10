#include "days.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <ranges>
#include <algorithm>
#include <numeric>

struct elf {
  uint64_t total_calories = 0;

  auto operator<=>(const elf&) const = default;
};

std::ostream& operator<<(std::ostream& out, const std::vector<elf>& elves) {
  bool first_e = true;
  for (const elf& e : elves) {
    if (!first_e) {
      out << ", ";
    }
    out << e.total_calories;
    first_e = false;
  }
  return out;
}

std::vector<elf> parse_elves(std::istream& input) {
  std::vector<elf> elves;
  bool new_elf = true;
  for (std::string line; std::getline(input, line); ) {
    if (new_elf) {
      elves.emplace_back();
      new_elf = false;
    }
    if (line.empty()) {
      new_elf = true;
    }
    else {
      auto& elf = elves.back().total_calories += std::stoull(line);
    }
  }
  return elves;
}

REGISTER_DAY("d01",
  [](std::istream& input) {
    auto elves = parse_elves(input);
    auto res = std::ranges::max_element(elves, {}, &elf::total_calories);
    return std::to_string(res->total_calories);
  },
  [](std::istream& input) {
    auto elves = parse_elves(input);
    std::ranges::sort(elves, std::greater<>{}, & elf::total_calories);
    auto top_3 = elves | std::views::take(3) | std::views::transform([](const elf& e) { return e.total_calories; });
    return std::to_string(std::reduce(top_3.begin(), top_3.end()));
  }
);

#include <sstream>

TEST(d01, parsing) {
  auto input = std::istringstream(
R"(1000
2000
3000

4000

5000
6000

7000
8000
9000

10000
)"
  );
  auto elves = parse_elves(input);
  auto target = std::vector<elf>{
    {6000},
    {4000},
    {11000},
    {24000},
    {10000}
  };
  EXPECT_EQ(elves, target);
}