#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <variant>
#include <algorithm>
#include <map>

namespace d21 {

  using value_t = int64_t;
  
  struct operation_t {
    std::string left, right;
    char operand;

    auto operator<=>(const operation_t&) const = default;

    friend std::ostream& operator<<(std::ostream& out, const operation_t& op) {
      return out << op.left << ' ' << op.operand << ' ' << op.right;
    }
  };

  using monkey_t = std::variant<value_t, operation_t>;

  std::ostream& operator<<(std::ostream& out, const monkey_t& m) {
    return std::visit([&out](const auto& v) -> decltype(auto) { return out << v; }, m);
  }

  using monkey_map_t = std::map<std::string, monkey_t>;

  monkey_map_t::value_type parse_monkey(std::string_view line) {
    auto column = line.find(':');
    if (column == std::string_view::npos) {
      throw std::runtime_error("missing column");
    }
    auto name = line.substr(0, column);
    auto monkey = line.substr(column + 2);
    if (std::ranges::all_of(monkey, [](char c) { return std::isdigit(c); })) {
      return std::make_pair(std::string(name.begin(), name.end()), string_view_to<value_t>(monkey));
    }
    auto space = monkey.find(' ');
    auto left = monkey.substr(0, space);
    auto op = monkey[space + 1];
    auto right = monkey.substr(space + 3);
    return std::make_pair(
      std::string(name.begin(), name.end()),
      operation_t{
        .left = std::string(left.begin(), left.end()),
        .right = std::string(right.begin(), right.end()),
        .operand = op
      }
    );
  }

  monkey_map_t parse_monkeys(std::istream& in) {
    monkey_map_t sent;
    for (std::string line; std::getline(in, line);) {
      sent.insert(parse_monkey(line));
    }
    return sent;
  }

  value_t solve(const monkey_map_t& m, const std::string& key) {
    return std::visit([&m](const auto& v) {
        if constexpr (std::is_same_v<decltype(v), const value_t&>) {
          return v;
        } else {
          auto l = solve(m, v.left);
          auto r = solve(m, v.right);
          switch (v.operand) {
            case '+': return l + r;
            case '-': return l - r;
            case '*': return l * r;
            case '/': return l / r;
            default: throw std::runtime_error("unknown operand");
          }
        }
      }, m.at(key));
  }

  struct unknown_t {
    auto operator<=>(const unknown_t&) const = default;
  };

  using with_unknown_t = std::variant<value_t, operation_t, unknown_t>;
  using monkey_map_with_unknown_t = std::map<std::string, with_unknown_t>;

  static constexpr const char* HUMN = "humn";

  monkey_map_with_unknown_t::iterator reduce_branch(const monkey_map_t & m, monkey_map_with_unknown_t & sent, const std::string & name) {
    if (name == HUMN) {
      return sent.insert(std::make_pair(HUMN, unknown_t{})).first;
    } else {
      return std::visit([&](const auto& v) {
          if constexpr (std::is_same_v<decltype(v), const value_t&>) {
            return sent.insert(std::make_pair(name, v)).first;
          } else {
            auto* lv = std::get_if<value_t>(&reduce_branch(m, sent, v.left)->second);
            auto* rv = std::get_if<value_t>(&reduce_branch(m, sent, v.right)->second);
            if (lv && rv) {
              value_t new_value;
              switch (v.operand) {
                case '+': new_value = *lv + *rv; break;
                case '-': new_value = *lv - *rv; break;
                case '*': new_value = *lv * *rv; break;
                case '/': new_value = *lv / *rv; break;
                default: throw std::runtime_error("unknown operand");
              }
              sent.erase(v.left);
              sent.erase(v.right);
              return sent.insert(std::make_pair(name, new_value)).first;
            } else {
              return sent.insert(std::make_pair(name, v)).first;
            }
          }
        }, m.at(name)
      );
    }
  }

  value_t solve_equality(const monkey_map_with_unknown_t& m, value_t target, const with_unknown_t& branch) {
    if (std::holds_alternative<unknown_t>(branch)) {
      return target;
    }
    auto* op = std::get_if<operation_t>(&branch);
    if (!op) {
      throw std::runtime_error("tree inconsistency");
    }
    const auto& l = m.at(op->left);
    const auto& r = m.at(op->right);
    const with_unknown_t* new_branch;
    const value_t* known = std::get_if<value_t>(&l);
    if (known) {
      new_branch = &r;
    } else {
      known = std::get_if<value_t>(&r);
      new_branch = &l;
    }
    if (!known) {
      throw std::runtime_error("no reduced branch");
    }
    switch (op->operand) {
      case '+': return solve_equality(m, target - *known, *new_branch);
      case '-': {
        value_t new_target;
        if (new_branch == &l) {
          new_target = target + *known;
        } else {
          new_target = *known - target;
        }
        return solve_equality(m, new_target, *new_branch);
      }
      case '*': return solve_equality(m, target / *known, *new_branch);
      case '/': {
        value_t new_target;
        if (new_branch == &l) {
          new_target = target * *known;
        } else {
          new_target = *known / target;
        }
        return solve_equality(m, new_target, *new_branch);
      }
      default: throw std::runtime_error("unknown operand");
    }
  }

