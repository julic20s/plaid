#pragma once
#ifndef PLAID_RENDER_PASS_H_
#define PLAID_RENDER_PASS_H_

#include <cstddef>
#include <cstdint>

namespace plaid {
class graphics_pipeline_impl;
class graphics_pipeline;
} // namespace plaid

namespace plaid {

using attachment_reference = std::uint8_t;

struct subpass_description {
  std::uint16_t input_attachments_count;
  std::uint16_t color_attachments_count;
  const attachment_reference *input_attachments;
  const attachment_reference *color_attachments;
  const attachment_reference *depth_stencil_attachment;
};

class render_pass {
public:

  render_pass() : m_subpasses_count(0), m_subpasses(nullptr) {}

  explicit render_pass(std::uint32_t subpasses_count, const subpass_description *subpasses);

  render_pass(const render_pass &) = delete;

  render_pass(render_pass &&mov) noexcept {
    *this = static_cast<render_pass &&>(mov);
  }

  ~render_pass();

  render_pass &operator=(render_pass &&mov) noexcept {
    m_subpasses_count = mov.m_subpasses_count;
    m_subpasses = mov.m_subpasses;
    mov.m_subpasses_count = 0;
    mov.m_subpasses = nullptr;
    return *this;
  }

  /// 记录渲染通道状态
  class state;

  struct begin_info;

private:

  class subpass;

  std::uint32_t m_subpasses_count;
  subpass *m_subpasses;
};

struct begin_info {
  render_pass &render_pass;
};

class render_pass::state {
public:

  state(const render_pass &);

  /// 绑定描述符集
  void bind_descriptor_set(std::uint8_t binding, const std::byte *);

  /// 绑定顶点缓冲区
  void bind_vertex_buffer(std::uint8_t binding, const std::byte *);

  /// 根据当前渲染通道状态，绘制一帧
  void draw(
      graphics_pipeline_impl *,
      std::uint32_t vertex_count, std::uint32_t instance_count,
      std::uint32_t first_vertex, std::uint32_t first_instance
  );

  /// 移动状态到下一个渲染子通道
  void next_subpass();

private:

  const std::byte *m_descriptor_set[1 << 8];
  const std::byte *m_vertex_buffer[1 << 8];

  friend class graphics_pipeline_impl;
};

} // namespace plaid

#endif // PLAID_RENDER_PASS_H_
