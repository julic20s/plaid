#pragma once

#include <cstddef>
#include <cstdint>

namespace plaid {

/// 记录一个帧缓冲区，一个帧缓冲区可以包含多个附件
struct frame_buffer {
public:

  using attachment = std::byte *;

  frame_buffer() : frame_buffer(0, nullptr, 0, 0) {}

  explicit frame_buffer(
      std::uint16_t attachments_count,
      const attachment attachments[],
      std::uint32_t width,
      std::uint32_t height
  );

  frame_buffer(const frame_buffer &);

  frame_buffer(frame_buffer &&);

  ~frame_buffer();

  frame_buffer &operator=(const frame_buffer &);

  frame_buffer &operator=(frame_buffer &&);

  [[nodiscard]] inline std::uint32_t width() { return m_width; }

  [[nodiscard]] inline std::uint32_t height() { return m_height; }

  [[nodiscard]] inline std::byte *attachement(std::uint8_t id) { return m_attachments[id]; }

private:

  std::uint16_t m_attachments_count;
  const attachment *m_attachments;
  std::uint32_t m_width;
  std::uint32_t m_height;
};

} // namespace plaid
