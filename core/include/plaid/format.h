#pragma once
#ifndef PLAID_FORMAT_H_
#define PLAID_FORMAT_H_

#include <cstdint>

namespace plaid {

// 标记像素格式，采用位掩码形式表示各个类型
enum class format : std::uint16_t {
  undefined = 0x000,
  R32f = 0x014,
  RG32f = 0x024,
  RGB32f = 0x034,
  RGBA32f = 0x044,
  RGBA32u = 0x144,
  BGRA8u = 0x141,
};

/// 获取给定格式所占的字节数量
[[nodiscard]] inline std::uint32_t format_size(format f) {
  auto num = static_cast<std::uint16_t>(f);
  return (num & 0xf) * ((num & 0xf0) >> 4);
}

/// 判断是否浮点型
[[nodiscard]] inline bool is_float_format(format f) {
  return (static_cast<std::uint16_t>(f) & 0xf00) == 0;
}

/// 判断是否无符号整型
[[nodiscard]] inline bool is_unsigned_integer_format(format f) {
  return (static_cast<std::uint16_t>(f) & 0xf00) == 0x100;
}

} // namespace plaid

#endif
