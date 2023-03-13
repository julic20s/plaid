#pragma once
#ifndef GLTF_HEADER_H_
#define GLTF_HEADER_H_

#include <cstdint>
#include <istream>

namespace gltf {

struct header {
  struct asset {
    struct version {
      std::uint8_t mojar;
      std::uint8_t minor;
    };

    version version;

  };

  static bool load(header &, std::istream &);
};

} // namespace gltf

#endif
