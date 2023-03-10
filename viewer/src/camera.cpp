#include <plaid/transition.h>

#include "camera.h"

static plaid::vec3 calculate_top(plaid::vec3 g) {
  if (g.y) {
    auto s = g.y > 0 ? -1 : 1;
    return plaid::norm(plaid::vec3{g.x, dot(g, g) / -g.y + g.y, g.z} * -s);
  } else return {0, -1, 0};
}

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
  m_top = calculate_top(m_gaze);
  m_gaze = plaid::norm(m_gaze);
}

plaid::mat4x4 camera::view() {
  auto s = side();
  auto rotate = plaid::mat4x4{{
      {s.x, s.y, s.z, 0},
      {-m_top.x, -m_top.y, -m_top.z, 0},
      {m_gaze.x, m_gaze.y, m_gaze.z, 0},
      {0, 0, 0, 1},
  }};
  auto pos = m_orbit - m_gaze * m_dolly;
  return rotate * plaid::translate(-pos);
}

plaid::mat4x4 camera::projection() {
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
  auto sinr = std::sin(rad), cosr = std::cos(rad);
  plaid::mat4x4 rotate{{
      {cosr, 0, sinr, 0},
      {0, 1, 0, 0},
      {-sinr, 0, cosr, 0},
      {0, 0, 0, 1},
  }};

  auto trans = plaid::translate(m_orbit) * rotate * plaid::translate(-m_orbit);

  auto next_gaze = trans * plaid::vec4{m_gaze.x, m_gaze.y, m_gaze.z};
  m_gaze = {next_gaze.x, next_gaze.y, next_gaze.z};
  m_top = calculate_top(m_gaze);
}

void camera::move_vet(float rad) {
  auto sinr = std::sin(rad), cosr = std::cos(rad);
  auto s = side();
  plaid::mat4x4 place{{
      {s.x, s.y, s.z, 0},
      {-m_top.x, -m_top.y, -m_top.z, 0},
      {m_gaze.x, m_gaze.y, m_gaze.z, 0},
      {0, 0, 0, 1},
  }};
  plaid::mat4x4 rotate{{
      {1, 0, 0, 0},
      {0, cosr, -sinr, 0},
      {0, sinr, cosr, 0},
      {0, 0, 0, 1},
  }};
  plaid::mat4x4 reset = plaid::transpose(place);

  auto vad = place * reset;

  auto trans = plaid::translate(m_orbit) *
               // reset * rotate * place *
               plaid::translate(-m_orbit);

  auto next_gaze = trans * plaid::vec4{m_gaze.x, m_gaze.y, m_gaze.z};
  m_gaze = {next_gaze.x, next_gaze.y, next_gaze.z};
  m_top = calculate_top(m_gaze);
}
