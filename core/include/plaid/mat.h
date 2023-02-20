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

private:
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

} // namespace plaid

#endif // PLAID_MAT_H_
