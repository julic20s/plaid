#pragma once
#ifndef PLAID_FRAME_BUFFER_H_
#define PLAID_FRAME_BUFFER_H_

#include <cstddef>
#include <cstdint>

namespace plaid {

/// \brief 记录一个帧缓冲区，一个帧缓冲区可以包含多个附件，它会持有全部附件内存的指针
struct frame_buffer {
public:
  /// 附件内存指针
  using attachment = std::byte *;

  frame_buffer() = default;

  /// \brief 创建一个帧缓冲区
  /// \param attachments_count 附件的总数目
  /// \param attachments 所有附件的指针数组
  /// \param width 帧缓冲区的宽度
  /// \param height 帧缓冲区的高度
  frame_buffer(
      std::uint8_t attachments_count,
      const attachment attachments[],
      std::uint32_t width,
      std::uint32_t height
  );

  frame_buffer(const frame_buffer &);

  frame_buffer(frame_buffer &&) noexcept;

  ~frame_buffer();

  frame_buffer &operator=(const frame_buffer &);

  frame_buffer &operator=(frame_buffer &&) noexcept;

  /// 获取指定 ID 附件对应的指针
  /// @param id 附件 ID
  /// @return 附件内存指针
  [[nodiscard]] constexpr attachment
  operator[](std::uint8_t i) const {
    return addresses_[i];
  }

  [[nodiscard]] constexpr std::uint32_t
  width() const noexcept { return width_; }

  [[nodiscard]] constexpr std::uint32_t
  height() const noexcept { return height_; }

private:
  std::uint8_t attachments_count_;
  std::uint32_t width_;
  std::uint32_t height_;
  const attachment *addresses_;
};

} // namespace plaid

#endif
