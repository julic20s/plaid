#include "camera.h"

camera::camera(
    const plaid::vec3 &position, const plaid::vec3 &orbit,
    float depth_range, float fovy, float ratio
) : m_position(position),
    m_gaze(orbit - position),
    m_depth_range(depth_range),
    m_dolly(abs(m_gaze)),
    m_fovy(fovy),
    m_ratio(ratio) {
  if (m_gaze.y) {
    auto dir = m_gaze.y > 0 ? -1 : 1;
    m_top = plaid::vec3{m_gaze.x, m_dolly / m_gaze.y * m_dolly + m_gaze.y, m_gaze.z} * dir;
  } else {
    m_top = {0, -1, 0};
  }
  m_gaze = plaid::norm(m_gaze);
}

plaid::mat4x4 camera::create_view() {
  auto s = plaid::norm(plaid::cross(m_gaze, m_top));
  auto rotate = plaid::mat4x4{{
      {s.x, s.y, s.z, 0},
      {-m_top.x, -m_top.y, -m_top.z, 0},
      {m_gaze.x, m_gaze.y, m_gaze.z, 0},
      {0, 0, 0, 1},
  }};
  auto translate = plaid::mat4x4{{
      {1, 0, 0, -m_position.x},
      {0, 1, 0, -m_position.y},
      {0, 0, 1, -m_position.z},
      {0, 0, 0, 1},
  }};
  return rotate * translate;
}

plaid::mat4x4 camera::create_projection() {
  auto n = m_dolly - m_depth_range / 2;
  auto f = n + m_depth_range;
  auto h = n * std::tan(m_fovy / 2) * 2;
  auto w = h * m_ratio;
  auto frustum = plaid::mat4x4{{
      {1, 0, 0, 0},
      {0, 1, 0, 0},
      {0, 0, n + f, -(n * f)},
      {0, 0, 1, 0},
  }};
  auto normalize = plaid::mat4x4{{
      {2 / w, 0, 0, 0},
      {0, 2 / h, 0, 0},
      {0, 0, 2 / m_depth_range, 0},
      {0, 0, 0, 1},
  }} * plaid::mat4x4{{
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, -m_dolly},
    {0, 0, 0, 1},
  }};
  return normalize * frustum;
}
