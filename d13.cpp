#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <variant>
#include <charconv>
#include <algorithm>

using number = int;
struct value : std::variant<number, std::vector<value>> {
  using std::variant<number, std::vector<value>>::variant;
};
using list = std::vector<value>;

std::ostream& operator<<(std::ostream& out, const list& l);
std::ostream& operator<<(std::ostream& out, const value& v);

std::ostream& operator<<(std::ostream& out, const list& l) {
  out << '[';
  bool first = true;
  for (const value& v : l) {
    if (!first) {
      out << ", ";
    } else {
      first = false;
    }
    out << v;
  }
  return out << ']';
}

std::ostream& operator<<(std::ostream& out, const value& v) {
  return std::visit([&out](const auto& nested) -> std::ostream& { return out << nested; }, v);
}

std::istream& operator>>(std::istream& in, list& out);
std::istream& operator>>(std::istream& in, value& out);

std::istream& operator>>(std::istream& in, list& out) {
  if (in.get() != '[') {
    in.setstate(std::istream::badbit);
    return in;
  }
  out.clear();
  while (in && in.peek() != ']') {
    in >> out.emplace_back();
    if (in.peek() == ',') {
      in.get();
    }
  }
  if (in && in.get() != ']') {
    in.setstate(std::istream::badbit);
    return in;
  }
  return in;
}

std::istream& operator>>(std::istream& in, value& out) {
  if (in.peek() == '[') {
    return in >> out.emplace<list>();
  }
  return in >> out.emplace<number>();
}

std::strong_ordering operator<=>(const number& l, const list& r);
std::strong_ordering operator<=>(const list& l, const number& r);
std::strong_ordering operator<=>(const list& l, const list& r);

std::strong_ordering operator<=>(const number& l, const list& r) {
  return list{ l } <=> r;
}

std::strong_ordering operator<=>(const list& l, const number& r) {
  return l <=> list{ r };
}

std::strong_ordering operator<=>(const list& l, const list& r) {
  auto lit = l.begin();

  auto rit = r.begin();
  for (; lit != l.end() && rit != r.end(); ++lit, ++rit) {
    auto sub_order = std::visit([](const auto& l, const auto& r) { return l <=> r; }, *lit, *rit);
    if (sub_order != std::strong_ordering::equal) {
      return sub_order;
    }
  }
  if (lit == l.end() && rit == r.end()) {
    return std::strong_ordering::equal;
  }
  if (lit == l.end()) {
    return std::strong_ordering::less;
  }
  return std::strong_ordering::greater;
}

REGISTER_DAY("d13",
  [](std::istream& input) {
    size_t total = 0;
    for (size_t i = 1; input; ++i) {
      list l, r;
      input >> l;
      input.get();
      input >> r;
      input.get();
      if (!input) {
        throw std::runtime_error("failed to parse");
      }
      if (l <= r) {
        total += i;
      }
      input.get();
    }
    return std::to_string(total);
  },
  [](std::istream& input) {
    std::vector<list> data;
    const auto key1 = list{ {list{2}} };
    const auto key2 = list{ {list{6}} };
    while (input) {
      list l;
      if (input >> l) {
        data.push_back(std::move(l));
      }
      input.get(); // skips end of line
      if (input.peek() == '\n') {
        input.get(); // skips empty line
      }
    }
    data.push_back(key1);
    data.push_back(key2);
    std::sort(data.begin(), data.end());
    auto found1 = std::ranges::find(data, key1) - data.begin() + 1;
    auto found2 = std::ranges::find(data, key2) - data.begin() + 1;
    return std::to_string(found1 * found2);
  }
)

TEST(d13, parse) {
  {
    auto stream = std::istringstream("[1,1,3,1,1]");
    list parsed;
    ASSERT_TRUE(stream >> parsed);
    auto target = list{ 1, 1, 3, 1, 1 };
    EXPECT_EQ(parsed, target);
  }
  {
    auto stream = std::istringstream("[[1],[2,3,4]]");
    list parsed;
    ASSERT_TRUE(stream >> parsed);
    auto target = list{ list{1}, list{2, 3, 4} };
    EXPECT_EQ(parsed, target);
  }
  {
    auto stream = std::istringstream("[[8,7,6]]");
    list parsed;
    ASSERT_TRUE(stream >> parsed);
    auto target = list{ { list{ 8, 7, 6 } } };
    EXPECT_EQ(parsed, target);
  }
}

TEST(d13, ordering) {
  {
    auto l = list{ 1, 1, 3, 1, 1 };
    auto r = list{ 1, 1, 5, 1, 1 };
    EXPECT_TRUE(l < r);
  }
  {
    auto l = list{ list{1}, list{2, 3, 4} };
    auto r = list{ list{1}, 4 };
    EXPECT_TRUE(l < r);
  }
  {
    auto l = list{ 9 };
    auto r = list{ { list{8, 7, 6} } };
    EXPECT_FALSE(l < r);
  }
  {
    auto l = list{ list{ 4, 4 }, 4, 4 };
    auto r = list{ list{ 4, 4 }, 4, 4, 4 };
    EXPECT_TRUE(l < r);
  }
  {
    auto l = list{ 7, 7, 7, 7 };
    auto r = list{ 7, 7, 7 };
    EXPECT_FALSE(l < r);
  }
}