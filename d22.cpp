#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <algorithm>

namespace d22 {

  using dim_t = int64_t;
  using dir_t = uint8_t;
  namespace dir {
    static constexpr dir_t right = 0, down = 1, left = 2, up = 3;

    const char* to_string(dir_t d) {
      switch (d) {
      case right: return "right";
      case down: return "down";
      case left: return "left";
      case up: return "up";
      default:
        assert(false);
        return nullptr;
      }
    }
  }

  struct vec_t {
    dim_t x = 0, y = 0;

    vec_t() = default;
    vec_t(dim_t x, dim_t y)
      : x(x), y(y)
    {}

    friend vec_t operator*(dim_t d, const vec_t& r) {
      return { r.x * d, r.y * d };
    }
  };

  static const vec_t dir_vecs[] = {
    { 1, 0 },
    { 0, 1 },
    { -1, 0 },
    { 0, -1 }
  };

  struct pos_t {
    dim_t x = 0, y = 0;

    pos_t() = default;
    pos_t(dim_t x, dim_t y)
      : x(x), y(y)
    {}

    auto operator<=>(const pos_t&) const = default;

    friend std::ostream& operator<<(std::ostream& out, const pos_t& p) {
      return out << '(' << p.x << ", " << p.y << ')';
    }

    pos_t& operator+=(const vec_t& r) {
      x += r.x;
      y += r.y;
      return *this;
    }

    pos_t& operator-=(const vec_t& r) {
      x -= r.x;
      y -= r.y;
      return *this;
    }

    friend pos_t operator+(const pos_t& l, const vec_t& r) {
      return pos_t{ l.x + r.x, l.y + r.y };
    }
  };

  struct advance_t {
    dim_t dist;

    auto operator<=>(const advance_t&) const = default;
    friend std::ostream& operator<<(std::ostream& out, const advance_t& v) { return out << v.dist; }
  };

  struct clockwise_t {
    auto operator<=>(const clockwise_t&) const = default;
    friend std::ostream& operator<<(std::ostream& out, const clockwise_t&) { return out << 'R'; }
  };

  struct counterclockwise_t {
    auto operator<=>(const counterclockwise_t&) const = default;
    friend std::ostream& operator<<(std::ostream& out, const counterclockwise_t&) { return out << 'L'; }
  };

  using action_t = std::variant<advance_t, clockwise_t, counterclockwise_t>;
  std::ostream& operator<<(std::ostream& out, const action_t& a) {
    return std::visit([&out](const auto& v) -> decltype(auto) {
      return out << v;
    }, a);
  }

  class action_stream_t {
  public:
    explicit action_stream_t(std::string data)
      : m_data(std::move(data))
      , m_cur(m_data.begin())
    {}

    std::optional<action_t> next() {
      std::optional<action_t> sent;
      if (m_cur != m_data.end()) {
        if (std::isdigit(*m_cur)) {
          auto end = std::find_if(m_cur, m_data.end(), [](char c) { return !std::isdigit(c); });
          sent = advance_t{ string_view_to<dim_t>(std::string_view(m_cur, end)) };
          m_cur = end;
        }
        else {
          if (*m_cur == 'R') {
            sent = clockwise_t{};
          }
          else if (*m_cur == 'L') {
            sent = counterclockwise_t{};
          }
          else {
            throw std::runtime_error("unknown command");
          }
          ++m_cur;
        }
      }
      return sent;
    }

  private:
    std::string m_data;
    std::string::iterator m_cur;
  };

  enum class cell_t {
    empty, path, wall
  };

  class map_t {
  public:
    explicit map_t(std::istream& in) {
      for (std::string line; std::getline(in, line) && !line.empty();) {
        m_map.push_back(std::move(line));
        m_map_width = std::max(m_map_width, static_cast<dim_t>(m_map.back().size()));
      }
      for (std::string& line : m_map) {
        line.resize(m_map_width, ' ');
      }
      for (m_face_size = std::min(map_width(), map_height()); true; --m_face_size)
      {
        if (map_height() % m_face_size != 0 || map_width() % m_face_size != 0) {
          continue;
        }
        size_t face_found = 0;
        for (dim_t y = 0; y < map_height(); y += (m_face_size + 1))
          for (dim_t x = 0; x < map_width(); x += (m_face_size + 1))
            if (at({ x, y }) != cell_t::empty)
              ++face_found;
        if (face_found == 6)
          break;
      }
      assert(m_face_size > 0);
    }

    cell_t operator[](const pos_t& p) const {
      return at(p);
    }

    cell_t at(const pos_t& p) const {
      if (p.y < 0 || p.y >= map_height()
        || p.x < 0 || p.x >= map_width()) {
        return cell_t::empty;
      }
      switch (m_map[p.y][p.x]) {
      case '#': return cell_t::wall;
      case '.': return cell_t::path;
      default: return cell_t::empty;
      }
    }

    dim_t face_size() const { return m_face_size; }
    dim_t map_height() const { return static_cast<dim_t>(m_map.size()); }
    dim_t map_width() const { return static_cast<int64_t>(m_map.front().size()); }

  private:

    std::vector<std::string> m_map;
    dim_t m_map_width = 0;
    dim_t m_face_size = 0;
  };

  struct you_t {
    dir_t dir;
    pos_t pos;

