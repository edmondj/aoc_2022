#include "days.hpp"
#include <gtest/gtest.h>
#include <algorithm>

namespace d25 {
  using value_t = int64_t;

  value_t snafu_digit(char c) {
    switch (c) {
    case '2': return 2;
    case '1': return 1;
    case '0': return 0;
    case '-': return -1;
    case '=': return -2;
    default:
      std::unreachable();
    }
  }

  value_t snafu_value(std::string_view snafu) {
    value_t sent = 0;
    for (char c : snafu) {
      sent *= 5;
      sent += snafu_digit(c);
    }
    return sent;
  }

  std::string value_snafu(value_t val) {
    std::string sent;
    while (val > 0) {
      switch (val % 5) {
      case 0:
        sent.push_back('0');
        break;
      case 1:
        sent.push_back('1');
        break;
      case 2:
        sent.push_back('2');
        break;
      case 3:
        sent.push_back('=');
        val += 2;
        break;
      case 4:
        sent.push_back('-');
        val += 1;
        break;
      default:
        std::unreachable();
      }
      val /= 5;
    }
    if (sent.empty()) { sent = "0"; }
    else {
      std::ranges::reverse(sent);
    }
    return sent;
  }

  REGISTER_DAY("d25",
    [](std::istream& in) {
      value_t res = 0;
      for (std::string line; std::getline(in, line);) {
        res += snafu_value(line);
      }
      return value_snafu(res);
    }
  )

  TEST(d25, snafu_value) {
    std::pair<value_t, std::string_view> tests[]{
      { 1, "1" },
      { 2, "2" },
      { 3, "1=" },
      { 4, "1-" },
      { 5, "10" },
      { 6, "11" },
      { 7, "12" },
      { 8, "2=" },
      { 9, "2-" },
      { 10, "20" },
      { 15, "1=0" },
      { 20, "1-0" },
      { 2022, "1=11-2" },
      { 12345, "1-0---0" },
      { 314159265, "1121-1110-1=0" },
    };
    for (const auto& [value, snafu] : tests) {
      EXPECT_EQ(value, snafu_value(snafu));
      EXPECT_EQ(value_snafu(value), snafu);
    }
    EXPECT_EQ(snafu_value("2=-01"), 976);
  }
}