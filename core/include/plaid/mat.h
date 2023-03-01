/// 所有矩阵均为行主序

#pragma once
#ifndef PLAID_MAT_H_
#define PLAID_MAT_H_

#include "vec.h"

namespace plaid {

class mat4x4 {
public:
  [[nodiscard]] constexpr vec4 &
  operator[](std::uint8_t i) noexcept { return r[i]; }

  [[nodiscard]] constexpr const vec4 &
  operator[](std::uint8_t i) const noexcept { return r[i]; }

  vec4 r[4];
};

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

[[nodiscard]] constexpr vec4
operator*(const mat4x4 &a, vec4 b) noexcept {
  return {
      a[0][0] * b.x + a[0][1] * b.y + a[0][2] * b.z + a[0][3] * b.w,
      a[1][0] * b.x + a[1][1] * b.y + a[1][2] * b.z + a[1][3] * b.w,
      a[2][0] * b.x + a[2][1] * b.y + a[2][2] * b.z + a[2][3] * b.w,
      a[3][0] * b.x + a[3][1] * b.y + a[3][2] * b.z + a[3][3] * b.w,
  };
}

} // namespace plaid

#endif // PLAID_MAT_H_
