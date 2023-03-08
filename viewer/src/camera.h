#pragma once
#ifndef PLAID_VIEWER_CAMERA_H_
#define PLAID_VIEWER_CAMERA_H_

#include <plaid/mat.h>

class camera {
public:

  camera(
    const plaid::vec3 &position, const plaid::vec3 &orbit,
    float depth_range, float fovy, float ratio
  );

  [[nodiscard]] plaid::mat4x4 create_view();

  [[nodiscard]] plaid::mat4x4 create_projection();

  [[nodiscard]] inline float &depth_range() noexcept { return m_depth_range; }
  [[nodiscard]] inline const float &depth_range() const noexcept { return m_depth_range; }

  [[nodiscard]] inline float &dolly() noexcept { return m_dolly; }
  [[nodiscard]] inline const float &dolly() const noexcept { return m_dolly; }

  [[nodiscard]] inline float &fovy() noexcept { return m_fovy; }
  [[nodiscard]] inline const float &fovy() const noexcept { return m_fovy; }

  [[nodiscard]] inline float &ratio() noexcept { return m_ratio; }
  [[nodiscard]] inline const float &ratio() const noexcept { return m_ratio; }

private:

  plaid::vec3 m_position;
  plaid::vec3 m_gaze;
  plaid::vec3 m_top;
  float m_depth_range;
  float m_dolly;
  float m_fovy;
  float m_ratio;
};

#endif