  value_t solve_humn(std::istream& in) {
    monkey_map_with_unknown_t m;
    reduce_branch(parse_monkeys(in), m, "root");
    auto* op = std::get_if<operation_t>(&m.at("root"));
    if (!op) {
      throw std::runtime_error("tree inconsistency");
    }
    const auto& l = m.at(op->left);
    const auto& r = m.at(op->right);
    const with_unknown_t* new_branch;
    const value_t* known = std::get_if<value_t>(&l);
    if (known) {
      new_branch = &r;
    }
    else {
      known = std::get_if<value_t>(&r);
      new_branch = &l;
    }
    return solve_equality(m, *known, *new_branch);
  }

  REGISTER_DAY("d21",
    [](std::istream& in) {
      return std::to_string(
        solve(parse_monkeys(in), "root")
      );
    },
    [](std::istream& in) {
      return std::to_string(solve_humn(in));
    }
  )

  TEST(d21, parse_monkey) {
    {
      auto m = parse_monkey("root: pppw + sjmn");
      auto target = monkey_map_t::value_type("root", operation_t{ .left = "pppw", .right = "sjmn", .operand = '+' });
      EXPECT_EQ(m, target);
    }
    {
      auto m = parse_monkey("dbpl: 5");
      auto target = monkey_map_t::value_type("dbpl", 5);
      EXPECT_EQ(m, target);
    }
  }

  TEST(d21, part1) {
    auto in = std::istringstream(R"(
root: pppw + sjmn
dbpl: 5
cczh: sllz + lgvd
zczc: 2
ptdq: humn - dvpt
dvpt: 3
lfqf: 4
humn: 5
ljgn: 2
sjmn: drzm * dbpl
sllz: 4
pppw: cczh / lfqf
lgvd: ljgn * ptdq
drzm: hmdt - zczc
hmdt: 32
)");
    in.get();
    auto value = solve(parse_monkeys(in), "root");
    auto target = 152;
    ASSERT_EQ(value, target);
  }

  TEST(d21, solve_op) {
    {
      auto in = std::istringstream(R"(
root: add + total
add: addl + humn
addl: 5
humn: 5
total: 10
)");
      in.get();
      EXPECT_EQ(solve_humn(in), 5);
    }
    {
      auto in = std::istringstream(R"(
root: add + total
add: humn + addr
addr: 5
humn: 5
total: 10
)");
      in.get();
      EXPECT_EQ(solve_humn(in), 5);
    }
    {
      auto in = std::istringstream(R"(
root: sub + total
sub: subl - humn
subl: 10
humn: 3
total: 7
)");
      in.get();
      EXPECT_EQ(solve_humn(in), 3);
    }
    {
      auto in = std::istringstream(R"(
root: sub + total
sub: humn - subr
subr: 3
humn: 10
total: 7
)");
      in.get();
      EXPECT_EQ(solve_humn(in), 10);
    }
    {
      auto in = std::istringstream(R"(
root: mul + total
mul: mull * humn
mull: 3
humn: 5
total: 15
)");
      in.get();
      EXPECT_EQ(solve_humn(in), 5);
    }
    {
      auto in = std::istringstream(R"(
root: mul + total
mul: humn * mulr
mulr: 3
humn: 5
total: 15
)");
      in.get();
      EXPECT_EQ(solve_humn(in), 5);
    }
    {
      auto in = std::istringstream(R"(
root: div + total
div: divl / humn
divl: 15
humn: 5
total: 3
)");
      in.get();
      EXPECT_EQ(solve_humn(in), 5);
    }
    {
      auto in = std::istringstream(R"(
root: div + total
div: humn / divr
divr: 3
humn: 15
total: 5
)");
      in.get();
      EXPECT_EQ(solve_humn(in), 15);
    }
  }

  TEST(d21, part2) {
    auto in = std::istringstream(R"(
root: pppw + sjmn
dbpl: 5
cczh: sllz + lgvd
zczc: 2
ptdq: humn - dvpt
dvpt: 3
lfqf: 4
humn: 5
ljgn: 2
sjmn: drzm * dbpl
sllz: 4
pppw: cczh / lfqf
lgvd: ljgn * ptdq
drzm: hmdt - zczc
hmdt: 32
)");
    in.get();
    ASSERT_EQ(solve_humn(in), 301);
  }
}