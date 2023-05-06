#pragma once

#include <charconv>
#include <string_view>
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
  template<std::ranges::input_range R, class T, class Proj = std::identity>
  bool contains(R&& r, const T& value, Proj proj = {}) {
    return std::ranges::find(r, value, proj) != std::ranges::end(r);
  }

  template<std::ranges::input_range R>
  auto reduce(R&& r) {
    auto common = r | std::views::common;
    return std::reduce(common.begin(), common.end());
  }
}

template<typename T>
T string_view_to(std::string_view s) {
  T sent{};
  auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), sent);
  if (ec != std::errc()) {
    throw std::runtime_error("parse error");
  }
  return sent;
}

template<std::signed_integral T>
T sign_of(T v) {
  if (v > 0) {
    return 1;
  }
  else if (v == 0) {
    return 0;
  }
  return -1;
}