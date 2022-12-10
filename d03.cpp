#include "days.hpp"
#include "utils.hpp"
#include <array>
#include <ranges>
#include <string_view>
#include <algorithm>

uint64_t priority_of(char item) {
  if (item >= 'a' && item <= 'z') {
    return item - 'a' + 1;
  } else if (item >= 'A' && item <= 'Z') {
    return item - 'A' + 27;
  } else {
    throw std::runtime_error("Invalid item");
  }
}

struct rucksack_ref {
  std::array<std::string_view, 2> compartments;

  rucksack_ref(std::string_view item_list)
    : compartments{
      item_list.substr(0, item_list.size() / 2),
      item_list.substr(item_list.size() / 2)
  } {
  }

  char duplicate_item() const {
    return *std::ranges::find_if(compartments[0], [this](char item) {
      return ranges::contains(compartments[1], item);
      });
  }
};

REGISTER_DAY("d03",
  [](std::istream& input) {
    uint64_t sum = 0;
    for (std::string line; std::getline(input, line);) {
      sum += priority_of(rucksack_ref(line).duplicate_item());
    }
    return std::to_string(sum);
  },
  [](std::istream& input) {
    uint64_t sum = 0;
    size_t elf_id = 0;
    std::array<std::string, 3> group;
    for (std::string line; std::getline(input, line);) {
      group[elf_id] = std::move(line);
      if (++elf_id == 3) {
        char badge = *std::ranges::find_if(group[0], [&group](char item) {
          return ranges::contains(group[1], item)
          && ranges::contains(group[2], item);
        });
        sum += priority_of(badge);
        elf_id = 0;
      }
    }
    return std::to_string(sum);
  }
)