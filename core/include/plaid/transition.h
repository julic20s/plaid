#pragma once
#ifndef PLAID_TRANSITION_H_
#define PLAID_TRANSITION_H_

#include "mat.h"

namespace plaid {

[[nodiscard]] constexpr mat4x4 translate(vec3 dist) noexcept {
  return {{
      {1, 0, 0, dist.x},
      {0, 1, 0, dist.y},
      {0, 0, 1, dist.z},
      {0, 0, 0, 1},
  }};
}

}

#endif
