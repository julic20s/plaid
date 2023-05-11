#pragma once
#ifndef PLAID_VIEWER_CAMERA_H_
#define PLAID_VIEWER_CAMERA_H_

#include <plaid/mat.h>

namespace plaid::viewer {

class camera {
public:
  camera() = default;

  camera(
      const plaid::vec3 &position, const plaid::vec3 &orbit,
      float near, float far, float fovy, float ratio
  );

  [[nodiscard]] plaid::mat4 view();

  [[nodiscard]] plaid::mat4 projection();

  [[nodiscard]] inline plaid::vec3 &obrit() noexcept { return orbit_; }
  [[nodiscard]] inline const plaid::vec3 &obrit() const noexcept { return orbit_; }

  [[nodiscard]] inline const plaid::vec3 &gaze() const noexcept { return gaze_; }

  [[nodiscard]] inline const plaid::vec3 side() const noexcept {
    // cross(Y, Z) == kX
    return plaid::norm(plaid::cross({0, 1, 0}, gaze_));
  }

  [[nodiscard]] inline float &near() noexcept { return near_; }
  [[nodiscard]] inline const float &near() const noexcept { return near_; }

  [[nodiscard]] inline float &far() noexcept { return far_; }
  [[nodiscard]] inline const float &far() const noexcept { return far_; }

  [[nodiscard]] inline float &dolly() noexcept { return dolly_; }
  [[nodiscard]] inline const float &dolly() const noexcept { return dolly_; }

  [[nodiscard]] inline float &fovy() noexcept { return fovy_; }
  [[nodiscard]] inline const float &fovy() const noexcept { return fovy_; }

  [[nodiscard]] inline float &ratio() noexcept { return ratio_; }
  [[nodiscard]] inline const float &ratio() const noexcept { return ratio_; }

  void move_hor(float rad);

  void move_vet(float rad);

private:

  /// 环绕点坐标
  plaid::vec3 orbit_;
  /// 相机坐标
  plaid::vec3 gaze_;
  /// 近平面距离
  float near_;
  /// 远平面距离
  float far_;
  /// 相机与环绕点的距离
  float dolly_;
  /// 相机Y轴可视角度
  float fovy_;
  /// 相机宽高比
  float ratio_;
};

} // namespace plaid::viewer

#endif
