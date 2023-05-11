#pragma once
#ifndef PLAID_FORMAT_H_
#define PLAID_FORMAT_H_

#include <cstdint>

namespace plaid {

struct format_traits {
  /// 通道数 (1 ~ 4)，与每个通道值占的字节数的幂 (0 ~ 3)，占据低 4 bit
  [[nodiscard]] static constexpr std::uint8_t
  components_size(int comp, int pow) noexcept {
    return (comp - 1) | (pow << 2);
  }

  static constexpr std::uint8_t RGBA = 0x00;
  static constexpr std::uint8_t ARGB = 0x10;
  static constexpr std::uint8_t BGRA = 0x20;
  static constexpr std::uint8_t ABGR = 0x30;

  static constexpr std::uint8_t float_point = 1 << 6;
  static constexpr std::uint8_t signed_integer = 2 << 6;
  static constexpr std::uint8_t unsigned_integer = 3 << 6;
  static constexpr std::uint8_t type_mask = 3 << 6;
};

namespace {
using ft = format_traits;
}

// 标记像素格式，采用位掩码形式表示各个类型
enum class format : std::uint8_t {
  undefined = 0,
  R32f = ft::float_point | ft::RGBA | ft::components_size(1, 2),
  RG32f = ft::float_point | ft::RGBA | ft::components_size(2, 2),
  RGB32f = ft::float_point | ft::RGBA | ft::components_size(3, 2),
  RGBA32f = ft::float_point | ft::RGBA | ft::components_size(4, 2),
  RGBA32u = ft::unsigned_integer | ft::RGBA | ft::components_size(4, 2),
  BGRA8u = ft::unsigned_integer | ft::BGRA | ft::components_size(4, 0),
};

/// \brief 获取给定格式所占的字节数量
/// \param f 给定格式
/// \return 字节数量
[[nodiscard]] constexpr std::uint32_t format_size(format f) {
  auto num = static_cast<std::uint8_t>(f);
  auto components = (num & 3) + 1;
  auto pow = (num >> 2) & 3;
  return components << pow;
}

/// \brief 判断是否浮点型
/// \param f 给定格式
/// \return 如果是浮点型则返回 true
[[nodiscard]] constexpr bool is_float_format(format f) {
  auto tp = static_cast<std::uint8_t>(f) & ft::type_mask;
  return tp == ft::float_point;
}

/// \brief 判断是否有符号整型
/// \param f 给定格式
/// \return 如果是有符号整型则返回 true
[[nodiscard]] constexpr bool is_signed_integer_format(format f) {
  auto tp = static_cast<std::uint8_t>(f) & ft::type_mask;
  return tp == ft::signed_integer;
}

/// \brief 判断是否无符号整型
/// \param f 给定格式
/// \return 如果是无符号整型则返回 true
[[nodiscard]] constexpr bool is_unsigned_integer_format(format f) {
  auto tp = static_cast<std::uint8_t>(f) & ft::type_mask;
  return tp == ft::unsigned_integer;
}

} // namespace plaid

#endif
