#include "camera.h"

camera::camera(
    const plaid::vec3 &position, const plaid::vec3 &orbit,
    float near, float far, float fovy, float ratio
) : m_orbit(orbit),
    m_gaze(orbit - position),
    m_near(near),
    m_far(far),
    m_dolly(abs(m_gaze)),
    m_fovy(fovy),
    m_ratio(ratio) {
  if (m_gaze.y) {
    auto dir = m_gaze.y > 0 ? -1 : 1;
    m_top = plaid::vec3{m_gaze.x, (m_dolly * m_dolly / abs(m_gaze.y)) * dir + m_gaze.y, m_gaze.z} * -dir;
    m_top = plaid::norm(m_top);
  } else {
    m_top = {0, -1, 0};
  }
  m_gaze = plaid::norm(m_gaze);
}

plaid::mat4x4 camera::create_view() {
  auto pos = m_orbit - m_gaze * m_dolly;
  auto side = plaid::norm(plaid::cross(m_gaze, m_top));
  auto rotate = plaid::mat4x4{{
      {side.x, side.y, side.z, 0},
      {-m_top.x, -m_top.y, -m_top.z, 0},
      {m_gaze.x, m_gaze.y, m_gaze.z, 0},
      {0, 0, 0, 1},
  }};
  auto translate = plaid::mat4x4{{
      {1, 0, 0, -pos.x},
      {0, 1, 0, -pos.y},
      {0, 0, 1, -pos.z},
      {0, 0, 0, 1},
  }};
  return rotate * translate;
}

plaid::mat4x4 camera::create_projection() {
  auto h = m_near * std::tan(m_fovy / 2) * 2;
  auto w = h * m_ratio;
  auto frustum = plaid::mat4x4{{
      {1, 0, 0, 0},
      {0, 1, 0, 0},
      {0, 0, m_near + m_far, -(m_near * m_far)},
      {0, 0, 1, 0},
  }};
  auto normalize = plaid::mat4x4{{
                       {2 / w, 0, 0, 0},
                       {0, 2 / h, 0, 0},
                       {0, 0, 1 / (m_far - m_near), 0},
                       {0, 0, 0, 1},
                   }} *
                   plaid::mat4x4{{
                       {1, 0, 0, 0},
                       {0, 1, 0, 0},
                       {0, 0, 1, -m_near},
                       {0, 0, 0, 1},
                   }};
  return normalize * frustum;
}

void camera::move_hor(float rad) {
  auto cs = std::cos(rad);
  auto s = std::sin(rad);
  auto rotate = plaid::mat4x4{{
                    {1, 0, 0, -m_orbit.x},
                    {0, 1, 0, -m_orbit.y},
                    {0, 0, 1, -m_orbit.z},
                    {0, 0, 0, 1},
                }} *
                plaid::mat4x4{{
                    {cs, 0, s, 0},
                    {0, 1, 0, 0},
                    {-s, 0, cs, 0},
                    {0, 0, 0, 1},
                }} *
                plaid::mat4x4{{
                    {1, 0, 0, m_orbit.x},
                    {0, 1, 0, m_orbit.y},
                    {0, 0, 1, m_orbit.z},
                    {0, 0, 0, 1},
                }};

  auto next_gaze = rotate * plaid::vec4{m_gaze.x, m_gaze.y, m_gaze.z};
  auto next_top = rotate * plaid::vec4{m_top.x, m_top.y, m_top.z};
  m_gaze = {next_gaze.x, next_gaze.y, next_gaze.z};
  m_top = {next_top.x, next_top.y, next_top.z};
}
