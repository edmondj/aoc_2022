#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>

bool is_valid(std::string_view chunk) {
  for (size_t i = 0; i < chunk.size(); ++i) {
    if (ranges::contains(chunk.substr(i + 1), chunk[i])) {
      return false;
    }
  }
  return true;
}

size_t find_marker(std::istream& input, size_t marker_size) {
  std::string line;
  std::getline(input, line);
  for (size_t i = marker_size; i < line.size(); ++i) {
    auto sub = std::string_view(line).substr(i - marker_size, marker_size);
    if (is_valid(sub)) {
      return i;
    }
  }
  throw std::runtime_error("Not found");
}

REGISTER_DAY("d06",
  [](std::istream& input) {
    return std::to_string(find_marker(input, 4));
  },
  [](std::istream& input) {
    return std::to_string(find_marker(input, 14));
  }
)

TEST(d06, valid) {
  EXPECT_TRUE(is_valid("abcd"));
  EXPECT_FALSE(is_valid("abca"));
}

TEST(d06, part1) {
  {
    auto stream = std::istringstream("bvwbjplbgvbhsrlpgdmjqwftvncz");
    EXPECT_EQ(find_marker(stream, 4), 5);
  }
  {
    auto stream = std::istringstream("nppdvjthqldpwncqszvftbrmjlhg");
    EXPECT_EQ(find_marker(stream, 4), 6);
  }
  {
    auto stream = std::istringstream("nznrnfrfntjfmvfwmzdfjlvtqnbhcprsg");
    EXPECT_EQ(find_marker(stream, 4), 10);
  }
  {
    auto stream = std::istringstream("zcfzfwzzqfrljwzlrfnpqdbhtmscgvjw");
    EXPECT_EQ(find_marker(stream, 4), 11);
  }
}