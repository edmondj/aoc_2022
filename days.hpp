#pragma once

#include <iostream>
#include <string>
#include <span>

using day_function = std::string(*)(std::istream&);

struct day {
  const char* name;
  day_function part1;
  day_function part2;
};

void register_day(const char* name, day_function part1 = nullptr, day_function part2 = nullptr);
std::span<const day> all_days();

#define REGISTER_DAY(...) namespace { static const struct auto_register_t { auto_register_t() { register_day(__VA_ARGS__); } } auto_register; }