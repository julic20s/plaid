#pragma once
#ifndef PLAID_RENDER_PASS_H_
#define PLAID_RENDER_PASS_H_

#include <cstddef>
#include <cstdint>

namespace plaid {

/// 描述附件规格
struct attachment_description {
};

/// 描述渲染子通道规格
struct subpass_description {
  /// 输入附件的数量
  std::uint16_t input_attachments_count;
  /// 颜色附件的数量
  std::uint16_t color_attachments_count;
  /// 输入附件描述数组
  const std::uint8_t *input_attachments;
  /// 颜色附件描述数组
  const std::uint8_t *color_attachments;
};

struct render_pass_impl;

/// 渲染通道句柄
class render_pass {
public:
  struct create_info {
    /// 附件数量
    std::uint16_t attachments_count;
    /// 子通道数量
    std::uint16_t subpass_count;
    /// 附件描述数组
    const attachment_description *attachments;
    /// 子通道描述数组
    const subpass_description *subpasses;
  };

  render_pass() noexcept : ptr(nullptr) {}

  explicit render_pass(const create_info &);

  render_pass(const render_pass &) = delete;

  render_pass(render_pass &&mov) noexcept {
    ptr = mov.ptr;
    mov.ptr = nullptr;
  }

  ~render_pass();

  render_pass &operator=(render_pass &&mov) noexcept {
    ptr = mov.ptr;
    mov.ptr = nullptr;
    return *this;
  }

  [[nodiscard]] inline operator render_pass_impl *() noexcept { return ptr; }

  /// 绑定管道附件
  void bind_attachment(std::uint8_t id, std::byte *);

private:
  render_pass_impl *ptr;
};

} // namespace plaid

#endif // PLAID_RENDER_PASS_H_
