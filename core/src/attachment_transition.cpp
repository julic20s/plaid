#include "attachment_transition.h"

using namespace plaid;

void plaid::RGB32f_to_BGRA8u(const std::byte *src, std::byte *dst) {
  auto final_color = reinterpret_cast<const float *>(src);
  auto r = std::uint32_t(final_color[0] * 0xff);
  auto g = std::uint32_t(final_color[1] * 0xff);
  auto b = std::uint32_t(final_color[2] * 0xff);
  *reinterpret_cast<std::uint32_t *>(dst) = r << 16 | g << 8 | b;
}

void plaid::RGBA32f_to_BGRA8u(const std::byte *src, std::byte *dst) {
  auto final_color = reinterpret_cast<const float *>(src);
  auto r = std::uint32_t(final_color[0] * 0xff);
  auto g = std::uint32_t(final_color[1] * 0xff);
  auto b = std::uint32_t(final_color[2] * 0xff);
  auto a = std::uint32_t(final_color[3] * 0xff);
  *reinterpret_cast<std::uint32_t *>(dst) = a << 24 | r << 16 | g << 8 | b;
}

void plaid::RGBA32u_to_BGRA8u(const std::byte *src, std::byte *dst) {
  auto final_color = *reinterpret_cast<const std::uint32_t *>(src);
  auto r = (final_color & 0x000000ff);
  auto g = (final_color & 0x0000ff00) >> 8;
  auto b = (final_color & 0x00ff0000) >> 16;
  auto a = (final_color & 0xff000000) >> 24;
  *reinterpret_cast<std::uint32_t *>(dst) = a << 24 | r << 16 | g << 8 | b;
}

attachment_transition_function *plaid::match_attachment_transition_function(format src, format dst) {
  if (src == format::RGB32f) {
    if (dst == format::BGRA8u) {
      return RGB32f_to_BGRA8u;
    }
  } else if (src == format::RGBA32f) {
    if (dst == format::BGRA8u) {
      return RGBA32f_to_BGRA8u;
    }
  } else if (src == format::RGBA32u) {
    if (dst == format::BGRA8u) {
      return RGBA32u_to_BGRA8u;
    }
  }
  return nullptr;
}
