#include "node.h"

using namespace plaid::viewer::gltf;

node &node::operator[](std::uint32_t index) noexcept {
  return nodes_pool_[children_[index]];
}

const node &node::operator[](std::uint32_t index) const noexcept {
  return nodes_pool_[children_[index]];
}

std::uint32_t node::size() const noexcept {
  return children_.size();
}
