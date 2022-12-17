#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <array>

namespace d17 {

  class tetris {
  public:
    using line_t = uint8_t;
    using grid_t = std::vector<line_t>;
    using pattern_t = std::vector<line_t>;

    explicit tetris(std::string input)
      : m_input(std::move(input))
    {
      m_patterns.push_back({
        0b11110000
      });
      m_patterns.push_back({
        0b01000000,
        0b11100000,
        0b01000000
      });
      m_patterns.push_back({
        0b11100000,
        0b00100000,
        0b00100000
      });
      m_patterns.push_back({
        0b10000000,
        0b10000000,
        0b10000000,
        0b10000000
      });
      m_patterns.push_back({
        0b11000000,
        0b11000000
      });
    }

    void drop_rock() {
      const auto& pattern = m_patterns[m_next_pattern];
      m_next_pattern = (m_next_pattern + 1) % m_patterns.size();

      int64_t x = 2;
      int64_t y = m_grid.size() + 3;
      while (true) {
        int64_t offset = m_input[m_next_move] == '<' ? -1 : 1;
        m_next_move = (m_next_move + 1) % m_input.size();

        if (!collides(pattern, x + offset, y)) {
          x += offset;
        }
        if (!collides(pattern, x, y - 1)) {
          y -= 1;
        } else {
          break;
        }
      }
      merge(pattern, x, y);
    }

    struct rock_cycle_t {
      std::vector<size_t> first_cycles_height;
      size_t cycle_start;
      size_t cycle_duration;
      size_t cycle_height;

      size_t height_of_iter(size_t iter) const {
        if (iter < cycle_start) {
          return first_cycles_height[iter];
        }
        iter -= cycle_start;
        return cycle_height * (iter / cycle_duration)
          + first_cycles_height[cycle_start + iter % cycle_duration];
      }
    };

    static rock_cycle_t find_cycles(std::string input) {
      struct state_t {
        size_t next_pattern;
        size_t next_move;
        grid_t last_grid;

        auto operator<=>(const state_t&) const = default;
      };

      auto t = tetris(std::move(input));

      auto visited = std::map<state_t, size_t>{
        {state_t{
          .next_pattern = t.m_next_pattern,
          .next_move = t.m_next_move,
          .last_grid = grid_t{}
        }, 0}
      };
      auto heights = std::vector<size_t>{ {0} };

      while (true) {
        t.drop_rock();
        auto max_drop = t.m_grid.end() - 1;
        line_t mask = 0;
        while (max_drop != t.m_grid.begin() && mask != 0b11111110) {
          mask |= *max_drop;
          --max_drop;
        }
        auto state = state_t{
          .next_pattern = t.m_next_pattern,
          .next_move = t.m_next_move,
          .last_grid = grid_t(max_drop, t.m_grid.end())
        };
        auto [it, inserted] = visited.insert(std::make_pair(std::move(state), heights.size()));
        if (!inserted) {
          size_t cycle_start = it->second;
          size_t cycle_duration = heights.size() - cycle_start;
          size_t cycle_height = t.grid_height() - heights[cycle_start];
          // we found the cycle
          return rock_cycle_t{
            .first_cycles_height = std::move(heights),
            .cycle_start = cycle_start,
            .cycle_duration = cycle_duration,
            .cycle_height = cycle_height
          };
        }
        heights.push_back(t.grid_height());
      }
    }

    size_t grid_height() const { return m_grid.empty() ? 0 : m_grid.size(); }

    void debug_print() {
      std::cout << '+';
      for (size_t x = 0; x < 7; ++x) {
        std::cout << '-';
      }
      std::cout << "+\n";
      for (int64_t y : std::views::iota(int64_t(0), int64_t(m_grid.size())) | std::views::reverse) {
        std::cout << '|';
        for (size_t x = 0; x < 7; ++x) {
          std::cout << (((m_grid[y] >> (7 - x)) & 1) == 1 ? '#' : '.');
        }
        std::cout << "|\n";
      }
      std::cout << '+';
      for (size_t x = 0; x < 7; ++x) {
        std::cout << '-';
      }
      std::cout << "+\n";
    }


