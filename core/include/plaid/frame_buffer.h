#pragma once

#include <cstddef>
#include <cstdint>

namespace plaid {

struct frame_buffer {
public:

  frame_buffer() : frame_buffer(0, nullptr, 0, 0) {}

  explicit frame_buffer(
      std::uint16_t attachments_count,
      std::byte **attachments,
      std::uint32_t width, std::uint32_t height
  );

  frame_buffer(const frame_buffer &);

  frame_buffer(frame_buffer &&);

  ~frame_buffer();

  frame_buffer &operator=(const frame_buffer &);

  frame_buffer &operator=(frame_buffer &&);

  [[nodiscard]] std::uint32_t width() { return m_width; }

  [[nodiscard]] std::uint32_t height() { return m_height; }

  [[nodiscard]] std::byte *attachement(std::uint16_t id) {
    return m_attachments[id];
  }

private:

  std::uint16_t m_attachments_count;
  std::byte **m_attachments;
  std::uint32_t m_width;
  std::uint32_t m_height;
};

} // namespace plaid
