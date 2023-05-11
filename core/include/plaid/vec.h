#pragma once
#ifndef PLAID_VEC_H_
#define PLAID_VEC_H_

#include <cmath>

namespace plaid {

template <std::size_t N>
concept vec_dimen = 1 <= N && N <= 4;

template <class Tp, std::size_t N>
requires vec_dimen<N>
struct vec_storage;

template <class Tp>
struct vec_storage<Tp, 1> {
  Tp x;
};

template <class Tp>
struct vec_storage<Tp, 2> {
  Tp x, y;
};

template <class Tp>
struct vec_storage<Tp, 3> {
  Tp x, y, z;
};

template <class Tp>
struct vec_storage<Tp, 4> {
  Tp x, y, z, w;
};

template <class Ty, std::size_t N>
requires vec_dimen<N>
struct vec : vec_storage<Ty, N> {
  using vec_type = vec<Ty, N>;
  using value_type = Ty;
  using size_type = std::size_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using iterator = pointer;
  using const_iterator = const_pointer;

  static constexpr size_type dimension = N;

  [[nodiscard]] constexpr iterator
  begin() noexcept { return &this->x; }

  [[nodiscard]] constexpr iterator
  end() noexcept { return begin() + dimension; }

  [[nodiscard]] constexpr const_iterator
  begin() const noexcept { return &this->x; }

  [[nodiscard]] constexpr const_iterator
  end() const noexcept { return begin() + dimension; }

  [[nodiscard]] constexpr const_iterator
  cbegin() noexcept { return begin(); }

  [[nodiscard]] constexpr const_iterator
  cend() noexcept { return end(); }

  [[nodiscard]] constexpr reference
  operator[](size_type i) noexcept { return (&this->x)[i]; }

