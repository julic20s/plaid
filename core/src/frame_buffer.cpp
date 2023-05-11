#include <algorithm>

#include <plaid/frame_buffer.h>

using namespace plaid;

frame_buffer::frame_buffer(
    std::uint8_t attachments_count,
    const attachment attachments[],
    std::uint32_t width, std::uint32_t height
) {
  attachment *copied_arr = nullptr;
  if (attachments_count) {
    copied_arr = new std::byte *[attachments_count];
    std::copy_n(attachments, attachments_count, copied_arr);
  }

  attachments_count_ = attachments_count;
  addresses_ = copied_arr;
  width_ = width;
  height_ = height;
}

frame_buffer::frame_buffer(const frame_buffer &copy) {
  *this = copy;
}

frame_buffer::frame_buffer(frame_buffer &&mov) noexcept {
  attachments_count_ = mov.attachments_count_;
  addresses_ = mov.addresses_;
  width_ = mov.width_;
  height_ = mov.height_;

  mov.attachments_count_ = 0;
  mov.addresses_ = nullptr;
}

frame_buffer &frame_buffer::operator=(const frame_buffer &copy) {
  if (&copy == this) {
    return *this;
  }
  return *new (this) frame_buffer(copy.attachments_count_, copy.addresses_, copy.width_, copy.height_);
}

frame_buffer &frame_buffer::operator=(frame_buffer &&mov) noexcept {
  if (&mov == this) {
    return *this;
  }
  return *new (this) frame_buffer(static_cast<frame_buffer &&>(mov));
}

frame_buffer::~frame_buffer() {
  if (addresses_) {
    delete[] addresses_;
  }
}
