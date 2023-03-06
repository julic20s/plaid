#pragma once

#include <cstddef>
#include <cstdint>

namespace plaid {

/// 记录一个帧缓冲区，一个帧缓冲区可以包含多个附件
/// 它会持有全部附件内存的指针，但不具有所有权
struct frame_buffer {
public:

  /// 附件内存指针
  using attachment = std::byte *;

  /// 创建一个空的帧缓冲区
  frame_buffer() : frame_buffer(0, nullptr, 0, 0) {}

  /// 创建一个帧缓冲区
  /// @param attachments_count 附件的总数目
  /// @param attachments 所有附件的指针数组
  /// @param width 帧缓冲区的宽度
  /// @param height 帧缓冲区的高度
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

  /// 获取帧缓冲区的宽度
  /// @return 当前缓冲区的宽度
  [[nodiscard]] inline std::uint32_t width() const noexcept {
    return m_width;
  }

  /// 获取帧缓冲区的高度
  /// @return 当前缓冲区的高度
  [[nodiscard]] inline std::uint32_t height() const noexcept {
    return m_height;
  }

  /// 获取指定 ID 附件对应的指针
  /// @param id 附件 ID
  /// @return 附件内存指针
  [[nodiscard]] inline attachment attachement(std::uint8_t id) const {
    return m_attachments[id];
  }

private:

  std::uint16_t m_attachments_count;
  const attachment *m_attachments;
  std::uint32_t m_width;
  std::uint32_t m_height;
};

} // namespace plaid
