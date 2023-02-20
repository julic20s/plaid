#pragma once
#ifndef PLAID_RENDER_PASS_H_
#define PLAID_RENDER_PASS_H_

#include <cstdint>

#include "utility.h"

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

  constexpr render_pass() : h(handle::invalid) {}

  explicit render_pass(const create_info &);

  render_pass(const render_pass &) = delete;

  render_pass(render_pass &&mov) noexcept {
    h = mov.h;
    mov.h = handle::invalid;
  }

  render_pass &operator=(render_pass &&mov) noexcept {
    h = mov.h;
    mov.h = handle::invalid;
    return *this;
  }

  [[nodiscard]] inline handle handle() noexcept { return h; }

private:
  plaid::handle h;
};

} // namespace plaid

#endif // PLAID_RENDER_PASS_H_
