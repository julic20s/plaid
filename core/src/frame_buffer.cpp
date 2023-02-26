#include <plaid/frame_buffer.h>

using namespace plaid;

frame_buffer::frame_buffer(
    std::uint16_t attachments_count,
    std::byte *attachments[],
    std::uint32_t width, std::uint32_t height
) {
  if (attachments_count) {
    m_attachments_count = attachments_count;
    m_attachments = new std::byte *[attachments_count];
    auto it = m_attachments, ed = it + attachments_count;
    for (; it != ed; ++it, ++attachments) {
      *it = *attachments;
    }
  } else {
    m_attachments_count = 0;
    m_attachments = nullptr;
  }

  m_width = width;
  m_height = height;
}

frame_buffer::frame_buffer(const frame_buffer &copy) {
  *this = copy;
}

frame_buffer::frame_buffer(frame_buffer &&mov) {
  *this = static_cast<frame_buffer &&>(mov);
}

frame_buffer &frame_buffer::operator=(const frame_buffer &copy) {
  auto cnt = copy.m_attachments_count;
  if (cnt) {
    m_attachments_count = cnt;
    m_attachments = new std::byte *[cnt];
    auto it = m_attachments, ed = it + cnt, cit = copy.m_attachments;
    for (; it != ed; ++it, ++cit) {
      *it = *cit;
    }
  } else {
    m_attachments_count = 0;
    m_attachments = nullptr;
  }
  m_width = copy.m_width;
  m_height = copy.m_height;
  return *this;
}

frame_buffer &frame_buffer::operator=(frame_buffer &&mov) {
  m_attachments_count = mov.m_attachments_count;
  m_attachments = mov.m_attachments;
  m_width = mov.m_width;
  m_height = mov.m_height;

  mov.m_attachments_count = 0;
  mov.m_attachments = nullptr;
  return *this;
}

frame_buffer::~frame_buffer() {
  if (m_attachments) {
    delete[] m_attachments;
  }
}
