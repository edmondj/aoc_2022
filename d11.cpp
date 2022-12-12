#include "days.hpp"
#include "utils.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <array>

#define CHECK_OVERFLOW 1

using int_worry = std::uint64_t;
using worry_limits = std::numeric_limits<int_worry>;

class worry {
public:
  worry() = default;
  worry(int_worry value)
    : value(value)
  {}

  worry operator+(const worry& r) const {
#ifdef CHECK_OVERFLOW
    if ((r.value > 0 && value > worry_limits::max() - r.value)
    || (r.value < 0 && value < worry_limits::min() - r.value)) {
      throw std::overflow_error("overflow during add");
    }
#endif
    return worry(value + r.value);
  }

  worry operator-(const worry& r) const {
#ifdef CHECK_OVERFLOW
    if ((r.value < 0 && value > worry_limits::max() + r.value)
      || (r.value > 0 && value < worry_limits::min() + r.value)) {
      throw std::overflow_error("overflow during sub");
    }
#endif
    return worry(value - r.value);
  }

  worry operator*(const worry& r) const {
#ifdef CHECK_OVERFLOW
    if ((value == -1 && r.value == worry_limits::min())
      || (r.value == -1 && value == worry_limits::min())
      || (r.value != 0 && value > worry_limits::max() / r.value)
      || (r.value != 0 && value < worry_limits::min() / r.value)) {
      throw std::overflow_error("overflow during mul");
    }
#endif
    return worry(value * r.value);
  }

  worry operator/(const worry& r) const {
    return worry(value / r.value);
  }

  worry& operator/=(const worry& r) {
    value = (*this / r).value;
    return *this;
  }

  worry operator%(const worry& r) const {
    return worry(value % r.value);
  }

  worry& operator%=(const worry& r) {
    value = (*this % r).value;
    return *this;
  }

  auto operator<=>(const worry&) const = default;

  static worry lcm(const worry& l, const worry& r) {
    return worry(std::lcm(l.value, r.value));
  }

private:
  int_worry value;
};


//using worry = int_worry;

struct monkey {
  std::vector<worry> items;
  std::function<worry(worry)> operation;
  worry test;
  size_t target_if_true, target_if_false;
  size_t inspected = 0;

  friend std::istream& operator>>(std::istream& in, monkey& out) {
    static const auto parse_rules = std::vector<std::pair<const char*, std::function<void(const std::string&, monkey&)>>>{
      { "Monkey ", [](const std::string&, monkey&) {} },
      { "  Starting items: ",
        [](const std::string& line, monkey& out) {
          out.items.clear();
          for (const auto& item : std::views::split(line.substr(18), ',')) {
            out.items.push_back(std::stoll(std::string(item.begin(), item.end())));
          }
        }
      },
      { "  Operation: new = ",
        [](const std::string& line, monkey& out) {
          auto splitted = std::views::split(line.substr(19), ' ');
          auto it = splitted.begin();
          auto end = splitted.end();
          std::optional<worry> lsrc, rscr;
          char op;
          if (std::string_view lstr((*it).begin(), (*it).end()); lstr != "old") {
            lsrc = std::stoll(std::string(lstr));
          }
          ++it;
          op = (*it).front();
          ++it;
          if (std::string_view lstr((*it).begin(), (*it).end()); lstr != "old") {
            rscr = std::stoull(std::string(lstr));
          }
          out.operation = [lsrc, rscr, op](worry old) {
            worry l = lsrc.value_or(old);
            worry r = rscr.value_or(old);

            switch (op) {
            case '+': return l + r;
            case '-': return l - r;
            case '*': return l * r;
            case '/': return l / r;
            }
            throw std::runtime_error("unreachable");
          };
        }
      },
      { "  Test: divisible by ",
        [](const std::string& line, monkey& out) {
          out.test = std::stoll(line.substr(21));
        }
      },
      { "    If true: throw to monkey ",
        [](const std::string& line, monkey& out) {
          out.target_if_true = std::stoll(line.substr(29));
        }
      },
      { "    If false: throw to monkey ",
        [](const std::string& line, monkey& out) {
          out.target_if_false = std::stoll(line.substr(29));
        }
      }
    };

    for (const auto& [prefix, action] : parse_rules) {
      std::string line;
      while (in && line.empty())
        std::getline(in, line);
      if (!in || !line.starts_with(prefix)) {
        in.setstate(std::istream::failbit);
        break;
      }
      action(line, out);
    }

    return in;
  }
};

std::vector<monkey> parse_all_monkeys(std::istream& input) {
  std::vector<monkey> sent;
  for (monkey m; input >> m;) {
    sent.push_back(std::move(m));
  }
  return sent;
}

