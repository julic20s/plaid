/// 所有矩阵均为行主序

#pragma once
#ifndef PLAID_MAT_H_
#define PLAID_MAT_H_

#include "vec.h"

namespace plaid {

struct mat3x3 {

  /// 获得指定的某一行
  /// @param i 行号
  [[nodiscard]] constexpr vec4 &
  operator[](std::uint8_t i) noexcept { return r[i]; }

  /// 获得指定的某一行
  /// @param i 行号
  [[nodiscard]] constexpr const vec4 &
  operator[](std::uint8_t i) const noexcept { return r[i]; }

  /// 行向量数组
  vec4 r[3];
};

/// 4 * 4 的矩阵，以行主序存储
struct mat4x4 {

  /// 获得指定的某一行
  /// @param i 行号
  [[nodiscard]] constexpr vec4 &
  operator[](std::uint8_t i) noexcept { return r[i]; }

  /// 获得指定的某一行
  /// @param i 行号
  [[nodiscard]] constexpr const vec4 &
  operator[](std::uint8_t i) const noexcept { return r[i]; }

  /// 行向量数组
  vec4 r[4];
};

/// 矩阵相加
/// @param a 左矩阵
/// @param b 右矩阵
[[nodiscard]] constexpr mat4x4
operator+(const mat4x4 &a, const mat4x4 &b) noexcept {
  mat4x4 res;
  for (int r = 0; r != 4; ++r) {
    for (int c = 0; c != 4; ++c) {
      res[r][c] = a[r][c] + b[r][c];
    }
  }
  return res;
}

/// 矩阵相减
/// @param a 左矩阵
/// @param b 右矩阵
[[nodiscard]] constexpr mat4x4
operator-(const mat4x4 &a, const mat4x4 &b) noexcept {
  mat4x4 res;
  for (int r = 0; r != 4; ++r) {
    for (int c = 0; c != 4; ++c) {
      res[r][c] = a[r][c] - b[r][c];
    }
  }
  return res;
}

/// 矩阵相乘
/// @param a 左矩阵
/// @param b 右矩阵
[[nodiscard]] constexpr mat4x4
operator*(const mat4x4 &a, const mat4x4 &b) noexcept {
  mat4x4 res;
  for (int r = 0; r != 4; ++r) {
    for (int c = 0; c != 4; ++c) {
      res[r][c] = 0;
      for (int k = 0; k != 4; ++k) {
        res[r][c] += a[r][k] * b[k][c];
      }
    }
  }
  return res;
}

/// 矩阵左乘到向量
/// @param a 左矩阵
/// @param b 右向量
[[nodiscard]] constexpr vec4
operator*(const mat4x4 &a, vec4 b) noexcept {
  return {
      a[0][0] * b.x + a[0][1] * b.y + a[0][2] * b.z + a[0][3] * b.w,
      a[1][0] * b.x + a[1][1] * b.y + a[1][2] * b.z + a[1][3] * b.w,
      a[2][0] * b.x + a[2][1] * b.y + a[2][2] * b.z + a[2][3] * b.w,
      a[3][0] * b.x + a[3][1] * b.y + a[3][2] * b.z + a[3][3] * b.w,
  };
}

[[nodiscard]] constexpr vec3
operator*(const mat3x3 &a, vec3 b) noexcept {
  return {
    a[0][0] * b.x + a[0][1] * b.y + a[0][2] * b.z,
    a[1][0] * b.x + a[1][1] * b.y + a[1][2] * b.z,
    a[2][0] * b.x + a[2][1] * b.y + a[2][2] * b.z,
  };
}

[[nodiscard]] constexpr mat4x4 transpose(const mat4x4 &m) {
  return {{
      {m[0][0], m[1][0], m[2][0], m[3][0]},
      {m[0][1], m[1][1], m[2][1], m[3][1]},
      {m[0][2], m[1][2], m[2][2], m[3][2]},
      {m[0][3], m[1][3], m[2][3], m[3][3]},
  }};
}
} // namespace plaid

#endif // PLAID_MAT_H_
