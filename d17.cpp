#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <array>

namespace d17 {

  class tetris {
  public:
    using cell_t = bool;
    static constexpr cell_t ROCKS = true;
    static constexpr cell_t SPACE = false;

    explicit tetris(std::string input)
      : m_input(std::move(input))
    {
      m_patterns.push_back({
        { ROCKS, ROCKS, ROCKS, ROCKS }
      });
      m_patterns.push_back({
        { SPACE, ROCKS, SPACE },
        { ROCKS, ROCKS, ROCKS },
        { SPACE, ROCKS, SPACE },
      });
      m_patterns.push_back({
        { ROCKS, ROCKS, ROCKS },
        { SPACE, SPACE, ROCKS },
        { SPACE, SPACE, ROCKS },
      });
      m_patterns.push_back({
        { ROCKS },
        { ROCKS },
        { ROCKS },
        { ROCKS }
      });
      m_patterns.push_back({
        { ROCKS, ROCKS },
        { ROCKS, ROCKS },
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

    size_t grid_height() const { return m_grid.size(); }

    cell_t grid_at(int64_t x, int64_t y) const {
      if (x < 0 || x >= 7 || y < 0) {
        return ROCKS;
      }
      if (y >= static_cast<int64_t>(m_grid.size())) {
        return SPACE;
      }
      return m_grid[y][x];
    }

    void debug_print() {
      std::cout << '+';
      for (size_t x = 0; x < 7; ++x) {
        std::cout << '-';
      }
      std::cout << "+\n";
      for (int64_t y : std::views::iota(int64_t(0), int64_t(m_grid.size())) | std::views::reverse) {
        std::cout << '|';
        for (size_t x = 0; x < 7; ++x) {
          std::cout << (grid_at(x, y) == ROCKS ? '#' : '.');
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
    using line_t = std::array<cell_t, 7>;
    using grid_t = std::vector<line_t>;
    using pattern_t = std::vector<std::vector<cell_t>>;

    bool collides(const pattern_t& p, int64_t x, int64_t y) const {
      for (size_t py = 0; py < p.size(); ++py) {
        for (size_t px = 0; px < p[py].size(); ++px) {
          if (p[py][px] == ROCKS
            && grid_at(x + px, y + py) == ROCKS) {
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
        for (size_t px = 0; px < p[py].size(); ++px) {
          if (p[py][px] == ROCKS) {
            m_grid[y + py][x + px] = ROCKS;
          }
        }
      }
    }

    std::vector<pattern_t> m_patterns;

    std::string m_input;
    size_t m_next_pattern = 0;
    size_t m_next_move = 0;
    grid_t m_grid;
  };

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
      auto t = tetris(std::move(line));
      for (size_t i = 0; i < 1000000000000; ++i) {
        t.drop_rock();
      }
      return std::to_string(t.grid_height());
    }
  )

  TEST(d17, tetris) {
    auto t = tetris(">>><<><>><<<>><>>><<<>>><<<><<<>><>><<>>");

    t.drop_rock();
    EXPECT_EQ(t.grid_height(), 1);
    t.drop_rock();
    EXPECT_EQ(t.grid_height(), 4);
    t.drop_rock();
    EXPECT_EQ(t.grid_height(), 6);
    t.drop_rock();
    EXPECT_EQ(t.grid_height(), 7);
    t.drop_rock();
    EXPECT_EQ(t.grid_height(), 9);
    t.drop_rock();
    EXPECT_EQ(t.grid_height(), 10);
    t.drop_rock();
    EXPECT_EQ(t.grid_height(), 13);
    t.drop_rock();
    EXPECT_EQ(t.grid_height(), 15);
    t.drop_rock();
    EXPECT_EQ(t.grid_height(), 17);
    t.drop_rock();
    EXPECT_EQ(t.grid_height(), 17);
  }

  TEST(d17, part1) {
    auto t = tetris(">>><<><>><<<>><>>><<<>>><<<><<<>><>><<>>");

    for (size_t i = 0; i < 2022; ++i) {
      t.drop_rock();
    }
    EXPECT_EQ(t.grid_height(), 3068);
  }

  TEST(d17, part2) {
    auto t = tetris(">>><<><>><<<>><>>><<<>>><<<><<<>><>><<>>");

    for (size_t i = 0; i < 1000000000000; ++i) {
      t.drop_rock();
    }
    EXPECT_EQ(t.grid_height(), 1514285714288);
  }
}