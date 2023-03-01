#include <algorithm>

#include <plaid/frame_buffer.h>

using namespace plaid;

frame_buffer::frame_buffer(
    std::uint16_t attachments_count,
    const attachment attachments[],
    std::uint32_t width, std::uint32_t height
) {
  attachment *copied_arr = nullptr;
  if (attachments_count) {
    copied_arr = new std::byte *[attachments_count];
    std::copy_n(attachments, attachments_count, copied_arr);
  }

  m_attachments_count = attachments_count;
  m_attachments = copied_arr;
  m_width = width;
  m_height = height;
}

frame_buffer::frame_buffer(const frame_buffer &copy) {
  *this = copy;
}

frame_buffer::frame_buffer(frame_buffer &&mov) {
  m_attachments_count = mov.m_attachments_count;
  m_attachments = mov.m_attachments;
  m_width = mov.m_width;
  m_height = mov.m_height;

  mov.m_attachments_count = 0;
  mov.m_attachments = nullptr;
}

frame_buffer &frame_buffer::operator=(const frame_buffer &copy) {
  return *new (this) frame_buffer(copy.m_attachments_count, copy.m_attachments, copy.m_width, copy.m_height);
}

frame_buffer &frame_buffer::operator=(frame_buffer &&mov) {
  return *new (this) frame_buffer(static_cast<frame_buffer &&>(mov));
}

frame_buffer::~frame_buffer() {
  if (m_attachments) {
    delete[] m_attachments;
  }
}
