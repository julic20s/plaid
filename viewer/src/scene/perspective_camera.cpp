#include <plaid/transition.h>

#include "perspective_camera.h"

using namespace plaid::viewer;

camera::camera(
    const plaid::vec3 &position, const plaid::vec3 &orbit,
    float near, float far, float fovy, float ratio
) : orbit_(orbit),
    gaze_(orbit - position),
    near_(near),
    far_(far),
    dolly_(abs(gaze_)),
    fovy_(fovy),
    ratio_(ratio) {
  gaze_ = plaid::norm(gaze_);
}

plaid::mat4 camera::view() {
  auto s = side();
  auto up = plaid::norm(plaid::cross(s, gaze_));
  auto rotate = plaid::mat4{{
      {s.x, s.y, s.z, 0},
      {-up.x, -up.y, -up.z, 0},
      {gaze_.x, gaze_.y, gaze_.z, 0},
      {0, 0, 0, 1},
  }};
  auto pos = orbit_ - gaze_ * dolly_;
  return rotate * plaid::translate(-pos);
}

plaid::mat4 camera::projection() {
  auto h = near_ * std::tan(fovy_ / 2) * 2;
  auto w = h * ratio_;
  plaid::mat4 frustum{{
      {1, 0, 0, 0},
      {0, 1, 0, 0},
      {0, 0, near_ + far_, -(near_ * far_)},
      {0, 0, 1, 0},
  }};
  auto normalize = scale(vec3{2 / w, 2 / h, 1 / (far_ - near_)}) *
                   translate(vec3{0, 0, -near_});
  return normalize * frustum;
}

void camera::move_hor(float rad) {
  auto sinr = std::sin(rad), cosr = std::cos(rad);
  plaid::mat4 rotate{{
      {cosr, 0, sinr, 0},
      {0, 1, 0, 0},
      {-sinr, 0, cosr, 0},
      {0, 0, 0, 1},
  }};

  auto trans = plaid::translate(orbit_) * rotate * plaid::translate(-orbit_);

  auto next_gaze = trans * plaid::vec4{gaze_.x, gaze_.y, gaze_.z};
  gaze_ = {next_gaze.x, next_gaze.y, next_gaze.z};
}

void camera::move_vet(float rad) {
  auto s = side();
  auto up = plaid::norm(plaid::cross(s, gaze_));
  plaid::mat4 place{{
      {s.x, s.y, s.z, 0},
      {-up.x, -up.y, -up.z, 0},
      {gaze_.x, gaze_.y, gaze_.z, 0},
      {0, 0, 0, 1},
  }};
  auto sinr = std::sin(rad), cosr = std::cos(rad);
  plaid::mat4 rotate{{
      {1, 0, 0, 0},
      {0, cosr, -sinr, 0},
      {0, sinr, cosr, 0},
      {0, 0, 0, 1},
  }};
  auto reset = plaid::transpose(place);

  auto trans = plaid::translate(orbit_) *
               reset * rotate * place *
               plaid::translate(-orbit_);

  auto next_gaze = plaid::norm(trans * plaid::vec4{gaze_.x, gaze_.y, gaze_.z});
  if (next_gaze.x * gaze_.x < 0 && next_gaze.z * gaze_.z < 0) return;
  gaze_ = {next_gaze.x, next_gaze.y, next_gaze.z};
}
