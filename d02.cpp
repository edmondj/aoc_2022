#include "days.hpp"
#include <gtest/gtest.h>

enum class Shape {
  Rock, Paper, Scissors
};

uint64_t score_of(Shape s) {
  switch (s) {
  case Shape::Rock: return 1;
  case Shape::Paper: return 2;
  case Shape::Scissors: return 3;
  }
  throw std::runtime_error("unreachable");
}

Shape winner_of(Shape s) {
  switch (s) {
  case Shape::Rock: return Shape::Paper;
  case Shape::Paper: return Shape::Scissors;
  case Shape::Scissors: return Shape::Rock;
  }
  throw std::runtime_error("unreachable");
}

Shape loser_of(Shape s) {
  switch (s) {
  case Shape::Rock: return Shape::Scissors;
  case Shape::Paper: return Shape::Rock;
  case Shape::Scissors: return Shape::Paper;
  }
  throw std::runtime_error("unreachable");
}

Shape get_opponent_shape(char c) {
  switch (c) {
  case 'A': return Shape::Rock;
  case 'B': return Shape::Paper;
  case 'C': return Shape::Scissors;
  default:
    throw std::runtime_error("Unknown shape");
  }
}

Shape get_my_shape(char c) {
  switch (c) {
  case 'X': return Shape::Rock;
  case 'Y': return Shape::Paper;
  case 'Z': return Shape::Scissors;
  default:
    throw std::runtime_error("Unknown shape");
  }
}

REGISTER_DAY("d02",
  [](std::istream& input) {
    size_t total_score = 0;
    for (std::string line; std::getline(input, line);) {
      if (!line.empty()) {
        Shape opponent = get_opponent_shape(line[0]);
        Shape my = get_my_shape(line[2]);
        total_score += score_of(my);
        if (opponent == my) {
          total_score += 3;
        }
        else if (my == winner_of(opponent)) {
          total_score += 6;
        }
      }
    }
    return std::to_string(total_score);
  },
  [](std::istream& input) {
    size_t total_score = 0;
    for (std::string line; std::getline(input, line);) {
      if (!line.empty()) {
        Shape opponent = get_opponent_shape(line[0]);
        Shape my;
        switch (line[2]) {
        case 'X': my = loser_of(opponent); break;
        case 'Y': my = opponent; break;
        case 'Z': my = winner_of(opponent); break;
        default: throw std::runtime_error("Unkonwn end");
        }
        total_score += score_of(my);
        if (opponent == my) {
          total_score += 3;
        }
        else if (my == winner_of(opponent)) {
          total_score += 6;
        }
      }
    }
    return std::to_string(total_score);
  }
);