    static you_t from_map(const map_t& map) {
      pos_t pos;
      while (map[pos] == cell_t::empty) {
        pos.x += map.face_size();
      }
      while (map[pos] == cell_t::wall) {
        ++pos.x;
      }
      assert(map[pos] == cell_t::path);
      return you_t{ .dir = dir::right, .pos = pos };
    }

    auto operator<=>(const you_t&) const = default;

    dim_t password() const { return (pos.y + 1) * 1000 + (pos.x + 1) * 4 + dir; }

    void rotate_right() { dir = (dir + 1) % 4; }

    void rotate_left() { dir = (dir + 3) % 4; }

    void advance_wrap(const map_t& map, dim_t dist) {
      const vec_t dir_vec = dir_vecs[dir];
      for (; dist > 0; --dist) {
        pos_t next = pos + dir_vec;
        if (map[next] == cell_t::empty) {
          next -= 6 * map.face_size() * dir_vec;
          while (map[next] == cell_t::empty) {
            next += map.face_size() * dir_vec;
          }
        }
        assert(map[next] != cell_t::empty);
        if (map[next] == cell_t::wall) {
          break;
        }
        pos = next;
      }
    }

    friend std::ostream& operator<<(std::ostream& out, const you_t& you) {
      return out << dir::to_string(you.dir) << ' ' << you.pos;
    }
  };

  using advance_func_t = void(you_t::*)(const map_t&, dim_t);

  class action_visitor_t {
  public:
    explicit action_visitor_t(map_t map, advance_func_t advance)
      : m_map(std::move(map))
      , m_you(you_t::from_map(m_map))
      , m_advance(advance)
    {}

    const you_t& you() const { return m_you; }

    void operator()(const clockwise_t&) { m_you.rotate_right(); }
    void operator()(const counterclockwise_t&) { m_you.rotate_left(); }
    void operator()(const advance_t& a) {
      (m_you.*m_advance)(m_map, a.dist);
    }

  private:
    map_t m_map;
    you_t m_you;
    advance_func_t m_advance;
  };

  std::string next_line(std::istream& in) {
    std::string line;
    std::getline(in, line);
    return line;
  }

  std::string d22(std::istream& in, advance_func_t advance) {
    auto visitor = action_visitor_t{ map_t(in), advance };
    auto actions = action_stream_t(next_line(in));
    while (auto action = actions.next()) {
      std::visit(visitor, *action);
    }
    return std::to_string(visitor.you().password());
  }

  auto part1(std::istream& in) {
    return d22(in, &you_t::advance_wrap);
  }

  REGISTER_DAY("d22", &part1)

    TEST(d22, parsing) {
    auto in = std::istringstream(R"(
        ...#
        .#..
        #...
        ....
...#.......#
........#...
..#....#....
..........#.
        ...#....
        .....#..
        .#......
        ......#.

10R5L5R10L4R5L5
)");
    in.get();
    auto map = map_t(in);
    ASSERT_EQ(map.map_height(), 12);
    ASSERT_EQ(map.map_width(), 16);
    ASSERT_EQ(map.face_size(), 4);

    std::string line = next_line(in);
    const action_t targets[] = {
      advance_t{10},
      clockwise_t{},
      advance_t{5},
      counterclockwise_t{},
      advance_t{5},
      clockwise_t{},
      advance_t{10},
      counterclockwise_t{},
      advance_t{4},
      clockwise_t{},
      advance_t{5},
      counterclockwise_t{},
      advance_t{5}
    };
    auto g = action_stream_t(std::move(line));
    for (const action_t& target : targets) {
      auto found = g.next();
      ASSERT_TRUE(found);
      EXPECT_EQ(*found, target);
    }

    you_t you = you_t::from_map(map);
    ASSERT_EQ(you.dir, dir::right);
    pos_t target_pos = { 8, 0 };
    ASSERT_EQ(you.pos, target_pos);
  }

  TEST(d22, part1) {
    auto in = std::istringstream(R"(
        ...#
        .#..
        #...
        ....
...#.......#
........#...
..#....#....
..........#.
        ...#....
        .....#..
        .#......
        ......#.
)");
    in.get();

    const std::tuple<action_t, you_t> scenario[] = {
      { advance_t{10}, { dir::right, { 10, 0} } },
      { clockwise_t{}, { dir::down, { 10, 0} } },
      { advance_t{5}, { dir::down, { 10, 5} } },
      { counterclockwise_t{}, { dir::right, { 10, 5} } },
      { advance_t{5}, { dir::right, { 3, 5} } },
      { clockwise_t{}, { dir::down, { 3, 5} } },
      { advance_t{10}, { dir::down, { 3, 7} } },
      { counterclockwise_t{}, { dir::right, { 3, 7} } },
      { advance_t{4}, { dir::right, { 7, 7} } },
      { clockwise_t{}, { dir::down, { 7, 7} } },
      { advance_t{5}, { dir::down, { 7, 5} } },
      { counterclockwise_t{}, { dir::right, { 7, 5} } },
      { advance_t{5}, { dir::right, { 7, 5} } },
    };

    auto visitor = action_visitor_t(map_t(in), &you_t::advance_wrap);
    for (const auto& [action, res] : scenario) {
      auto was = visitor.you();
      std::visit(visitor, action);
      ASSERT_EQ(visitor.you(), res) << "while executing " << action << " while being in " << was;
    }
    ASSERT_EQ(visitor.you().password(), 6032);
  }
}