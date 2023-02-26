#include <plaid/frame_buffer.h>

using namespace plaid;

frame_buffer::frame_buffer(
    std::uint16_t attachments_count,
    std::byte **attachments,
    std::uint32_t width, std::uint32_t height
) {
  m_attachments_count = attachments_count;
  m_attachments = new std::byte *[attachments_count];
  for (auto it = m_attachments, ed = it + attachments_count; it != ed; ++it, ++attachments) {
    *it = *attachments;
  }
  m_width = width;
  m_height = height;
}

frame_buffer::~frame_buffer() {
  delete[] m_attachments;
}
