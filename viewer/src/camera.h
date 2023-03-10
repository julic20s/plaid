#pragma once
#ifndef PLAID_VIEWER_CAMERA_H_
#define PLAID_VIEWER_CAMERA_H_

#include <plaid/mat.h>

class camera {
public:

  camera(
    const plaid::vec3 &position, const plaid::vec3 &orbit,
    float near, float far, float fovy, float ratio
  );

  [[nodiscard]] plaid::mat4x4 create_view();

  [[nodiscard]] plaid::mat4x4 create_projection();

  [[nodiscard]] inline float &near() noexcept { return m_near; }
  [[nodiscard]] inline const float &near() const noexcept { return m_near; }

  [[nodiscard]] inline float &far() noexcept { return m_far; }
  [[nodiscard]] inline const float &far() const noexcept { return m_far; }

  [[nodiscard]] inline float &dolly() noexcept { return m_dolly; }
  [[nodiscard]] inline const float &dolly() const noexcept { return m_dolly; }

  [[nodiscard]] inline float &fovy() noexcept { return m_fovy; }
  [[nodiscard]] inline const float &fovy() const noexcept { return m_fovy; }

  [[nodiscard]] inline float &ratio() noexcept { return m_ratio; }
  [[nodiscard]] inline const float &ratio() const noexcept { return m_ratio; }

  void move_hor(float rad);

private:

  /// 环绕点坐标
  plaid::vec3 m_orbit;
  /// 相机拍摄方向矢量，归一化
  plaid::vec3 m_gaze;
  /// 相机顶部方向矢量，归一化
  plaid::vec3 m_top;
  /// 近平面距离
  float m_near;
  /// 远平面距离
  float m_far;
  /// 相机与环绕点的距离
  float m_dolly;
  /// 相机Y轴可视角度
  float m_fovy;
  /// 相机宽高比
  float m_ratio;
};

#endif
