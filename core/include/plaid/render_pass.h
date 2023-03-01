#pragma once
#ifndef PLAID_RENDER_PASS_H_
#define PLAID_RENDER_PASS_H_

#include <cstddef>
#include <cstdint>

#include "format.h"

namespace plaid {
class graphics_pipeline_impl;
class graphics_pipeline;
class frame_buffer;
} // namespace plaid

namespace plaid {

struct attachment_reference {
  std::uint8_t id;
  format format;
};

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

  render_pass(render_pass &&mov) noexcept;

  ~render_pass();

  [[nodiscard]] const subpass_description &subpass(std::uint32_t index) const {
    return m_subpasses[index];
  }

  render_pass &operator=(render_pass &&mov);

  /// 记录渲染通道状态
  class state;

  struct begin_info;

private:
  std::uint32_t m_subpasses_count;
  subpass_description *m_subpasses;
};

struct render_pass::begin_info {
  render_pass &render_pass;
  frame_buffer &frame_buffer;
};

class render_pass::state {
public:

  state(const begin_info &);

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

  subpass_description *m_first_subpass;
  subpass_description *m_current_subpass;
  subpass_description *m_last_subpass;
  const std::byte *m_descriptor_set[1 << 8];
  const std::byte *m_vertex_buffer[1 << 8];
  frame_buffer *m_frame_buffer;

  friend class graphics_pipeline_impl;
};

} // namespace plaid

#endif // PLAID_RENDER_PASS_H_