void exec_round(std::vector<monkey>& state, bool worried_inspection, worry lcm) {
  for (monkey& m : state) {
    for (worry item : m.items) {
      ++m.inspected;
      item = m.operation(item);
      if (!worried_inspection) {
        item /= 3;
      }
      item %= lcm;
      size_t target;
      if ((item % m.test) == 0) {
        target = m.target_if_true;
      } else {
        target = m.target_if_false;
      }
      state[target].items.push_back(item);
    }
    m.items.clear();
  }
}

size_t exec_multiple_rounds(std::vector<monkey>& monkeys, size_t count, bool worried_insepction) {
  worry lcm = 1;
  for (const monkey& m : monkeys) {
    lcm = worry::lcm(lcm, m.test);
  }
  for ([[maybe_unused]] auto i : std::views::iota(size_t{ 0 }, count)) {
    exec_round(monkeys, worried_insepction, lcm);
  }
  auto top_2 = std::array<size_t, 2>{0, 0};
  for (const monkey& m : monkeys) {
    auto it_min = std::ranges::min_element(top_2);
    if (m.inspected > *it_min) {
      *it_min = m.inspected;
    }
  }
  return top_2[0] * top_2[1];
}

REGISTER_DAY("d11",
  [](std::istream& input) {
    auto monkeys = parse_all_monkeys(input);
    return std::to_string(exec_multiple_rounds(monkeys, 20, false));
  },
  [](std::istream& input) {
    auto monkeys = parse_all_monkeys(input);
    return std::to_string(exec_multiple_rounds(monkeys, 10000, true));
  }
)

TEST(d11, parsing) {
  auto stream = std::istringstream(R"(
Monkey 0:
  Starting items: 79, 98
  Operation: new = old * 19
  Test: divisible by 23
    If true: throw to monkey 2
    If false: throw to monkey 3
)");
  monkey m;
  ASSERT_TRUE(stream >> m);
  auto target_item = std::vector<worry>{ 79, 98 };
  EXPECT_EQ(m.items, target_item);
  EXPECT_EQ(m.operation(2), 2 * 19);
  EXPECT_EQ(m.test, 23);
  EXPECT_EQ(m.target_if_true, 2);
  EXPECT_EQ(m.target_if_false, 3);
}

TEST(d11, part2) {
  auto stream = std::istringstream(R"(
Monkey 0:
  Starting items: 79, 98
  Operation: new = old * 19
  Test: divisible by 23
    If true: throw to monkey 2
    If false: throw to monkey 3

Monkey 1:
  Starting items: 54, 65, 75, 74
  Operation: new = old + 6
  Test: divisible by 19
    If true: throw to monkey 2
    If false: throw to monkey 0

Monkey 2:
  Starting items: 79, 60, 97
  Operation: new = old * old
  Test: divisible by 13
    If true: throw to monkey 1
    If false: throw to monkey 3

Monkey 3:
  Starting items: 74
  Operation: new = old + 3
  Test: divisible by 17
    If true: throw to monkey 0
    If false: throw to monkey 1
)");
  auto monkeys = parse_all_monkeys(stream);

  // Round 1
  exec_multiple_rounds(monkeys, 1, true);
  ASSERT_EQ(monkeys[0].inspected, 2);
  ASSERT_EQ(monkeys[1].inspected, 4);
  ASSERT_EQ(monkeys[2].inspected, 3);
  ASSERT_EQ(monkeys[3].inspected, 6);

  // Round 20
  exec_multiple_rounds(monkeys, 19, true);
  ASSERT_EQ(monkeys[0].inspected, 99);
  ASSERT_EQ(monkeys[1].inspected, 97);
  ASSERT_EQ(monkeys[2].inspected, 8);
  ASSERT_EQ(monkeys[3].inspected, 103);

  // Round 1000
  exec_multiple_rounds(monkeys, 980, true);
  ASSERT_EQ(monkeys[0].inspected, 5204);
  ASSERT_EQ(monkeys[1].inspected, 4792);
  ASSERT_EQ(monkeys[2].inspected, 199);
  ASSERT_EQ(monkeys[3].inspected, 5192);

  // Round 10000
  exec_multiple_rounds(monkeys, 9000, true);
  ASSERT_EQ(monkeys[0].inspected, 52166);
  ASSERT_EQ(monkeys[1].inspected, 47830);
  ASSERT_EQ(monkeys[2].inspected, 1938);
  ASSERT_EQ(monkeys[3].inspected, 52013);
}