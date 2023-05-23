#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <algorithm>
#include <queue>

namespace d22 {

  using dim_t = int64_t;
  using dir_t = uint8_t;
  namespace dir {
    static constexpr dir_t right = 0, down = 1, left = 2, up = 3;
    static constexpr dir_t all_dirs[] { right, down, left, up };

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

  dir_t rotated_right(dir_t d, uint8_t angle) { return (d + angle) % 4; }
  dir_t rotated_left(dir_t d, uint8_t angle) { assert(angle <= 8); return (d + 8 - angle) % 4; }

  struct vec_t {
    dim_t x = 0, y = 0;

    vec_t() = default;
    vec_t(dim_t x, dim_t y)
      : x(x), y(y)
    {}

    friend vec_t operator*(dim_t d, const vec_t& r) {
      return { r.x * d, r.y * d };
    }

    friend vec_t operator*(const vec_t& r, dim_t d) {
      return d * r;
    }
  };

  vec_t rotate_vec(const vec_t& v, dir_t angle) {
    switch (angle) {
    case 0: return v;
    case 1: return { -v.y, v.x };
    case 2: return { -v.x, -v.y };
    case 3: return { v.y, -v.x };
    default:
      assert(false);
      return {};
    }
  }

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

    friend pos_t operator-(const pos_t& l, const vec_t& r) {
      return pos_t{ l.x - r.x, l.y - r.y };
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

  using face_id_t = uint8_t;

  struct face_t {
    face_id_t id;
    uint8_t rotation;

    auto operator<=>(const face_t&) const = default;

    friend std::ostream& operator<<(std::ostream& out, const face_t& face) {
      return out << '(' << static_cast<int>(face.id) << ' ' << (face.rotation * 90) << ')';
    }
  };

  struct edge_t {
    face_id_t from;
    dir_t dir;
    face_t to;
  };

  static const edge_t die_edges[] = {
    { 1, dir::right, { 3, 0 } },
    { 1, dir::down, { 2, 2 } },
    { 1, dir::left, { 4, 1 } },
    { 1, dir::up, { 5, 0 } },
    { 2, dir::right, { 4, 2 } },
    { 2, dir::left, { 3, 3 } },
    { 2, dir::up, { 6, 0 } },
    { 3, dir::right, { 6, 0 } },
    { 3, dir::up, { 5, 1 } },
    { 4, dir::down, { 6, 3 } },
    { 4, dir::left, { 5, 2 } },
    { 5, dir::up, { 6, 2} }
  };

  face_t face_next_to(face_id_t from, dir_t d) {
    for (const edge_t& edge : die_edges) {
      if (from == edge.from && d == edge.dir) { return edge.to; }

      if (from == edge.to.id && rotated_left(rotated_right(edge.dir, 2), edge.to.rotation) == d) {
        return { edge.from, static_cast<uint8_t>((4 - edge.to.rotation) % 4) };
      }
    }
    assert(false);
    return {};
  }

  struct absolute_face_t {
    pos_t root;
    face_t face;

    auto operator<=>(const absolute_face_t&) const = default;
  };

  class map_t {
  public:
    explicit map_t(std::istream& in, dim_t face_size)
      : m_face_size(face_size)
    {
      for (std::string line; std::getline(in, line) && !line.empty();) {
        m_map.push_back(std::move(line));
        m_map_width = std::max(m_map_width, static_cast<dim_t>(m_map.back().size()));
      }
      auto first_pos = pos_t{ 0, 0 };
      while (at(first_pos) == cell_t::empty)
        first_pos.x += m_face_size;
      m_faces.push_back({ first_pos, face_t{ 1, 0 } });
      std::queue<absolute_face_t> todo;
      todo.push({ first_pos, face_t{1, 0 } });
      while (!todo.empty()) {
        auto [cur_pos, cur_face] = todo.front();
        todo.pop();
        for (dir_t d : dir::all_dirs) {
          absolute_face_t new_face;
          new_face.root = cur_pos + rotate_vec({ 1, 0 }, d) * m_face_size;
          if (at(new_face.root) != cell_t::empty) {
            dir_t side = rotated_left(d, cur_face.rotation);
            new_face.face = face_next_to(cur_face.id, side);
            new_face.face.rotation = (new_face.face.rotation + cur_face.rotation) % 4;
            auto it = std::ranges::find_if(m_faces, [&](const absolute_face_t& f) { return f.root == new_face.root; });
            if (it == m_faces.end()) {
              m_faces.push_back(new_face);
              todo.push(new_face);
            } else {
              assert(*it == new_face);
            }
          }
        }
      }
    }

    cell_t operator[](const pos_t& p) const {
      return at(p);
    }

    cell_t at(const pos_t& p) const {
      if (p.y < 0 || p.y >= map_height()
        || p.x < 0 || p.x >= static_cast<dim_t>(m_map[p.y].size())) {
        return cell_t::empty;
      }
      switch (m_map[p.y][p.x]) {
      case '#': return cell_t::wall;
      case '.': return cell_t::path;
      default: return cell_t::empty;
      }
    }

    const face_t& face_at(const pos_t& p) const {
      pos_t root = { p.x - p.x % face_size(), p.y - p.y % face_size() };
      for (const absolute_face_t& f : m_faces) {
        if (f.root == root) {
          return f.face;
        }
      }
      assert(false);
      std::unreachable();
    }

    const absolute_face_t& face_absolute(face_id_t id) const {
      for (const absolute_face_t& face : m_faces) {
        if (face.face.id == id) {
          return face;
        }
      }
      assert(false);
      std::unreachable();
    }

    dim_t face_size() const { return m_face_size; }
    dim_t map_height() const { return static_cast<dim_t>(m_map.size()); }
    dim_t map_width() const { return m_map_width; }

  private:
    std::vector<std::string> m_map;
    dim_t m_map_width = 0;
    dim_t m_face_size = 0;

    std::vector<absolute_face_t> m_faces;
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

    void rotate_right() { dir = rotated_right(dir, 1); }
    void rotate_left() { dir = rotated_left(dir, 1); }

    void advance_wrap(const map_t& map, dim_t dist) {
      const vec_t dir_vec = rotate_vec({ 1, 0 }, dir);
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

    void advance_fold(const map_t& map, dim_t dist) {
      for (; dist > 0; --dist) {
        const vec_t dir_vec = rotate_vec({ 1, 0 }, dir);
        you_t next = *this;
        next.pos += dir_vec;
        if (map[next.pos] == cell_t::empty) {
          face_t cur_face = map.face_at(pos);
          dir_t localized_dir = rotated_left(dir, cur_face.rotation);
          face_t next_face = face_next_to(cur_face.id, localized_dir);
          next_face.rotation = rotated_right(next_face.rotation, cur_face.rotation);
          absolute_face_t absolute_face = map.face_absolute(next_face.id);
          auto pos_in_face = vec_t{ (next.pos.x + map.face_size()) % map.face_size(), (next.pos.y + map.face_size()) % map.face_size()};
          uint8_t rotation_diff = (absolute_face.face.rotation + 4 - next_face.rotation) % 4;
          switch (rotation_diff) {
          case 0: break;
          case 1: pos_in_face = { map.face_size() - 1 - pos_in_face.y, pos_in_face.x }; break;
          case 2: pos_in_face = { map.face_size() - 1 - pos_in_face.x, map.face_size() - 1 - pos_in_face.y }; break;
          case 3: pos_in_face = { pos_in_face.y, map.face_size() - 1 - pos_in_face.x }; break;
          default:
            assert(false);
            break;
          }
          next.pos = absolute_face.root + pos_in_face;
          next.dir = rotated_right(next.dir, rotation_diff);
        }
        assert(map[next.pos] != cell_t::empty);
        if (map[next.pos] == cell_t::wall) {
          break;
        }

        *this = next;
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
    auto visitor = action_visitor_t{ map_t(in, 50), advance };
    auto actions = action_stream_t(next_line(in));
    while (auto action = actions.next()) {
      std::visit(visitor, *action);
    }
    return std::to_string(visitor.you().password());
  }

  REGISTER_DAY("d22",
    [](std::istream& in) { return d22(in, &you_t::advance_wrap); },
    [](std::istream& in) { return d22(in, &you_t::advance_fold); }
  )

    TEST(d22, parsing) {

    EXPECT_EQ(face_next_to(6, dir::right), face_t(4, 1));
    EXPECT_EQ(face_next_to(6, dir::down), face_t(2, 0));
    EXPECT_EQ(face_next_to(6, dir::left), face_t(3, 0));
    EXPECT_EQ(face_next_to(6, dir::up), face_t(5, 2));
    EXPECT_EQ(face_next_to(5, dir::right), face_t(3, 3));
    EXPECT_EQ(face_next_to(5, dir::down), face_t(1, 0));
    EXPECT_EQ(face_next_to(5, dir::left), face_t(4, 2));
    EXPECT_EQ(face_next_to(4, dir::up), face_t(1, 3));
    EXPECT_EQ(face_next_to(4, dir::right), face_t(2, 2));
    EXPECT_EQ(face_next_to(3, dir::down), face_t(2, 1));
    EXPECT_EQ(face_next_to(3, dir::left), face_t(1, 0));
    EXPECT_EQ(face_next_to(2, dir::down), face_t(1, 2));

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
    auto map = map_t(in, 4);
    ASSERT_EQ(map.map_height(), 12);
    ASSERT_EQ(map.map_width(), 16);
    ASSERT_EQ(map.face_size(), 4);
    ASSERT_EQ(map.face_at(pos_t(8, 0)), face_t(1, 0));
    ASSERT_EQ(map.face_at(pos_t(8, 4)), face_t(2, 2));
    ASSERT_EQ(map.face_at(pos_t(4, 4)), face_t(4, 0));
    ASSERT_EQ(map.face_at(pos_t(0, 4)), face_t(5, 2));
    ASSERT_EQ(map.face_at(pos_t(8, 8)), face_t(6, 2));
    ASSERT_EQ(map.face_at(pos_t(12, 8)), face_t(3, 2));


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

    auto visitor = action_visitor_t(map_t(in, 4), &you_t::advance_wrap);
    for (const auto& [action, res] : scenario) {
      auto was = visitor.you();
      std::visit(visitor, action);
      ASSERT_EQ(visitor.you(), res) << "while executing " << action << " while being in " << was;
    }
    ASSERT_EQ(visitor.you().password(), 6032);
  }

  TEST(d22, part2) {
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
      { advance_t{5}, { dir::down, { 14, 10} } },
      { clockwise_t{}, { dir::left, { 14, 10} } },
      { advance_t{10}, { dir::left, { 10, 10} } },
      { counterclockwise_t{}, { dir::down, { 10, 10} } },
      { advance_t{4}, { dir::up, { 1, 5} } },
      { clockwise_t{}, { dir::right, { 1, 5} } },
      { advance_t{5}, { dir::right, { 6, 5} } },
      { counterclockwise_t{}, { dir::up, { 6, 5} } },
      { advance_t{5}, { dir::up, { 6, 4} } },
    };

    auto visitor = action_visitor_t(map_t(in, 4), &you_t::advance_fold);
    for (const auto& [action, res] : scenario) {
      auto was = visitor.you();
      std::visit(visitor, action);
      ASSERT_EQ(visitor.you(), res) << "while executing " << action << " while being in " << was;
    }
    ASSERT_EQ(visitor.you().password(), 5031);
  }
}