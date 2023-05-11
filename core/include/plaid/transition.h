#pragma once
#ifndef PLAID_TRANSITION_H_
#define PLAID_TRANSITION_H_

#include "mat.h"

namespace plaid {

template <class Tp>
[[nodiscard]] constexpr mat<Tp, 4, 4>
translate(const vec<Tp, 3> &dist) noexcept {
  return {{
      {1, 0, 0, dist.x},
      {0, 1, 0, dist.y},
      {0, 0, 1, dist.z},
      {0, 0, 0, 1},
  }};
}

template <class Tp>
[[nodiscard]] constexpr mat<Tp, 4, 4>
scale(const vec<Tp, 3> &factor) noexcept {
  return {{
      {factor.x, 0, 0, 0},
      {0, factor.y, 0, 0},
      {0, 0, factor.z, 0},
      {0, 0, 0, 1},
  }};
}

template <class Tp>
[[nodiscard]] constexpr mat<Tp, 4, 4>
scale(const Tp &factor) noexcept {
  return scale(vec<Tp, 3>{factor, factor, factor});
}

} // namespace plaid

#endif
