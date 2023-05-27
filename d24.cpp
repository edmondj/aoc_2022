#include "days.hpp"
#include <gtest/gtest.h>
#include <ranges>
#include <algorithm>

namespace d24 {

  enum dir_t : uint8_t {
    up = 1,
    right = 2,
    down = 4,
    left = 8
  };

  dir_t cardinal_dirs[]{ up, right, down, left };

  class winds_t {
  public:

    winds_t() = default;
    winds_t(dir_t d) { add(d); }

    bool has_any() const { return m_winds != 0; }
    bool has(dir_t d) const { return (m_winds & d) != 0; }
    void clear() { m_winds = 0; }

    void add(dir_t d) {
      assert(!has(d));
      m_winds |= d;
    }

    auto operator<=>(const winds_t&) const = default;

  private:
    uint8_t m_winds = 0;
  };

  struct cell_t {

    enum kind_t {
      empty,
      wall
    };

    kind_t kind = empty;
    winds_t winds;

    cell_t() = default;
    cell_t(kind_t kind)
      : kind(kind)
    {}
    cell_t(winds_t winds)
      : kind(empty)
      , winds(winds)
    {}
    cell_t(dir_t d)
      : cell_t(winds_t{d})
    {}

    static cell_t from_char(char c) {
      switch (c) {
      case '.': return empty;
      case '#': return wall;
      case '^': return up;
      case '>': return right;
      case 'v': return down;
      case '<': return left;
      default:
        std::unreachable();
      }
    }

    auto operator<=>(const cell_t&) const = default;
  };

  std::ostream& operator<<(std::ostream& out, const cell_t& cell) {
    switch (cell.kind) {
    case cell_t::empty: {
      size_t count = 0;
      char c = 0;
      if (cell.winds.has(up)) {
        c = '^';
        ++count;
      }
      if (cell.winds.has(right)) {
        c = '>';
        ++count;
      }
      if (cell.winds.has(down)) {
        c = 'v';
        ++count;
      }
      if (cell.winds.has(left)) {
        c = '<';
        ++count;
      }
      switch (count) {
      case 0: out << '.'; break;
      case 1: out << c; break;
      default: out << count; break;
      }
      break;
    }
    case cell_t::wall: out << '#'; break;
    default:
      std::unreachable();
    }
    return out;
  }

  using dist_t = int64_t;

  struct pos_t {
    dist_t x = 0, y = 0;

    auto operator<=>(const pos_t&) const = default;
  };

  class map_t {
  public:

    static map_t from_stream(std::istream& in) {
      map_t sent;
      for (std::string line; std::getline(in, line) && !line.empty(); ++sent.m_height) {
        assert(sent.m_width == 0 || sent.m_width == static_cast<dist_t>(line.size()));
        sent.m_width = static_cast<dist_t>(line.size());
        for (char c : line) {
          sent.m_cells.push_back(cell_t::from_char(c));
        }
      }
      sent.m_start = { .x = 1, .y = 0 };
      assert(sent[sent.m_start] == cell_t::empty);
      sent.m_end = { .x = sent.width() - 2, .y = sent.height() - 1};
      assert(sent[sent.m_end] == cell_t::empty);
      return sent;
    }

    template<typename Self>
    auto&& at(this Self&& self, const pos_t& pos) {
      assert(pos.x >= 0 && pos.x < self.m_width && pos.y >= 0 && pos.y < self.m_height);
      return std::forward<Self>(self).m_cells.at(pos.y * self.m_width + pos.x);
    }

    template<typename Self>
    auto&& operator[](this Self&& self, const pos_t& pos) {
      return std::forward<Self>(self).at(pos);
    }

    dist_t width() const { return m_width; }
    dist_t height() const { return m_height; }
    pos_t start() const { return m_start; }
    pos_t end() const { return m_end; }

    auto operator<=>(const map_t&) const = default;

    void clear_wind() {
      for (cell_t& c : m_cells) {
        c.winds.clear();
      }
    }

    friend void swap(map_t& l, map_t& r) {
      std::swap(l.m_width, r.m_width);
      std::swap(l.m_height, r.m_height);
      std::swap(l.m_cells, r.m_cells);
    }

  private:
    map_t() = default;

    pos_t m_start;
    pos_t m_end;
    dist_t m_width = 0;
    dist_t m_height = 0;
    std::vector<cell_t> m_cells;
  };

  struct vec_t {
    dist_t x = 0, y = 0;

    static vec_t from_dir(dir_t d) {
      switch (d) {
      case up: return { 0, -1 };
      case right: return { 1, 0 };
      case down: return { 0, 1 };
      case left: return { -1, 0 };
      default:
        std::unreachable();
      }
    }
  };

  pos_t operator+(const pos_t& l, const vec_t& r) {
    return { l.x + r.x, l.y + r.y };
  }

  pos_t advance(pos_t from, dist_t width) {
    if (++from.x == width) {
      from.x = 0;
      ++from.y;
    }
    return from;
  }

  class moving_map_t : public map_t {
  public:
    
    static moving_map_t from_stream(std::istream& in) {
      return map_t::from_stream(in);
    }

    void move() {
      m_buffer.clear_wind();
      for (auto p = pos_t{ 0, 0 }; p.x < width() && p.y < height(); p = advance(p, width())) {
        cell_t source = at(p);
        if (source.winds.has_any()) {
          for (dir_t d : cardinal_dirs) {
            if (source.winds.has(d)) {
              auto vec = vec_t::from_dir(d);
              cell_t* dest = &m_buffer.at(p + vec);
              if (dest->kind == cell_t::wall) {
                dest = &m_buffer.at(p + vec_t{ vec.x * -(width() - 3), vec.y * -(height() - 3) });
              }
              assert(dest->kind == cell_t::empty);
              dest->winds.add(d);
            }
          }
        }
      }
      swap(*this, m_buffer);
    }

