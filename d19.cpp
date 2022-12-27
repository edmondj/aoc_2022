#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <regex>
#include <array>

namespace d19 {

  enum class minerals_t : unsigned {
    ore, clay, obsidian, geode
  };

  static constexpr auto all_minerals = std::array<minerals_t, 4>{
    minerals_t::ore, minerals_t::clay, minerals_t::obsidian, minerals_t::geode
  };

  minerals_t parse_minerals(std::string_view name) {
    if (name == "ore") { return minerals_t::ore; }
    if (name == "clay") { return minerals_t::clay; }
    if (name == "obsidian") { return minerals_t::obsidian; }
    if (name == "geode") { return minerals_t::geode; }
    throw std::runtime_error("invalid mineral");
  }

  template<typename T>
  struct mineral_map_t : public std::array<T, 4> {
    mineral_map_t() noexcept {
      this->fill(T{});
    }

    const T& operator[](minerals_t m) const {
      return static_cast<const std::array<T, 4>&>(*this)[static_cast<unsigned>(m)];
    }

    T& operator[](minerals_t m) {
      return static_cast<std::array<T, 4>&>(*this)[static_cast<unsigned>(m)];
    }

    auto operator<=>(const mineral_map_t&) const = default;
  };
  
  using quantity_t = uint64_t;
  using ore_t = mineral_map_t<quantity_t>;
  using blueprints_t = mineral_map_t<ore_t>;

  blueprints_t parse_blueprint(std::string_view line) {
    auto sent = blueprints_t{};

    if (auto column = line.find(':'); column != std::string_view::npos) {
      line = line.substr(column + 1);
    }
    for (auto&& subrange : line | std::views::split(std::string_view{ "." })) {
      static const auto re = std::regex(R"( Each (\w+) robot costs (\d+) (\w+)( and (\d+) (\w+))?)");

      auto blueprint_str = std::string_view(subrange.begin(), subrange.end());
      std::match_results<std::string_view::const_iterator> matches;
      if (std::regex_match(blueprint_str.begin(), blueprint_str.end(), matches, re)) {
        minerals_t robot = parse_minerals(matches[1].str());
        sent[robot][parse_minerals(matches[3].str())] = string_view_to<quantity_t>(matches[2].str());
        if (matches[4].length() > 0) {
          sent[robot][parse_minerals(matches[6].str())] = string_view_to<quantity_t>(matches[5].str());
        }
      }
    }

    return sent;
  }

  struct factory_t {
    ore_t ores;
    ore_t robots;

    auto operator<=>(const factory_t&) const = default;

    factory_t() noexcept {
      robots[minerals_t::ore] = 1;
    }

    void advance(size_t time = 1) {
      for (minerals_t m : all_minerals) {
        ores[m] += robots[m] * time;
      }
    }

    void extract(const ore_t& to_remove) {
      for (minerals_t m : all_minerals) {
        assert(ores[m] >= to_remove[m]);
        ores[m] -= to_remove[m];
      }
    }
  };

  std::optional<size_t> time_to_build(const factory_t& f, const ore_t& requirements) {
    size_t time = 0;
    for (minerals_t m : all_minerals) {
      if (requirements[m] > 0 && f.robots[m] == 0) {
        return std::nullopt;
      }
      if (requirements[m] > 0 && f.ores[m] < requirements[m]) {
        time = std::max(time, (requirements[m] - f.ores[m] + f.robots[m] - 1) / f.robots[m]);
      }
    }
    return time + 1;
  }

#define DEBUG_HISTORY 0

  quantity_t maximize_geodes(size_t time, const blueprints_t& blueprints) {
    struct state_t {
      size_t time = 0;
      factory_t factory;
#if DEBUG_HISTORY
      std::vector<state_t> history;
#endif
    };

    quantity_t max_geodes = 0;
    auto states = std::vector<state_t>(1);
    mineral_map_t<quantity_t> max_rpm;
    for (minerals_t m : all_minerals) {
      for (minerals_t robot : all_minerals) {
        max_rpm[m] = std::max(max_rpm[m], blueprints[robot][m]);
      }
    }

    while (!states.empty()) {
      std::vector<state_t> new_states;
      for (const state_t& s : states) {
        for (minerals_t r : all_minerals) {
          if (r != minerals_t::geode && s.factory.robots[r] >= max_rpm[r]) {
            continue;
          }
          auto time_needed = time_to_build(s.factory, blueprints[r]);
          if (time_needed && *time_needed < (time - s.time)) {
            auto& new_state = new_states.emplace_back(s);
            new_state.time += *time_needed;
            new_state.factory.advance(*time_needed);
            new_state.factory.robots[r] += 1;
            new_state.factory.extract(blueprints[r]);
#if DEBUG_HISTORY
            new_state.history.push_back(s);
#endif
          }
        }
        quantity_t potential_geodes = s.factory.ores[minerals_t::geode] + s.factory.robots[minerals_t::geode] * (time - s.time);
        if (potential_geodes > max_geodes) {
          max_geodes = potential_geodes;
        }
      }
      states = std::move(new_states);
    }
    return max_geodes;
  }

