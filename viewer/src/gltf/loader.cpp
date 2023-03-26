#include <set>

#include "loader.h"
#include "validate.h"

using namespace plaid::json;
using namespace plaid::viewer::gltf;

static constexpr auto no_default_scene = ~std::uint32_t(0);

loader::loader() noexcept
    : def_scene_(no_default_scene),
      scenes_size_(0) {}

void loader::load(const dom &gltf, bool do_validate) {
  if (do_validate) {
    validate(gltf);
  }
  read_scenes(gltf);
  read_nodes(gltf);
  read_buffers(gltf);
}

void loader::read_nodes(const dom &gltf) {
  auto nodes_json = gltf["nodes"];
  auto nodes_cnt = nodes_json.size();
  std::uint32_t children_pool_size = 0;
  for (auto &node : nodes_json.to_value_span()) {
    children_pool_size += node["children"].size();
  }
  /// 为节点申请内存
  nodes_ = std::make_unique<node[]>(nodes_cnt);
  /// 为每个节点的孩子数组申请内存
  children_pool_ = std::make_unique<std::uint32_t[]>(children_pool_size);

  auto node_it = nodes_.get();
  auto children_pool_it = children_pool_.get();
  for (auto &node_json : nodes_json.to_value_span()) {
    auto children_json = node_json["children"];
    /// 初始化节点
    new (node_it++) class node(std::span{children_pool_it, children_json.size()}, nodes_.get());
    /// 更新孩子数组
    for (auto &child_json : children_json.to_value_span()) {
      *children_pool_it = child_json.to_double();
      ++children_pool_it;
    }
  }
}

void loader::read_scenes(const dom &gltf) {
  auto def_scene_json = gltf["scene"];
  auto scenes_json = gltf["scenes"];
  if (def_scene_json != nullval) {
    def_scene_ = def_scene_json.to_double();
  } else {
    def_scene_ = no_default_scene;
  }
  scenes_size_ = scenes_json.size();

  // 为场景申请内存
  scenes_ = std::make_unique<scene[]>(scenes_size_);

  std::uint32_t scenes_roots_pools_size = 0;
  for (auto &sc_json : scenes_json.to_value_span()) {
    scenes_roots_pools_size += sc_json["nodes"].size();
  }

  // 为场景根节点池申请内存
  scenes_roots_pool_ = std::make_unique<std::uint32_t[]>(scenes_roots_pools_size);

  // 场景对象的迭代器
  auto scene_it = scenes_.get();
  // 场景根节点组的迭代器
  auto sc_root_it = scenes_roots_pool_.get();
  for (auto &sc_json : scenes_json.to_value_span()) {
    auto nodes_json = sc_json["nodes"];
    scene_it->nodes = {sc_root_it, nodes_json.size()};
    for (auto &root_json : nodes_json.to_value_span()) {
      *sc_root_it = root_json.to_double();
      ++sc_root_it;
    }
    ++scene_it;
  }
}

void loader::read_buffers(const dom &gltf) {
  auto buffers_json = gltf["buffers"];
  buffers_ = std::make_unique<buffer[]>(buffers_json.size());
  auto it = buffers_.get();
  for (auto &buf_json : buffers_json.to_value_span()) {
    it->byte_length = buf_json["byteLength"].to_double();
    auto &uri_json = buf_json["uri"];
    if (uri_json != nullval) {
      it->uri = uri_json.to_string_view();
    }
    ++it;
  }
}

bool loader::has_default_scene() const noexcept {
  return def_scene_ != no_default_scene;
}

const scene &loader::load_scene(std::uint32_t index) {
  if (index >= scene_size()) {
    throw std::out_of_range("Illegal index of scene.");
  }

  // TODO: load buffers if need

  return scenes_[index];
}