  private:
    moving_map_t(map_t initial_state)
      : map_t(initial_state)
      , m_buffer(std::move(initial_state))
    {}

    map_t m_buffer;
  };

  auto solve_fastest(moving_map_t& map, pos_t from, pos_t to) {
    auto to_visit = std::set<pos_t>{ from };
    for (size_t count = 1; true; ++count) {
      std::set<pos_t> new_pos;
      auto check_add = [&](pos_t p) {
        if (p.x >= 0 && p.x < map.width()
          && p.y >= 0 && p.y < map.height()
          && map[p] == cell_t::empty
          ) {
          new_pos.insert(p);
        }
      };

      map.move();
      for (pos_t p : to_visit) {
        check_add(p);
        for (dir_t d : cardinal_dirs) {
          pos_t next = p + vec_t::from_dir(d);
          if (next == to) {
            return count;
          }
          check_add(next);
        }
      }
      to_visit = std::move(new_pos);
    }
  }

  auto solve_fastest(moving_map_t map) {
    return solve_fastest(map, map.start(), map.end());
  }

  REGISTER_DAY("d24",
    [](std::istream& in) {
      return std::to_string(solve_fastest(moving_map_t::from_stream(in)));
    },
    [](std::istream& in) {
      auto map = moving_map_t::from_stream(in);
      auto time = solve_fastest(map, map.start(), map.end());
      time += solve_fastest(map, map.end(), map.start());
      time += solve_fastest(map, map.start(), map.end());
      return std::to_string(time);
    }
  )

  TEST(d24, parsing) {
    auto in = std::istringstream(R"(
#.######
#>>.<^<#
#.<..<<#
#>v.><>#
#<^v^^>#
######.#
)");
    in.get();
    auto map = map_t::from_stream(in);
    EXPECT_EQ(map.width(), 8);
    EXPECT_EQ(map.height(), 6);
    EXPECT_EQ(map.start(), pos_t(1, 0));
    EXPECT_EQ(map.end(), pos_t(6, 5));
    EXPECT_EQ(map.at({ .x = 1, .y = 0 }), cell_t::empty);
    EXPECT_EQ(map.at({ .x = 6, .y = 5 }), cell_t::empty);
    EXPECT_EQ(map.at({ .x = 1, .y = 1 }), right);
    EXPECT_EQ(map.at({ .x = 0, .y = 1 }), cell_t::wall);
    EXPECT_EQ(map.at({ .x = 3, .y = 1 }), cell_t::empty);
    EXPECT_EQ(map.at({ .x = 4, .y = 1 }), left);
    EXPECT_EQ(map.at({ .x = 5, .y = 1 }), up);
    EXPECT_EQ(map.at({ .x = 2, .y = 3 }), down);
  }

  TEST(d24, move) {
    auto in = std::istringstream(R"(
#.######
#>>.<^<#
#.<..<<#
#>v.><>#
#<^v^^>#
######.#
)");
    in.get();
    auto map = moving_map_t::from_stream(in);

    const char* expected[] = { R"(
#.######
#E>3.<.#
#<..<<.#
#>2.22.#
#>v..^<#
######.#
)", R"(
#.######
#.2>2..#
#E^22^<#
#.>2.^>#
#.>..<.#
######.#
)", R"(
#.######
#<^<22.#
#E2<.2.#
#><2>..#
#..><..#
######.#
)"
    };
    for (const char* e : expected) {
      map.move();
      for (auto p = pos_t{ 0, 0 }; p.y < map.height(); p = advance(p, map.width())) {
        char c = e[p.y * map.width() + p.x
          + p.y + 1 // skipping \n
        ];
        auto cell = map[p];
        switch (c) {
        case '#':
          EXPECT_EQ(cell, cell_t::wall);
          break;
        case '.':
        case 'E':
          EXPECT_EQ(cell, cell_t::empty);
          break;
        default:
        {
          EXPECT_EQ(cell.kind, cell_t::empty);
          if (c >= '2' && c <= '4') {
            int8_t count = 0;
            for (dir_t d : cardinal_dirs) {
              if (cell.winds.has(d)) {
                ++count;
              }
            }
            EXPECT_EQ(c - '0', count);
          } else {
            EXPECT_EQ(cell, cell_t::from_char(c));
          }
          break;
        }
        }
      }
    }
  }

  TEST(d24, part1) {
    auto in = std::istringstream(R"(
#.######
#>>.<^<#
#.<..<<#
#>v.><>#
#<^v^^>#
######.#
)");
    in.get();

    auto res = solve_fastest(moving_map_t::from_stream(in));
    ASSERT_EQ(res, 18);
  }

  TEST(d24, part2) {
    auto in = std::istringstream(R"(
#.######
#>>.<^<#
#.<..<<#
#>v.><>#
#<^v^^>#
######.#
)");
    in.get();

    auto map = moving_map_t::from_stream(in);
    EXPECT_EQ(solve_fastest(map, map.start(), map.end()), 18);
    EXPECT_EQ(solve_fastest(map, map.end(), map.start()), 23);
    EXPECT_EQ(solve_fastest(map, map.start(), map.end()), 13);
  }
}