#pragma once

#include <map>
#include <memory>
#include <string>

#include <plaid/mat.h>

#include <json/dom.h>

#include "node.h"
#include "scene.h"

namespace plaid::viewer::gltf {

class loader {

public:
  loader() noexcept;

  void load(const json::dom &, bool do_validate);

  /// 判断是否提供默认场景
  [[nodiscard]] bool has_default_scene() const noexcept;

  /// 获取默认场景
  [[nodiscard]] std::uint32_t default_scene() const;

  /// 获取场景总数
  [[nodiscard]] std::uint32_t scene_size() const;

  /// 加载场景
  [[nodiscard]] const scene &load_scene(std::uint32_t index);

  /// 根据 ID 查找节点
  [[nodiscard]] const node &find_node(std::uint32_t id);

private:
  /// 读取所有节点信息
  void read_nodes(const json::dom &);

  /// 读取所有场景信息
  void read_scenes(const json::dom &);

  std::unique_ptr<scene[]> scenes_;
  /// 场景根节点索引池
  std::unique_ptr<std::uint32_t[]> scenes_roots_pool_;

  /// 节点数据
  std::unique_ptr<node[]> nodes_;
  /// 子节点索引池
  std::unique_ptr<std::uint32_t[]> children_pool_;

  /// 默认场景的 id
  std::uint32_t def_scene_;
  /// 场景个数
  std::uint32_t scenes_size_;

  /// 根据节点索引查找相机
  std::map<std::uint32_t, camera> cameras_;

  /// bin 文件缓冲区
  std::map<std::string, std::byte *> buffers_;
};

} // namespace plaid::viewer::gltf
