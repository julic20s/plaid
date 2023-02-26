#pragma once

#include <cstddef>
#include <cstdint>

namespace plaid {

struct frame_buffer {
public:

  frame_buffer(
      std::uint16_t attachments_count,
      std::byte **attachments,
      std::uint32_t width, std::uint32_t height
  );

  ~frame_buffer();

private:

  std::uint16_t m_attachments_count;
  std::byte **m_attachments;
  std::uint32_t m_width;
  std::uint32_t m_height;
};

} // namespace plaid
