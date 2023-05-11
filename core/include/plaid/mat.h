#pragma once
#ifndef PLAID_MAT_H_
#define PLAID_MAT_H_

#include "vec.h"

namespace plaid {

/// \brief N 行 M 列的矩阵
/// \tparam Tp 元素类型
/// \tparam N 行数
/// \tparam M 列数
template <class Tp, std::size_t N, std::size_t M>
requires vec_dimen<N> && vec_dimen<M>
struct mat {

  using colum_type = vec<Tp, M>;
  using value_type = colum_type::value_type;
  using size_type = colum_type::size_type;
  using reference = colum_type::reference;
  using const_reference = colum_type::const_reference;

  mat() = default;

private:
  /// \brief 为 constexpr 服务，目前的operator[]使用的是指针加偏移的方式，在 constexpr 中不支持赋值
  static constexpr Tp &vec_member_(vec<Tp, N> &v, std::size_t r) {
    switch (r) {
      case 0: return v.x;
      case 1: return v.y;
      case 2: return v.z;
      case 3: return v.w;

      /// ASSUME UNREACHABLE
      default: return v.w;
    }
  }

public:
  constexpr mat(const value_type (&list)[N * M]) noexcept {
    for (std::size_t r = 0; r != N; ++r) {
      for (std::size_t c = 0; c != M; ++c) {
        vec_member_(colums[c], r) = list[r * M + c];
      }
    }
  }

  constexpr mat(const value_type (&list)[N][M]) noexcept {
    for (std::size_t r = 0; r != N; ++r) {
      for (std::size_t c = 0; c != M; ++c) {
        vec_member_(colums[c], r) = list[r][c];
      }
    }
  }

  [[nodiscard]] constexpr reference
  operator()(size_type r, size_type c) noexcept {
    return colums[c][r];
  }

  [[nodiscard]] constexpr const_reference
  operator()(size_type r, size_type c) const noexcept {
    return colums[c][r];
  }

  vec<Tp, M> colums[N];
};

using mat2 = mat<float, 2, 2>;
using mat3 = mat<float, 3, 3>;
using mat4 = mat<float, 4, 4>;

/// 矩阵相加
/// @param a 左矩阵
/// @param b 右矩阵
template <class LTp, class RTp, std::size_t N, std::size_t M>
[[nodiscard]] constexpr auto
operator+(const mat<LTp, N, M> &a, const mat<RTp, N, M> &b) noexcept {
  mat<decltype(a.colums[0] + b.colums[0]), N, M> res;
  for (std::size_t c = 0; c != M; ++c) {
    res[c] = a.colums[c] + b.colums[c];
  }
  return res;
}

/// 矩阵相减
/// @param a 左矩阵
/// @param b 右矩阵
template <class LTp, class RTp, std::size_t N, std::size_t M>
[[nodiscard]] constexpr auto
operator-(const mat<LTp, N, M> &a, const mat<RTp, N, M> &b) noexcept {
  mat<decltype(a.colums[0] - b.colums[0]), N, M> res;
  for (std::size_t c = 0; c != M; ++c) {
    res[c] = a.colums[c] - b.colums[c];
  }
  return res;
}

/// 矩阵相乘
/// @param a 左矩阵
/// @param b 右矩阵
template <class LTp, class RTp, std::size_t N, std::size_t P, std::size_t M>
[[nodiscard]] constexpr auto
operator*(const mat<LTp, N, P> &a, const mat<RTp, P, M> &b) noexcept {
  mat<decltype(a(0, 0) * b(0, 0)), N, M> res;
  for (std::size_t c = 0; c != M; ++c) {
    for (std::size_t r = 0; r != N; ++r) {
      res(r, c) = 0;
      for (std::size_t k = 0; k != P; ++k) {
        res(r, c) += a(r, k) * b(k, c);
      }
    }
  }
  return res;
}

/// 矩阵左乘到列向量
/// @param a 左矩阵
/// @param b 右向量
template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator*(const mat<LTp, N, N> &a, const vec<RTp, N> &b) noexcept {
  vec<decltype(a(0, 0) * b[0]), N> res;
  std::size_t c = 0;
  for (auto &d : res) {
    d = 0;
    for (std::size_t k = 0; k != N; ++k) {
      d += a(c, k) * b[k];
    }
    ++c;
  }
  return res;
}

template <class Tp, std::size_t N>
[[nodiscard]] constexpr mat<Tp, N, N> transpose(const mat<Tp, N, N> &m) {
  mat<Tp, N, N> res;
  for (std::size_t r = 0; r != N; ++r) {
    for (std::size_t c = 0; c != N; ++c) {
      res(c, r) = m(r, c);
    }
  }
  return res;
}
} // namespace plaid

#endif // PLAID_MAT_H_