  REGISTER_DAY("d19",
    [](std::istream& input) {
      std::string line;
      size_t total = 0;
      for (size_t i = 1; std::getline(input, line); ++i) {
        total += i * maximize_geodes(24, parse_blueprint(line));
      }
      return std::to_string(total);
    },
    [](std::istream& input) {
      std::string line;
      size_t total = 1;
      for (size_t i = 1; std::getline(input, line) && i <= 3; ++i) {
        total *= maximize_geodes(32, parse_blueprint(line));
      }
      return std::to_string(total);
    }
  )

  TEST(d19, parsing) {
    blueprints_t b = parse_blueprint("Blueprint 1: Each ore robot costs 4 ore. Each clay robot costs 2 ore. Each obsidian robot costs 3 ore and 14 clay. Each geode robot costs 2 ore and 7 obsidian.");
    EXPECT_EQ(b[minerals_t::ore][minerals_t::ore], 4);
    EXPECT_EQ(b[minerals_t::ore][minerals_t::clay], 0);
    EXPECT_EQ(b[minerals_t::ore][minerals_t::obsidian], 0);
    EXPECT_EQ(b[minerals_t::ore][minerals_t::geode], 0);

    EXPECT_EQ(b[minerals_t::clay][minerals_t::ore], 2);
    EXPECT_EQ(b[minerals_t::clay][minerals_t::clay], 0);
    EXPECT_EQ(b[minerals_t::clay][minerals_t::obsidian], 0);
    EXPECT_EQ(b[minerals_t::clay][minerals_t::geode], 0);

    EXPECT_EQ(b[minerals_t::obsidian][minerals_t::ore], 3);
    EXPECT_EQ(b[minerals_t::obsidian][minerals_t::clay], 14);
    EXPECT_EQ(b[minerals_t::obsidian][minerals_t::obsidian], 0);
    EXPECT_EQ(b[minerals_t::obsidian][minerals_t::geode], 0);

    EXPECT_EQ(b[minerals_t::geode][minerals_t::ore], 2);
    EXPECT_EQ(b[minerals_t::geode][minerals_t::clay], 0);
    EXPECT_EQ(b[minerals_t::geode][minerals_t::obsidian], 7);
    EXPECT_EQ(b[minerals_t::geode][minerals_t::geode], 0);
  }

  TEST(d19, time_to_build) {
    auto f = factory_t{};
    blueprints_t b = parse_blueprint("Blueprint 1: Each ore robot costs 4 ore. Each clay robot costs 2 ore. Each obsidian robot costs 3 ore and 14 clay. Each geode robot costs 2 ore and 7 obsidian.");
    ASSERT_EQ(time_to_build(f, b[minerals_t::clay]), 3);
    f.advance(3);
    f.extract(b[minerals_t::clay]);
    f.robots[minerals_t::clay] += 1;
    ASSERT_EQ(time_to_build(f, b[minerals_t::clay]), 2);
    f.advance(2);
    f.extract(b[minerals_t::clay]);
    f.robots[minerals_t::clay] += 1;
    ASSERT_EQ(time_to_build(f, b[minerals_t::clay]), 2);
    f.advance(2);
    f.extract(b[minerals_t::clay]);
    f.robots[minerals_t::clay] += 1;
    ASSERT_EQ(time_to_build(f, b[minerals_t::obsidian]), 4);
    f.advance(4);
    f.extract(b[minerals_t::obsidian]);
    f.robots[minerals_t::obsidian] += 1;
    ASSERT_EQ(time_to_build(f, b[minerals_t::clay]), 1);
    f.advance(1);
    f.extract(b[minerals_t::clay]);
    f.robots[minerals_t::clay] += 1;
    ASSERT_EQ(time_to_build(f, b[minerals_t::obsidian]), 3);
    f.advance(3);
    f.extract(b[minerals_t::obsidian]);
    f.robots[minerals_t::obsidian] += 1;
    ASSERT_EQ(time_to_build(f, b[minerals_t::geode]), 3);
    f.advance(3);
    f.extract(b[minerals_t::geode]);
    f.robots[minerals_t::geode] += 1;
    ASSERT_EQ(time_to_build(f, b[minerals_t::geode]), 3);
    f.advance(3);
    f.extract(b[minerals_t::geode]);
    f.robots[minerals_t::geode] += 1;
  }

  TEST(d19, part1) {
    blueprints_t b1 = parse_blueprint("Blueprint 1: Each ore robot costs 4 ore. Each clay robot costs 2 ore. Each obsidian robot costs 3 ore and 14 clay. Each geode robot costs 2 ore and 7 obsidian.");
    EXPECT_EQ(maximize_geodes(24, b1), 9);

    blueprints_t b2 = parse_blueprint("Blueprint 2: Each ore robot costs 2 ore. Each clay robot costs 3 ore. Each obsidian robot costs 3 ore and 8 clay. Each geode robot costs 3 ore and 12 obsidian.");
    EXPECT_EQ(maximize_geodes(24, b2), 12);
  }
}