  private:
    bool collides(const pattern_t& p, int64_t x, int64_t y) const {
      if (y < 0 || x < 0) {
        return true;
      }
      for (size_t py = 0; py < p.size(); ++py) {
        line_t line = p[py];
        if ((line & (1 << (x + 1)) - 1) != 0) {
          return true;
        }
        if (y + py < m_grid.size()) {
          if ((m_grid[y + py] & (line >> x)) != 0) {
            return true;
          }
        }
      }
      return false;
    }

    void merge(const pattern_t& p, int64_t x, int64_t y) {
      for (size_t py = 0; py < p.size(); ++py) {
        if (m_grid.size() <= y + py) {
          m_grid.push_back(line_t{});
        }
        m_grid[y + py] |= (p[py] >> x);
      }
    }

    std::vector<pattern_t> m_patterns;

    std::string m_input;
    size_t m_next_pattern = 0;
    size_t m_next_move = 0;
    grid_t m_grid;
  };

  size_t find_height_after(std::string input, size_t iter) {
    auto cycles = tetris::find_cycles(std::move(input));
    return cycles.height_of_iter(iter);
  }

  REGISTER_DAY("d17",
    [](std::istream& in) {
      std::string line;
      std::getline(in, line);
      auto t = tetris(std::move(line));
      for (size_t i = 0; i < 2022; ++i) {
        t.drop_rock();
      }
      return std::to_string(t.grid_height());
    },
    [](std::istream& in) {
      std::string line;
      std::getline(in, line);
      return std::to_string(find_height_after(std::move(line), 1000000000000));
    }
  )

  TEST(d17, tetris) {
    auto t = tetris(">>><<><>><<<>><>>><<<>>><<<><<<>><>><<>>");

    t.drop_rock();
    ASSERT_EQ(t.grid_height(), 1);
    t.drop_rock();
    ASSERT_EQ(t.grid_height(), 4);
    t.drop_rock();
    ASSERT_EQ(t.grid_height(), 6);
    t.drop_rock();
    ASSERT_EQ(t.grid_height(), 7);
    t.drop_rock();
    ASSERT_EQ(t.grid_height(), 9);
    t.drop_rock();
    ASSERT_EQ(t.grid_height(), 10);
    t.drop_rock();
    ASSERT_EQ(t.grid_height(), 13);
    t.drop_rock();
    ASSERT_EQ(t.grid_height(), 15);
    t.drop_rock();
    ASSERT_EQ(t.grid_height(), 17);
    t.drop_rock();
    ASSERT_EQ(t.grid_height(), 17);
  }

  TEST(d17, cycles) {
    std::string input = ">>><<><>><<<>><>>><<<>>><<<><<<>><>><<>>";
    auto cycles = tetris::find_cycles(input);
    auto t = tetris(input);
    for (size_t i = 0; i <= cycles.cycle_start + 2 * cycles.cycle_duration; ++i) {
      ASSERT_EQ(cycles.height_of_iter(i), t.grid_height()) << " i == " << i;
      t.drop_rock();
    }
  }

  TEST(d17, cycles_real) {
    auto f = std::ifstream("input/d17.txt");
    std::string input;
    std::getline(f, input);
    auto cycles = tetris::find_cycles(input);
    auto t = tetris(input);
    for (size_t i = 0; i <= cycles.cycle_start + 2 * cycles.cycle_duration; ++i) {
      ASSERT_EQ(cycles.height_of_iter(i), t.grid_height()) << " i == " << i;
      t.drop_rock();
    }
  }

  TEST(d17, part1) {
    auto size = find_height_after(">>><<><>><<<>><>>><<<>>><<<><<<>><>><<>>", 2022);
    ASSERT_EQ(size, 3068);
  }

  TEST(d17, part2) {
    auto size = find_height_after(">>><<><>><<<>><>>><<<>>><<<><<<>><>><<>>", 1000000000000);
    ASSERT_EQ(size, 1514285714288);
  }
}