// aos_2022_cpp.cpp : Defines the entry point for the application.
//

#include "days.hpp"
#include "utils.hpp"
#include <optional>
#include <fstream>
#include <iostream>
#include <gtest/gtest.h>

struct launch_option {
  struct help {};
  struct run {
    std::string_view day;
  };
  struct test {
    std::string_view day;
  };
  std::variant<help, run, test> mode;
};

launch_option parse_args(int ac, const char** av) {
  launch_option sent = { .mode = launch_option::help{} };
  if (ac == 3) {
    if (strcmp(av[1], "run") == 0) {
      sent.mode = launch_option::run{ av[2] };
    }
    else if (strcmp(av[1], "test") == 0) {
      sent.mode = launch_option::test{ av[2] };
    }
  }
  return sent;
}

const char* exec_name(const char* arg) {
  size_t offset = std::strlen(arg) - 1;
  while (offset > 0 && arg[offset - 1] != '/' && arg[offset - 1] != '\\') {
    --offset;
  }
  return &arg[offset];
}


int main(int ac, const char** av) {
  match(parse_args(ac, av).mode,
    [exec = av[0]](launch_option::help) { std::cout << "Usage: " << exec_name(exec) << " run|test <day>\n"; },
    [](launch_option::run run) {
      std::cout << "Running " << run.day << ":\n";
      for (const day& d : all_days()) {
        if (d.name == run.day) {
          auto filename = std::string{ "input/" }.append(run.day) + ".txt";
          auto file = std::ifstream(filename);
          if (!file) {
            std::cout << "NO INPUT\n";
          } else {
            if (!d.part1) {
              std::cout << "NOT IMPLEMENTED\n";
            } else {
              auto start = std::chrono::steady_clock::now();
              auto res = d.part1(file);
              auto end = std::chrono::steady_clock::now();
              std::cout << "Part 1: " << res << '\n';
              std::cout << "  found in " << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start).count() << "ms\n";
              if (d.part2) {
                file.clear();
                file.seekg(0, std::ios_base::beg);
                start = std::chrono::steady_clock::now();
                res = d.part2(file);
                end = std::chrono::steady_clock::now();
                std::cout << "Part 2: " << res << '\n';
                std::cout << "  found in " << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start).count() << "ms\n";
              }
            }
          }
          return;
        }
      }
      std::cout << "DAY NOT FOUND\n";
    },
    [](launch_option::test test) {
      ::testing::InitGoogleTest();
      ::testing::GTEST_FLAG(filter) = std::string{ test.day } + "*";
      RUN_ALL_TESTS();
    }
  );
}
