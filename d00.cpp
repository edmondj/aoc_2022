#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>

// This is a test day, to make sure setup is alright

REGISTER_DAY("d00",
  [](std::istream& input) -> std::string {
    return read_all_file(input).value();
  }
);

TEST(d00, basic) {
  EXPECT_EQ(0, 0);
}