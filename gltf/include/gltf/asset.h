#pragma once

#include <string>

#include "version.h"

namespace gltf {

struct asset {
  struct version version;
  struct version min_version;
  std::string copyright;
  std::string generator;
};

} // namespace gltf
