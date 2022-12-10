#include "days.hpp"
#include <vector>

std::vector<day> g_days;

void register_day(const char* name, day_function part1, day_function part2)
{
  g_days.push_back(day{ name, part1, part2 });
}

std::span<const day> all_days()
{
  return g_days;
}
