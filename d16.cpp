#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <regex>

namespace d16 {
  struct room {
    std::string name;
    size_t pressure;
    std::vector<size_t> connections;

    auto operator<=>(const room&) const = default;
  };

  std::vector<room> parse_rooms(std::istream& input) {
    std::vector<room> sent;
    std::vector<std::vector<std::string>> connections;
    for (std::string line; std::getline(input, line);) {
      static const auto re = std::regex(R"(Valve (\w+) has flow rate=(\d+); tunnel(?:s?) lead(?:s?) to valve(?:s?) (.*))");
      static const auto delim = std::string(", ");
      std::smatch matches;
      if (!std::regex_match(line, matches, re)) {
        throw std::runtime_error("bad format");
      }
      sent.push_back(room{ .name = matches[1].str(), .pressure = std::stoull(matches[2].str()) });
      std::vector<std::string>& c = connections.emplace_back();
      for (auto subrange : matches[3].str() | std::views::split(delim)) {
        c.push_back(std::string{ subrange.begin(), subrange.end() });
      }
    }
    for (size_t i = 0; i < sent.size(); ++i) {
      for (const std::string& r : connections[i]) {
        sent[i].connections.push_back(std::ranges::find_if(sent, [&r](const room& room) { return room.name == r; }) - sent.begin());
      }
    }
    return sent;
  }

  size_t find_shortest_dist(size_t from, size_t to, const std::vector<room>& rooms) {
    auto cur = std::vector<size_t>{ from };
    auto visited = std::set<size_t>{ from };
    auto dist = size_t{ 1 };
    while (!cur.empty()) {
      auto new_cur = std::vector<size_t>{};
      for (size_t r : cur) {
        const room& room = rooms[r];
        for (size_t connection : room.connections) {
          if (connection == to) {
            return dist;
          }
          if (visited.insert(connection).second) {
            new_cur.push_back(connection);
          }
        }
      }
      cur = std::move(new_cur);
      ++dist;
    }
    throw std::runtime_error{ "not found" };
  }

  struct candidate {
    std::vector<size_t> path;
    size_t pressure;
  };

  std::vector<candidate> find_all_paths(const std::vector<size_t>& path, size_t acc_pressure, size_t time_left, const std::vector<room>& rooms, const std::vector<size_t>& relevant_rooms, const std::map<std::pair<size_t, size_t>, size_t>& shortest_dist) {
    std::vector<candidate> sent;
    for (size_t r : relevant_rooms) {
      size_t dist = shortest_dist.at({ path.back(), r}) + 1;
      if (dist < time_left) {
        std::vector<size_t> new_path = path;
        new_path.push_back(r);
        size_t new_time_left = time_left - dist;
        size_t pressure = acc_pressure + new_time_left * rooms[r].pressure;
        std::vector<size_t> new_relevant_rooms;
        new_relevant_rooms.reserve(relevant_rooms.size());
        for (size_t s : relevant_rooms) {
          if (s != r) {
            new_relevant_rooms.push_back(s);
          }
        }
        sent.push_back(candidate{ new_path, pressure });
        for (candidate& c : find_all_paths(new_path, pressure, new_time_left, rooms, new_relevant_rooms, shortest_dist)) {
          sent.push_back(std::move(c));
        }
      }
    }
    return sent;
  }

  bool are_exclusives(const std::vector<size_t>& l, const std::vector<size_t>& r) {
    return std::ranges::all_of(l | std::views::drop(1), [&r](size_t l) {
      return std::ranges::all_of(r | std::views::drop(1), [l](size_t r) {
          return l != r;
        });
    });
  }

  size_t maximize_pressure(const std::vector<room>& rooms, size_t time, bool with_elephant)
  {
    static const auto START_NAME = std::string("AA");
    size_t start = std::ranges::find(rooms, START_NAME, &room::name) - rooms.begin();
    
    assert(rooms[start].pressure == 0);

    std::vector<size_t> relevant_rooms;
    for (size_t r = 0; r < rooms.size(); ++r) {
      if (rooms[r].pressure != 0) {
        relevant_rooms.push_back(r);
      }
    }
    std::sort(relevant_rooms.begin(), relevant_rooms.end());

    std::map<std::pair<size_t, size_t>, size_t> shortest_dist;
    for (auto from = relevant_rooms.begin(); from != relevant_rooms.end(); ++from) {
      shortest_dist[{start, *from}] = find_shortest_dist(start, *from, rooms);
      for (auto to = from + 1; to != relevant_rooms.end(); ++to) {
        size_t dist = find_shortest_dist(*from, *to, rooms);
        shortest_dist[{ *from, * to }] = dist;
        shortest_dist[{ *to, *from }] = dist;
      }
    }

    auto all_candidates = find_all_paths({ start }, 0, time, rooms, relevant_rooms, shortest_dist);
    
    if (!with_elephant) {
      return std::ranges::max(all_candidates | std::views::transform(&candidate::pressure));
    }
    size_t max_pressure = 0;
    for (auto it = all_candidates.begin(); it != all_candidates.end(); ++it) {
      for (auto next = it + 1; next != all_candidates.end(); ++next) {
        if (are_exclusives(it->path, next->path)) {
          max_pressure = std::max(max_pressure, it->pressure + next->pressure);
        }
      }
    }
    return max_pressure;
  }

  REGISTER_DAY("d16",
    [](std::istream& in) {
      return std::to_string(maximize_pressure(parse_rooms(in), 30, false));
    },
    [](std::istream& in) {
      return std::to_string(maximize_pressure(parse_rooms(in), 26, true));
    }
  )

  TEST(d16, part1) {
    auto stream = std::istringstream(R"(
Valve AA has flow rate=0; tunnels lead to valves DD, II, BB
Valve BB has flow rate=13; tunnels lead to valves CC, AA
Valve CC has flow rate=2; tunnels lead to valves DD, BB
Valve DD has flow rate=20; tunnels lead to valves CC, AA, EE
Valve EE has flow rate=3; tunnels lead to valves FF, DD
Valve FF has flow rate=0; tunnels lead to valves EE, GG
Valve GG has flow rate=0; tunnels lead to valves FF, HH
Valve HH has flow rate=22; tunnel leads to valve GG
Valve II has flow rate=0; tunnels lead to valves AA, JJ
Valve JJ has flow rate=21; tunnel leads to valve II
)");
    stream.get();
    auto rooms = parse_rooms(stream);
    auto pressure = maximize_pressure(rooms, 30, false);
    EXPECT_EQ(pressure, 1651);
  }

  TEST(d16, part2) {
    auto stream = std::istringstream(R"(
Valve AA has flow rate=0; tunnels lead to valves DD, II, BB
Valve BB has flow rate=13; tunnels lead to valves CC, AA
Valve CC has flow rate=2; tunnels lead to valves DD, BB
Valve DD has flow rate=20; tunnels lead to valves CC, AA, EE
Valve EE has flow rate=3; tunnels lead to valves FF, DD
Valve FF has flow rate=0; tunnels lead to valves EE, GG
Valve GG has flow rate=0; tunnels lead to valves FF, HH
Valve HH has flow rate=22; tunnel leads to valve GG
Valve II has flow rate=0; tunnels lead to valves AA, JJ
Valve JJ has flow rate=21; tunnel leads to valve II
)");
    stream.get();
    auto rooms = parse_rooms(stream);
    auto pressure = maximize_pressure(rooms, 26, true);
    EXPECT_EQ(pressure, 1707);
  }
}