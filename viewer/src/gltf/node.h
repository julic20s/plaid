#pragma once

#include <span>

namespace plaid::viewer::gltf {

class node {

public:
  node() = default;

  node(std::span<const std::uint32_t> children, node *nodes_pool)
      : children_(children), nodes_pool_(nodes_pool) {}

  [[nodiscard]] const node &operator[](std::uint32_t index) const noexcept;
  [[nodiscard]] node &operator[](std::uint32_t index) noexcept;
  [[nodiscard]] std::uint32_t size() const noexcept;

private:
  std::span<const std::uint32_t> children_;
  node *nodes_pool_;
};

} // namespace plaid::viewer::gltf
