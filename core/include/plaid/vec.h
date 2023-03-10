#pragma once
#ifndef PLAID_VEC_H_
#define PLAID_VEC_H_

#include <cmath>

#include <type_traits>

// 二维向量
namespace plaid {

struct vec2 {

  [[nodiscard]] constexpr float &
  operator[](std::uint8_t i) noexcept { return *(&x + i); }

  [[nodiscard]] constexpr const float &
  operator[](std::uint8_t i) const noexcept {
    return *(&x + i);
  }

  float x, y;
};

[[nodiscard]] constexpr vec2 operator+(vec2 a, vec2 b) noexcept {
  return {a.x + b.x, a.y + b.y};
}

[[nodiscard]] constexpr vec2 operator-(vec2 a, vec2 b) noexcept {
  return {a.x - b.x, a.y - b.y};
}

[[nodiscard]] constexpr vec2 operator*(vec2 a, vec2 b) noexcept {
  return {a.x * b.x, a.y * b.y};
}

[[nodiscard]] constexpr vec2 operator/(vec2 a, vec2 b) noexcept {
  return {a.x / b.x, a.y / b.y};
}

[[nodiscard]] constexpr vec2 operator*(vec2 a, float b) noexcept {
  return {a.x * b, a.y * b};
}

[[nodiscard]] constexpr vec2 operator/(vec2 a, float b) noexcept {
  return {a.x / b, a.y / b};
}

[[nodiscard]] constexpr float cross(vec2 a, vec2 b) noexcept {
  return a.x * b.y - a.y * b.x;
}

[[nodiscard]] constexpr float dot(vec2 a, vec2 b) noexcept {
  return a.x * b.x + a.y * b.y;
}

[[nodiscard]] inline vec2 pow(vec2 a, float n) noexcept {
  return {std::pow(a.x, n), std::pow(a.y, n)};
}

} // namespace plaid

// 三维向量
namespace plaid {

struct vec3 {

  [[nodiscard]] constexpr float &
  operator[](std::uint8_t i) noexcept { return *(&x + i); }

  [[nodiscard]] constexpr const float &
  operator[](std::uint8_t i) const noexcept { return *(&x + i); }

  float x, y, z;
};

[[nodiscard]] constexpr vec3 operator+(vec3 a, vec3 b) noexcept {
  return {a.x + b.x, a.y + b.y, a.z + b.z};
}

[[nodiscard]] constexpr vec3 operator-(vec3 a, vec3 b) noexcept {
  return {a.x - b.x, a.y - b.y, a.z - b.z};
}

[[nodiscard]] constexpr vec3 operator*(vec3 a, vec3 b) noexcept {
  return {a.x * b.x, a.y * b.y, a.z * b.z};
}

[[nodiscard]] constexpr vec3 operator/(vec3 a, vec3 b) noexcept {
  return {a.x / b.x, a.y / b.y, a.z / b.z};
}

[[nodiscard]] constexpr vec3 operator*(vec3 a, float b) noexcept {
  return {a.x * b, a.y * b, a.z * b};
}

[[nodiscard]] constexpr vec3 operator/(vec3 a, float b) noexcept {
  return {a.x / b, a.y / b, a.z / b};
}

[[nodiscard]] constexpr vec3 cross(vec3 a, vec3 b) noexcept {
  return {
      a.y * b.z - a.z * b.y,
      a.z * b.x - a.x * b.z,
      a.x * b.y - a.y * b.x,
  };
}

[[nodiscard]] constexpr float dot(vec3 a, vec3 b) noexcept {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

[[nodiscard]] inline vec3 pow(vec3 a, float n) noexcept {
  return {std::pow(a.x, n), std::pow(a.y, n), std::pow(a.z, n)};
}

} // namespace plaid

// 四维向量
namespace plaid {

struct vec4 {
  [[nodiscard]] constexpr float &
  operator[](std::uint8_t i) noexcept { return *(&x + i); }

  [[nodiscard]] constexpr const float &
  operator[](std::uint8_t i) const noexcept { return *(&x + i); }

  float x, y, z, w;
};

[[nodiscard]] constexpr vec4 operator+(vec4 a, vec4 b) noexcept {
  return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

[[nodiscard]] constexpr vec4 operator-(vec4 a, vec4 b) noexcept {
  return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

[[nodiscard]] constexpr vec4 operator*(vec4 a, vec4 b) noexcept {
  return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

[[nodiscard]] constexpr vec4 operator/(vec4 a, vec4 b) noexcept {
  return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

[[nodiscard]] constexpr vec4 operator*(vec4 a, float b) noexcept {
  return {a.x * b, a.y * b, a.z * b, a.w * b};
}

[[nodiscard]] constexpr vec4 operator/(vec4 a, float b) noexcept {
  return {a.x / b, a.y / b, a.z / b, a.w / b};
}

[[nodiscard]] constexpr float dot(vec4 a, vec4 b) noexcept {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

[[nodiscard]] inline vec4 pow(vec4 a, float n) noexcept {
  return {std::pow(a.x, n), std::pow(a.y, n), std::pow(a.z, n), std::pow(a.w, n)};
}

} // namespace plaid

// 向量类工具实现
namespace plaid {

template <class Ty>
concept vec = std::is_same_v<std::remove_cv_t<Ty>, vec2> ||
              std::is_same_v<std::remove_cv_t<Ty>, vec3> ||
              std::is_same_v<std::remove_cv_t<Ty>, vec4>;

template <vec Ty>
[[nodiscard]] inline float abs(Ty v) noexcept {
  return std::sqrt(dot(v, v));
}

template <vec Ty>
[[nodiscard]] constexpr auto
operator*(float a, Ty b) noexcept {
  return b * a;
}

template <vec Ty>
[[nodiscard]] constexpr auto norm(Ty v) {
  return v / abs(v);
}

template <vec Ty>
[[nodiscard]] constexpr auto operator-(Ty v) {
  return v * -1;
}

} // namespace plaid

#endif // PLAID_VEC_H_
