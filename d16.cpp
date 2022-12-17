#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <regex>

namespace d16 {
  struct room {
    size_t pressure;
    std::vector<std::string> connections;

    auto operator<=>(const room&) const = default;
  };

  std::pair<std::string, room> parse_room(const std::string& line) {
    static const auto re = std::regex(R"(Valve (\w+) has flow rate=(\d+); tunnel(?:s?) lead(?:s?) to valve(?:s?) (.*))");
    static const auto delim = std::string(", ");
    std::smatch matches;
    if (!std::regex_match(line, matches, re)) {
      throw std::runtime_error("bad format");
    }
    std::vector<std::string> connections;
    for (auto subrange : matches[3].str() | std::views::split(delim)) {
      connections.push_back(std::string{ subrange.begin(), subrange.end() });
    }
    return {
      matches[1].str(),
      room{
        .pressure = std::stoull(matches[2].str()),
        .connections = std::move(connections)
      }
    };
  }

  std::map<std::string, room> parse_rooms(std::istream& input) {
    std::map<std::string, room> sent;
    for (std::string line; std::getline(input, line);) {
      sent.insert(parse_room(line));
    }
    return sent;
  }

  size_t find_shortest_dist(const std::string& from, const std::string& to, const std::map<std::string, room>& rooms) {
    auto cur = std::vector<std::string>{ from };
    auto visited = std::set<std::string>{ from };
    auto dist = size_t{ 1 };
    while (!cur.empty()) {
      auto new_cur = std::vector<std::string>{};
      for (const std::string& r : cur) {
        const room& room = rooms.at(r);
        for (const std::string& connection : room.connections) {
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

  size_t find_max_pressure(const std::string& cur, const std::map<std::string, room>& rooms, const std::vector<std::string>& relevant_rooms, const std::map<std::pair<std::string, std::string>, size_t>& shortest_dist, size_t time) {
    size_t max_pressure = 0;
    std::vector<std::string> leftovers = relevant_rooms;

    for (const std::string& room : relevant_rooms) {
      size_t dist = shortest_dist.at({ cur, room }) + 1;
      if (dist >= time) {
        continue;
      }
      size_t new_time = time - dist;
      std::vector<std::string> filtered_rooms;
      for (const std::string& s : relevant_rooms) {
        if (s != room) {
          filtered_rooms.push_back(s);
        }
      }
      size_t candidate_pressure = find_max_pressure(room, rooms, filtered_rooms, shortest_dist, new_time);
      candidate_pressure += rooms.at(room).pressure * new_time;
      if (candidate_pressure > max_pressure) {
        max_pressure = candidate_pressure;
      }
    }
    return max_pressure;
  }

  size_t maximize_pressure(const std::map<std::string, room>& rooms, size_t time, size_t actors)
  {
    static const auto START = std::string("AA");
    
    assert(rooms.at(START).pressure == 0);

    std::vector<std::string> relevant_rooms;
    for (const auto& [name, room] : rooms) {
      if (room.pressure != 0) {
        relevant_rooms.push_back(name);
      }
    }
    std::sort(relevant_rooms.begin(), relevant_rooms.end());

    std::map<std::pair<std::string, std::string>, size_t> shortest_dist;
    for (auto from = relevant_rooms.begin(); from != relevant_rooms.end(); ++from) {
      shortest_dist[{START, *from}] = find_shortest_dist(START, *from, rooms);
      for (auto to = from + 1; to != relevant_rooms.end(); ++to) {
        size_t dist = find_shortest_dist(*from, *to, rooms);
        shortest_dist[{ *from, * to }] = dist;
        shortest_dist[{ *to, *from }] = dist;
      }
    }

    (void)actors;
    return find_max_pressure(START, rooms, relevant_rooms, shortest_dist, time);
  }

  REGISTER_DAY("d16",
    [](std::istream& in) {
      return std::to_string(maximize_pressure(parse_rooms(in), 30, 1));
    }
  )

  TEST(d16, parse) {
    auto stream = std::istringstream(R"(
Valve AA has flow rate=0; tunnels lead to valves DD, II, BB
Valve HH has flow rate=22; tunnel leads to valve GG
)");
    stream.get();
    auto parsed = parse_rooms(stream);
    auto target = std::map<std::string, room>{
      { "AA", room{ .pressure = 0, .connections = { "DD", "II", "BB" } } },
      { "HH", room{ .pressure = 22, .connections = { "GG" } } }
    };
    EXPECT_EQ(parsed, target);
  }

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
    auto pressure = maximize_pressure(rooms, 30, 1);
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
    auto pressure = maximize_pressure(rooms, 26, 2);
    EXPECT_EQ(pressure, 1707);
  }
}