  [[nodiscard]] constexpr const_reference
  operator[](size_type i) const noexcept { return (&this->x)[i]; }
};

template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator+(const vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  vec<decltype(a.x + b.x), N> res;
  for (std::size_t i = 0; i != N; ++i) {
    res[i] = a[i] + b[i];
  }
  return res;
}

template <class LTp, class RTp, std::size_t N>
constexpr decltype(auto)
operator+=(vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  for (std::size_t i = 0; i != N; ++i) {
    a[i] += b[i];
  }
  return a;
}

template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator-(const vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  vec<decltype(a.x - b.x), N> res;
  for (std::size_t i = 0; i != N; ++i) {
    res[i] = a[i] - b[i];
  }
  return res;
}

template <class LTp, class RTp, std::size_t N>
constexpr decltype(auto)
operator-=(vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  for (std::size_t i = 0; i != N; ++i) {
    a[i] -= b[i];
  }
  return a;
}

template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator*(const vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  vec<decltype(a.x * b.x), N> res;
  for (std::size_t i = 0; i != N; ++i) {
    res[i] = a[i] * b[i];
  }
  return res;
}

template <class LTp, class RTp, std::size_t N>
constexpr decltype(auto)
operator*=(vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  for (std::size_t i = 0; i != N; ++i) {
    a[i] *= b[i];
  }
  return a;
}

template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator*(const vec<LTp, N> &a, const RTp &b) noexcept {
  vec<decltype(a.x * b), N> res;
  for (std::size_t i = 0; i != N; ++i) {
    res[i] = a[i] * b;
  }
  return res;
}

template <class LTp, class RTp, std::size_t N>
constexpr decltype(auto)
operator*=(vec<LTp, N> &a, const RTp &b) noexcept {
  for (std::size_t i = 0; i != N; ++i) {
    a[i] *= b;
  }
  return a;
}

template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator*(const LTp &a, const vec<RTp, N> &b) noexcept {
  return b * a;
}

template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator/(const vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  vec<decltype(a.x / b.x), N> res;
  for (std::size_t i = 0; i != N; ++i) {
    res[i] = a[i] / b[i];
  }
  return res;
}

template <class LTp, class RTp, std::size_t N>
constexpr decltype(auto)
operator/=(vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  for (std::size_t i = 0; i != N; ++i) {
    a[i] /= b[i];
  }
  return a;
}

template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator/(const vec<LTp, N> &a, const RTp &b) noexcept {
  vec<decltype(a.x / b), N> res;
  for (std::size_t i = 0; i != N; ++i) {
    res[i] = a[i] / b;
  }
  return res;
}

template <class LTp, class RTp, std::size_t N>
constexpr decltype(auto)
operator/=(vec<LTp, N> &a, const RTp &b) noexcept {
  for (std::size_t i = 0; i != N; ++i) {
    a[i] /= b;
  }
  return a;
}

template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator%(const vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  vec<decltype(a.x % b.x), N> res;
  for (std::size_t i = 0; i != N; ++i) {
    res[i] = a[i] % b[i];
  }
  return res;
}

template <class LTp, class RTp, std::size_t N>
constexpr decltype(auto)
operator%=(vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  for (std::size_t i = 0; i != N; ++i) {
    a[i] %= b[i];
  }
  return a;
}

template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator&(const vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  vec<decltype(a.x & b.x), N> res;
  for (std::size_t i = 0; i != N; ++i) {
    res[i] = a[i] & b[i];
  }
  return res;
}

template <class LTp, class RTp, std::size_t N>
constexpr decltype(auto)
operator&=(vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  for (std::size_t i = 0; i != N; ++i) {
    a[i] &= b[i];
  }
  return a;
}

template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator|(const vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  vec<decltype(a.x | b.x), N> res;
  for (std::size_t i = 0; i != N; ++i) {
    res[i] = a[i] | b[i];
  }
  return res;
}

template <class LTp, class RTp, std::size_t N>
constexpr decltype(auto)
operator|=(vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  for (std::size_t i = 0; i != N; ++i) {
    a[i] |= b[i];
  }
  return a;
}

template <class LTp, class RTp, std::size_t N>
[[nodiscard]] constexpr auto
operator^(const vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  vec<decltype(a.x ^ b.x), N> res;
  for (std::size_t i = 0; i != N; ++i) {
    res[i] = a[i] ^ b[i];
  }
  return res;
}

template <class LTp, class RTp, std::size_t N>
constexpr decltype(auto)
operator^=(vec<LTp, N> &a, const vec<RTp, N> &b) noexcept {
  for (std::size_t i = 0; i != N; ++i) {
    a[i] ^= b[i];
  }
  return a;
}

template <class Tp, std::size_t N>
[[nodiscard]] constexpr auto dot(const vec<Tp, N> &a, const vec<Tp, N> &b) noexcept {
  decltype(a.x * a.x) res = 0;
  for (auto i = 0; i != N; ++i) {
    res += a[i] * b[i];
  }
  return res;
}

template <class Tp, std::size_t N>
[[nodiscard]] vec<Tp, N> pow(const vec<Tp, N> &a, float n) noexcept {
  vec<Tp, N> res;
  for (auto i = 0; i != N; ++i) {
    res[i] = std::pow(a[i], n);
  }
}

template <class Tp, std::size_t N>
[[nodiscard]] auto abs(const vec<Tp, N> &v) noexcept {
  return std::sqrt(dot(v, v));
}

template <class Tp, std::size_t N>
[[nodiscard]] auto norm(const vec<Tp, N> &v) noexcept {
  return v / abs(v);
}

template <class Tp, std::size_t N>
[[nodiscard]] constexpr auto operator-(const vec<Tp, N> &v) {
  return v * -1;
}

} // namespace plaid

namespace plaid {

using vec2 = vec<float, 2>;

[[nodiscard]] constexpr float cross(const vec2 &a, const vec2 &b) noexcept {
  return a.x * b.y - a.y * b.x;
}

} // namespace plaid

namespace plaid {

using vec3 = vec<float, 3>;

[[nodiscard]] constexpr vec3 cross(const vec3 &a, const vec3 &b) noexcept {
  return {
      a.y * b.z - a.z * b.y,
      a.z * b.x - a.x * b.z,
      a.x * b.y - a.y * b.x,
  };
}

} // namespace plaid

namespace plaid {

using vec4 = vec<float, 4>;

} // namespace plaid

#endif // PLAID_VEC_H_
