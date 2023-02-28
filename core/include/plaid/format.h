#pragma once
#ifndef PLAID_FORMAT_H_
#define PLAID_FORMAT_H_

#include <cstdint>

namespace plaid {
enum class format : std::uint16_t {
  undefined = 0x0,
  rg3232_float = 0x024,
  rgb323232_float = 0x034,
  rgba32323232_float = 0x044,
  rgb888_integer = 0x141,
};

[[nodiscard]] inline std::uint32_t format_size(format f) {
  auto num = static_cast<std::uint16_t>(f);
  return (num & 0xf) * ((num & 0xf0) >> 4);
}

}

#endif
