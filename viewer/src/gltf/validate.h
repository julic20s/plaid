#pragma once

#include <stdexcept>

#include <json/dom.h>

namespace plaid::viewer::gltf {

void validate(const json::dom &gltf);

struct format_error : std::runtime_error {
  template <class... Args>
  format_error(Args &&...args)
      : std::runtime_error(std::forward<Args>(args)...) {}
};

} // namespace plaid::viewer::gltf
