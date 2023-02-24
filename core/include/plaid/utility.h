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

} // namespace plaid

#endif // PLAID_UTILITY_H_
