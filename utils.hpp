#pragma once

#include <variant>
#include <optional>
#include <fstream>
#include <ranges>
#include <numeric>

template<typename V, typename... F>
auto match(V&& v, F&&... f) {
  struct overload_t : public F... {
    overload_t(F&&... f) : F(f)... {}
    using F::operator()...;
  } overload(std::forward<F>(f)...);
  return std::visit(overload, std::forward<V>(v));
}

inline std::optional<std::string> read_all_file(std::istream& file) {
  std::optional<std::string> sent;
  if (file.seekg(0, std::ios_base::end))
  {
    size_t file_size = file.tellg();
    if (!file.seekg(0, std::ios_base::beg))
      return std::nullopt;
    sent.emplace().resize(file_size);
    file.read(sent->data(), file_size);
  }
  return sent;
}

inline std::optional<std::string> read_all_file(const std::string& filename) {
  auto file = std::ifstream(filename);
  return read_all_file(file);
}

namespace ranges {
  template<std::ranges::input_range R, class T>
  bool contains(R&& r, const T& value) {
    return std::ranges::find(r, value) != std::ranges::end(r);
  }

  template<std::ranges::input_range R>
  auto reduce(R&& r) {
    auto common = r | std::views::common;
    return std::reduce(common.begin(), common.end());
  }
}