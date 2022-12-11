#include "days.hpp"
#include "utils.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <array>

struct monkey {
  std::vector<int64_t> items;
  std::function<int64_t(int64_t)> operation;
  int64_t test;
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
          std::optional<int64_t> lsrc, rscr;
          char op;
          if (std::string_view lstr((*it).begin(), (*it).end()); lstr != "old") {
            lsrc = std::stoll(std::string(lstr));
          }
          ++it;
          op = (*it).front();
          ++it;
          if (std::string_view lstr((*it).begin(), (*it).end()); lstr != "old") {
            rscr = std::stoll(std::string(lstr));
          }
          out.operation = [lsrc, rscr, op](int64_t old) {
            int64_t l = lsrc.value_or(old);
            int64_t r = rscr.value_or(old);

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

void exec_round(std::vector<monkey>& state, bool worried_inspection, int64_t lcm) {
  for (monkey& m : state) {
    for (int64_t item : m.items) {
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
  int64_t lcm = 1;
  for (const monkey& m : monkeys) {
    lcm = std::lcm(lcm, m.test);
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
  auto target_item = std::vector<int64_t>{ 79, 98 };
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