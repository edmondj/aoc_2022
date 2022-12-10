#include "days.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <stack>
#include <algorithm>

struct file {
  std::string name;
  size_t size;

  auto operator<=>(const file&) const = default;
};

struct dir {
  std::string name;
  std::vector<file> files;
  std::vector<dir> subdirs;

  auto operator<=>(const dir&) const = default;
};

dir parse_input(std::istream& input) {
  dir root{ "/" };
  std::stack<dir*> parents;
  dir* cur = &root;

  for (std::string line; std::getline(input, line) && !line.empty();) {
    if (line[0] == '$') {
      auto cmd = std::string_view(line).substr(2, 2);
      if (cmd == "ls") {
        continue;
      } else if (cmd == "cd") {
        auto path = std::string_view(line).substr(5);
        if (path == "/") {
          parents = {};
          cur = &root;
        } else if (path == "..") {
          if (!parents.empty()) {
            cur = parents.top();
            parents.pop();
          }
        } else {
          parents.push(cur);
          auto found = std::ranges::find_if(cur->subdirs, [&](const dir& d) { return d.name == path; });
          if (found == cur->subdirs.end()) {
            throw std::runtime_error("subdir not found");
          }
          cur = &*found;
        }
      } else {
        throw std::runtime_error("invalid command");
      }
    } else {
      auto space_index = line.find(' ');
      auto type = line.substr(0, space_index);
      auto name = line.substr(space_index + 1);
      if (type == "dir") {
        cur->subdirs.push_back({ std::move(name) });
      } else {
        cur->files.push_back({ std::move(name), std::stoull(type) });
      }
    }
  }
  return root;
}

struct dir_size {
  const dir* dir;
  size_t size;
  std::vector<dir_size> subdir;
};

dir_size compute_dir_size(const dir& d) {
  auto sent = dir_size{
    .dir = &d,
    .size = ranges::reduce(d.files | std::views::transform(&file::size))
  };

  for (const dir& sub : d.subdirs) {
    auto& sub_size = sent.subdir.emplace_back(compute_dir_size(sub));
    sent.size += sub_size.size;
  }
  return sent;
}

template<typename TDir, typename TFunc>
void for_each_dir(const TDir& dir, const TFunc& func) {
  func(dir);
  for (const TDir& sub : dir.subdir) {
    for_each_dir(sub, func);
  }
}

size_t dir_size_under_100000(const dir& root) {
  size_t total = 0;
  dir_size sizes = compute_dir_size(root);
  for_each_dir(sizes, [&](const dir_size& d) {
    if (d.size <= 100000) {
      total += d.size;
    }
  });
  return total;
}

REGISTER_DAY("d07",
  [](std::istream& input) {
    auto d = parse_input(input);
    return std::to_string(dir_size_under_100000(d));
  },
  [](std::istream& input) {
    const auto d = parse_input(input);
    const auto sizes = compute_dir_size(d);
    const auto to_free = 30000000 - (70000000 - sizes.size);
    size_t min_size = 70000000;
    for_each_dir(sizes, [&](const dir_size& d) {
      if (d.size >= to_free) {
        min_size = std::min(min_size, d.size);
      }
    });
    return std::to_string(min_size);
  }
)

TEST(d07, parse) {
    auto stream = std::istringstream(R"(
$ cd /
$ ls
dir a
14848514 b.txt
8504156 c.dat
dir d
$ cd a
$ ls
dir e
29116 f
2557 g
62596 h.lst
$ cd e
$ ls
584 i
$ cd ..
$ cd ..
$ cd d
$ ls
4060174 j
8033020 d.log
5626152 d.ext
7214296 k
)");
    stream.get();
    auto d = parse_input(stream);
    auto target = dir{
      .name = "/",
      .files = {
        { "b.txt", 14848514 },
        { "c.dat", 8504156 },
      },
      .subdirs = {
        dir{
          .name = "a",
          .files = {
            { "f", 29116 },
            { "g", 2557 },
            { "h.lst", 62596 }
          },
          .subdirs = {
            {
              .name = "e",
              .files = {
                { "i", 584 }
              },
              .subdirs = {}
            }
          }
        },
        dir{
          .name = "d",
          .files = {
            { "j", 4060174 },
            { "d.log", 8033020 },
            { "d.ext", 5626152 },
            { "k", 7214296 },
          },
          .subdirs = {}
        },
      }
    };

    EXPECT_EQ(d, target);
}

TEST(d07, part1) {
  auto stream = std::istringstream(R"(
$ cd /
$ ls
dir a
14848514 b.txt
8504156 c.dat
dir d
$ cd a
$ ls
dir e
29116 f
2557 g
62596 h.lst
$ cd e
$ ls
584 i
$ cd ..
$ cd ..
$ cd d
$ ls
4060174 j
8033020 d.log
5626152 d.ext
7214296 k
)");
  stream.get();
  auto d = parse_input(stream);

  EXPECT_EQ(dir_size_under_100000(d), 95437);
}