#include "days.hpp"
#include <gtest/gtest.h>
#include <set>
#include <array>

namespace d23 {

  enum class cell_t {
    ground,
    elf
  };

  using dist_t = int64_t;

  struct pos_t {
    dist_t x = 0, y = 0;

    auto operator<=>(const pos_t&) const = default;
  };

  std::ostream& operator<<(std::ostream& out, const pos_t& p) {
    return out << '(' << p.x << ", " << p.y << ')';
  }

  struct vec_t {
    dist_t x = 0, y = 0;
  };

  constexpr pos_t operator+(const pos_t& l, const vec_t& r) {
    return { .x = l.x + r.x, .y = l.y + r.y };
  }

  constexpr pos_t& operator+=(pos_t& l, const vec_t& r) {
    l.x += r.x;
    l.y += r.y;
    return l;
  }

  constexpr vec_t operator+(const vec_t& l, const vec_t& r) {
    return { .x = l.x + r.x, .y = l.y + r.y };
  }

  using elves_t = std::set<pos_t>;

  elves_t parse_elves(std::istream& input) {
    elves_t sent;
    dist_t y = 0;
    for (std::string line; std::getline(input, line) && !line.empty(); ++y) {
      for (size_t x = 0; x < line.size(); ++x) {
        if (line[x] == '#') {
          sent.insert(pos_t{ .x = static_cast<dist_t>(x), .y = y });
        }
      }
    }
    return sent;
  }

  enum dir_t : uint8_t
  {
    N = 0b00000011,
    E = 0b00001100,
    S = 0b00110000,
    W = 0b11000000
  };

  constexpr dir_t operator|(dir_t l, dir_t r) {
    return static_cast<dir_t>(static_cast<uint8_t>(l) | static_cast<uint8_t>(r));
  }

  static constexpr dir_t all_dirs[] = { N, E, S, W, N | E, S | E, S | W, N | W };

  constexpr vec_t vec_from_dir(dir_t d) {
    vec_t sent;
    if (d & N) { sent.y = -1; }
    else if (d & S) { sent.y = 1; }
    if (d & W) { sent.x = -1; }
    else if (d & E) { sent.x = 1; }
    return sent;
  }

  class neighbor_tracker_t {
  public:
    void track(dir_t d) {
      m_count += d & 0b01010101;
    }

    bool is_empty() const {
      return m_count == 0;
    }

    uint8_t count_for(dir_t d) {
      uint8_t sent = 0;
      uint8_t filtered = m_count & d;
      for (int i = 0; i < 4; ++i) {
        sent += filtered & 0b11;
        filtered >>= 2;
      }
      return sent;
    }

  private:
    uint8_t m_count = 0;
  };

  std::pair<elves_t, bool> move_elves(const elves_t& elves, std::span<const dir_t> dir_order) {
    // Step 1: suggestions
    std::map<pos_t, pos_t> suggestion;
    std::map<pos_t, size_t> counts;
    for (const pos_t& e : elves) {
      neighbor_tracker_t neighbor_tracker;
      for (dir_t dir : all_dirs) {
        if (elves.contains(e + vec_from_dir(dir))) {
          neighbor_tracker.track(dir);
        }
      }
      auto slot = suggestion.emplace(e, e).first;
      if (!neighbor_tracker.is_empty()) {
        for (dir_t d : dir_order) {
          if (neighbor_tracker.count_for(d) == 0) {
            slot->second += vec_from_dir(d);
            break;
          }
        }
      }
      auto [count_slot, inserted] = counts.try_emplace(slot->second, 1);
      if (!inserted) { ++count_slot->second; }
    }

    // Step 2: moves
    elves_t sent;
    bool moved = false;
    for (const auto& [from, to] : suggestion) {
      if (counts.at(to) == 1) {
        sent.insert(to);
        if (to != from) { moved = true; }
      } else {
        sent.insert(from);
      }
    }
    return { sent, moved };
  }

  class runner_t {
  public:
    explicit runner_t(elves_t initial_state)
      : m_elves(std::move(initial_state))
    {}

    const elves_t& elves() const { return m_elves; }

    bool tick() {
      auto [new_elves, moved] = move_elves(m_elves, m_dirs);
      m_elves = std::move(new_elves);
      std::rotate(m_dirs.begin(), m_dirs.begin() + 1, m_dirs.end());
      return moved;
    }

    dist_t empty_region() const {
      pos_t min = *m_elves.begin();
      pos_t max = *m_elves.begin();
      for (const pos_t& e : m_elves) {
        min.x = std::min(min.x, e.x);
        min.y = std::min(min.y, e.y);
        max.x = std::max(max.x, e.x);
        max.y = std::max(max.y, e.y);
      }
      return (max.y - min.y + 1) * (max.x - min.x + 1) - m_elves.size();
    }

  private:
    elves_t m_elves;
    std::array<dir_t, 4> m_dirs{ N, S, W, E };
  };

  REGISTER_DAY("d23",
    [](std::istream& in) {
      auto runner = runner_t(parse_elves(in));
      for (int i = 0; i < 10; ++i) {
        runner.tick();
      }
      return std::to_string(runner.empty_region());
    },
    [](std::istream& in) {
      auto runner = runner_t(parse_elves(in));
      size_t count = 1;
      while (runner.tick()) {
        ++count;
      }
      return std::to_string(count);
    }
  );

  TEST(d23, parsing) {
    auto in = std::istringstream(R"(
.....
..##.
..#..
.....
..##.
.....
)");
    in.get();
    auto res = parse_elves(in);
    ASSERT_EQ(res.size(), 5);
    auto expected = elves_t{
      { 2, 1 }, { 3, 1 }, { 2, 2 }, { 2, 4 }, { 3, 4 }
    };
    ASSERT_EQ(res, expected);
  }

  TEST(d23, basic) {
    auto in = std::istringstream(R"(
.....
..##.
..#..
.....
..##.
.....
)");
    in.get();
    auto runner = runner_t(parse_elves(in));
    elves_t expected[]{
      elves_t{ { 2, 0 }, { 3, 0 }, { 2, 2 }, { 2, 4 }, { 3, 3 }},
      elves_t{ { 2, 1 }, { 3, 1 }, { 1, 2 }, { 4, 3 }, { 2, 5 }},
      elves_t{ { 2, 0 }, { 4, 1 }, { 0, 2 }, { 4, 3 }, { 2, 5 }},
    };
    for (const elves_t& e : expected) {
      runner.tick();
      ASSERT_EQ(runner.elves(), e);
    }
  }

  TEST(d23, part1) {
    auto in = std::istringstream(R"(
..............
..............
.......#......
.....###.#....
...#...#.#....
....#...##....
...#.###......
...##.#.##....
....#..#......
..............
..............
..............
)");
    in.get();
    auto runner = runner_t(parse_elves(in));
    for (int i = 0; i < 10; ++i) {
      runner.tick();
    }
    ASSERT_EQ(runner.empty_region(), 110);
  }

  TEST(d23, part2) {
    auto in = std::istringstream(R"(
..............
..............
.......#......
.....###.#....
...#...#.#....
....#...##....
...#.###......
...##.#.##....
....#..#......
..............
..............
..............
)");
    in.get();
    auto runner = runner_t(parse_elves(in));
    size_t count = 1;
    while (runner.tick()) {
      ++count;
    }
    ASSERT_EQ(count, 20);
  }
}