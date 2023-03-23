#pragma once

#include <map>
#include <memory>
#include <span>
#include <vector>

#include <plaid/mat.h>

#include "../scene/camera.h"
#include "renderer.h"


namespace plaid::viewer::gltf {
class loader;
}

namespace plaid::viewer::gltf {

class scene {
public:
  /// 绘制场景
  void render();

  /// 获取相机节点集合
  std::span<const uint32_t> cameras();

  /// 切换到给定节点 id 所绑定的相机，
  void switch_camera(std::uint32_t id);

  std::span<const std::uint32_t> nodes;

private:
  /// 根节点编号
  renderer renderer_;
};

} // namespace plaid::viewer::gltf
