#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>

namespace d20 {

  using value_t = int64_t;
  using data_t = std::vector<value_t>;

  data_t parse_data(std::istream& in) {
    data_t sent;
    value_t i = 0;
    for (std::string line; std::getline(in, line); ++i) {
      sent.push_back(string_view_to<value_t>(line));
    }
    return sent;
  }

  bool is_all_unique(const data_t& d) {
    for (auto it = d.begin(); it != d.end(); ++it) {
      for (auto rit = std::next(it); rit != d.end(); ++rit) {
        if (*it == *rit) {
          return false;
        }
      }
    }
    return true;
  }

  template<typename TContainer>
  void move(TContainer& data, int64_t found, value_t value) {
    if (value == 0) {
      return;
    }

    const auto size = static_cast<int64_t>(data.size()) - 1;
    auto target = (found + value) % size;
    if (target <= 0) {
      target += size;
    }
    value_t dir = (found < target ? 1 : -1);
    for (; found != target; found += dir) {
      std::swap(data[found], data[found + dir]);
    }
  }

  void mix(data_t& data, size_t n = 1) {
    struct entry_t {
      size_t index;
      value_t value;
    };
    std::vector<entry_t> tmp;
    tmp.reserve(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
      tmp.push_back({ .index = i, .value = data[i] });
    }
    for (; n > 0; --n) {
      for (size_t i = 0; i < data.size(); ++i) {
        auto found = std::ranges::find(tmp, i, &entry_t::index);
        move(tmp, found - std::ranges::begin(tmp), found->value);
      }
    }
    std::ranges::copy(tmp | std::views::transform(&entry_t::value), data.begin());
  }

  value_t find_key(const data_t& data) {
    auto zero_idx = std::ranges::find(data, 0) - std::ranges::begin(data);
    return data[(1000 + zero_idx) % data.size()] + data[(2000 + zero_idx) % data.size()] + data[(3000 + zero_idx) % data.size()];
  }

  REGISTER_DAY("d20",
    [](std::istream& input) {
      data_t data = parse_data(input);
      mix(data);
      return std::to_string(find_key(data));
    },
    [](std::istream& input) {
      data_t data = parse_data(input);
      for (value_t& v : data) {
        v *= 811589153;
      }
      mix(data, 10);
      return std::to_string(find_key(data));
    }
  )

  TEST(d20, part1) {
    {
      auto data = data_t{ 1, 2, -3, 3, -2, 0, 4 };
      {
        move(data, 0, 1);
        auto target = data_t{ 2, 1, -3, 3, -2, 0, 4 };
        ASSERT_EQ(data, target);
      }
      {
        move(data, 0, 2);
        auto target = data_t{ 1, -3, 2, 3, -2, 0, 4 };
        ASSERT_EQ(data, target);
      }
      {
        move(data, 1, -3);
        auto target = data_t{ 1, 2, 3, -2, -3, 0, 4 };
        ASSERT_EQ(data, target);
      }
      {
        move(data, 2, 3);
        auto target = data_t{ 1, 2, -2, -3, 0, 3, 4 };
        ASSERT_EQ(data, target);
      }
      {
        move(data, 2, -2);
        auto target = data_t{ 1, 2, -3, 0, 3, 4, -2 };
        ASSERT_EQ(data, target);
      }
      {
        move(data, 3, 0);
        auto target = data_t{ 1, 2, -3, 0, 3, 4, -2 };
        ASSERT_EQ(data, target);
      }
      {
        move(data, 5, 4);
        auto target = data_t{ 1, 2, -3, 4, 0, 3, -2 };
        ASSERT_EQ(data, target);
      }
    }
    {
      auto data = data_t{ 1, 2, -3, 3, -2, 0, 4 };
      mix(data);
      auto target = data_t{ 1, 2, -3, 4, 0, 3, -2 };
      ASSERT_EQ(data, target);
    }
  }

  TEST(d20, part2) {
    {
      auto data = data_t{ 811589153, 1623178306, -2434767459, 2434767459, -1623178306, 0, 3246356612 };
      mix(data, 1);
      auto target = data_t{ 0, -2434767459, 3246356612, -1623178306, 2434767459, 1623178306, 811589153 };
      ASSERT_EQ(data, target);
    }
    {
      auto data = data_t{ 811589153, 1623178306, -2434767459, 2434767459, -1623178306, 0, 3246356612 };
      mix(data, 2);
      auto target = data_t{ 0, 2434767459, 1623178306, 3246356612, -2434767459, -1623178306, 811589153 };
      ASSERT_EQ(data, target);
    }
    {
      auto data = data_t{ 811589153, 1623178306, -2434767459, 2434767459, -1623178306, 0, 3246356612 };
      mix(data, 3);
      auto target = data_t{ 0, 811589153, 2434767459, 3246356612, 1623178306, -1623178306, -2434767459 };
      ASSERT_EQ(data, target);
    }
    {
      auto data = data_t{ 811589153, 1623178306, -2434767459, 2434767459, -1623178306, 0, 3246356612 };
      mix(data, 4);
      auto target = data_t{ 0, 1623178306, -2434767459, 811589153, 2434767459, 3246356612, -1623178306 };
      ASSERT_EQ(data, target);
    }
    {
      auto data = data_t{ 811589153, 1623178306, -2434767459, 2434767459, -1623178306, 0, 3246356612 };
      mix(data, 5);
      auto target = data_t{ 0, 811589153, -1623178306, 1623178306, -2434767459, 3246356612, 2434767459 };
      ASSERT_EQ(data, target);
    }
    {
      auto data = data_t{ 811589153, 1623178306, -2434767459, 2434767459, -1623178306, 0, 3246356612 };
      mix(data, 6);
      auto target = data_t{ 0, 811589153, -1623178306, 3246356612, -2434767459, 1623178306, 2434767459 };
      ASSERT_EQ(data, target);
    }
    {
      auto data = data_t{ 811589153, 1623178306, -2434767459, 2434767459, -1623178306, 0, 3246356612 };
      mix(data, 7);
      auto target = data_t{ 0, -2434767459, 2434767459, 1623178306, -1623178306, 811589153, 3246356612 };
      ASSERT_EQ(data, target);
    }
    {
      auto data = data_t{ 811589153, 1623178306, -2434767459, 2434767459, -1623178306, 0, 3246356612 };
      mix(data, 8);
      auto target = data_t{ 0, 1623178306, 3246356612, 811589153, -2434767459, 2434767459, -1623178306 };
      ASSERT_EQ(data, target);
    }
    {
      auto data = data_t{ 811589153, 1623178306, -2434767459, 2434767459, -1623178306, 0, 3246356612 };
      mix(data, 9);
      auto target = data_t{ 0, 811589153, 1623178306, -2434767459, 3246356612, 2434767459, -1623178306 };
      ASSERT_EQ(data, target);
    }
    {
      auto data = data_t{ 811589153, 1623178306, -2434767459, 2434767459, -1623178306, 0, 3246356612 };
      mix(data, 10);
      auto target = data_t{ 0, -2434767459, 1623178306, 3246356612, -1623178306, 2434767459, 811589153 };
      ASSERT_EQ(data, target);
    }
  }
}