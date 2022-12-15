#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>

namespace {

struct pos {
  size_t x, y;

  auto operator<=>(const pos&) const = default;

  pos operator+(const pos& p) const {
    return pos{ x + p.x, y + p.y };
  }
};

using elevation = char;

class elevation_map {
public:
  elevation_map(std::vector<std::string> elevations)
    : elevations(std::move(elevations))
  {}

  elevation_map() = default;

  const elevation& operator[](const pos& p) const {
    return elevations[p.y][p.x];
  }

  elevation& operator[](const pos& p){
    return elevations[p.y][p.x];
  }

  size_t height() const { return elevations.size(); }
  size_t width() const { return elevations.empty() ? 0 : elevations[0].size(); }

private:
  std::vector<std::string> elevations;
};

struct puzzle {
  elevation_map map;
  pos start;
  pos end;
};

puzzle parse_input(std::istream& input) {
  std::vector<std::string> lines;
  puzzle sent;

  for (std::string line; std::getline(input, line) && !line.empty(); ) {
    auto start_x = line.find('S');
    if (start_x != std::string::npos) {
      sent.start = pos{ start_x, lines.size() };
      line[start_x] = 'a';
    }
    auto end_x = line.find('E');
    if (end_x != std::string::npos) {
      sent.end = pos{ end_x, lines.size() };
      line[end_x] = 'z';
    }
    lines.push_back(std::move(line));
  }
  sent.map = elevation_map(std::move(lines));
  return sent;
}

template<std::predicate<const pos&> IsEnd, std::predicate<char /*cur*/, char /*target*/> IsValidMove>
size_t find_shortest_path(const pos& start, const elevation_map& map, IsEnd&& is_end, IsValidMove&& is_valid_move) {
  auto visited = std::set<pos>{ start };
  auto to_visit = std::vector<pos>{ {start} };

  for (size_t iter = 1; !to_visit.empty(); ++iter) {
    std::vector<pos> new_pos;

    for (const pos& cur : to_visit) {
      char cur_elevation = map[cur];
      auto test_position = [&](const pos& p) {
        if (!is_valid_move(cur_elevation, map[p])) {
          return false;
        }
        if (is_end(p)) {
          return true;
        }
        if (visited.insert(p).second) {
          new_pos.push_back(p);
        }
        return false;
      };

      if (cur.y > 0 && test_position(pos{ cur.x, cur.y - 1 })) {
        return iter;
      }
      if (cur.y < map.height() - 1 && test_position(pos{ cur.x, cur.y + 1 })) {
        return iter;
      }
      if (cur.x > 0 && test_position(pos{ cur.x - 1, cur.y })) {
        return iter;
      }
      if (cur.x < map.width() - 1 && test_position(pos{ cur.x + 1, cur.y })) {
        return iter;
      }
    }
    to_visit = std::move(new_pos);
  }
  throw std::runtime_error("no solution found");
}

size_t best_path_size(const puzzle& input) {
  return find_shortest_path(input.start, input.map,
    [&input](const pos& p) { return p == input.end; },
    [&input](char cur, char target) { return target <= cur + 1; }
  );
}

REGISTER_DAY("d12",
  [](std::istream& input) {
    return std::to_string(best_path_size(parse_input(input)));
  },
  [](std::istream& input) {
    auto puzzle = parse_input(input);
    size_t found = find_shortest_path(puzzle.end, puzzle.map,
      [&puzzle](const pos& p) { return puzzle.map[p] == 'a'; },
      [&puzzle](char cur, char target) { return target >= cur - 1; }
    );
    return std::to_string(found);
  }
)

TEST(d12, part1) {
  auto stream = std::istringstream(R"(
Sabqponm
abcryxxl
accszExk
acctuvwj
abdefghi
)");
  stream.get();
  EXPECT_EQ(best_path_size(parse_input(stream)), 31);
}
}