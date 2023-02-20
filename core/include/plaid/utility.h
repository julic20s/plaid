#pragma once
#ifndef PLAID_UTILITY_H_
#define PLAID_UTILITY_H_

#include <cstdint>

namespace plaid {

/// 2维偏移量
struct offset2d {
  std::int32_t x, y;
};

/// 定义矩形
struct extent2d {
  std::uint32_t width;
  std::uint32_t height;
};

/// 定义矩形区域
struct rect2d {
  offset2d offset;
  extent2d extent;
};

/// 透明句柄
enum struct handle : std::intptr_t {
  invalid = 0,
};

[[nodiscard]] inline handle wrap(const void *ptr) noexcept {
  return handle{reinterpret_cast<std::intptr_t>(ptr)};
}

template <class Ty>
[[nodiscard]] inline Ty *unwrap(handle h) noexcept {
  return reinterpret_cast<Ty *>(h);
}

[[nodiscard]] constexpr bool valid(handle h) noexcept {
  return h == handle::invalid;
}

} // namespace plaid

#endif // PLAID_UTILITY_H_
