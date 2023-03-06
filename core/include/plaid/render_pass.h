#pragma once
#ifndef PLAID_RENDER_PASS_H_
#define PLAID_RENDER_PASS_H_

#include <cstddef>
#include <cstdint>

#include "format.h"

namespace plaid {
class frame_buffer;
class graphics_pipeline_impl;
} // namespace plaid

namespace plaid {

/// 附件引用，它不关心附件的具体内存，只关心附件在帧缓冲区的编号，具体的内存位置由帧缓冲区确定
struct attachment_reference {
  /// 指明附件在帧缓冲区中的编号
  std::uint8_t id;
  /// 指明附件格式
  format format;
};

/// 描述一个渲染子通道
struct subpass_description {
  /// 指明输入附件的总数目
  std::uint16_t input_attachments_count;
  /// 指明颜色附件的总数目
  std::uint16_t color_attachments_count;
  /// 输入附件数组
  const attachment_reference *input_attachments;
  /// 颜色附件数组
  const attachment_reference *color_attachments;
  /// 深度附件
  const attachment_reference *depth_stencil_attachment;
};

/// 记录一个多通道渲染
class render_pass {
public:

  /// 创建一个空的多通道渲染
  render_pass() : m_subpasses_count(0), m_subpasses(nullptr) {}

  /// 根据参数创建多通道渲染
  /// @param subpasses_count 子通道总数目
  /// @param subpasses 每个子通道的描述数组
  explicit render_pass(std::uint32_t subpasses_count, const subpass_description *subpasses);

  render_pass(const render_pass &) = delete;

  render_pass(render_pass &&mov) noexcept;

  ~render_pass();

  render_pass &operator=(render_pass &&mov);

  /// 获得指定子通道的描述
  /// @param index 子通道的编号
  [[nodiscard]] const subpass_description &subpass(std::uint32_t index) const {
    return m_subpasses[index];
  }

  /// 记录渲染通道状态
  class state;

private:

  std::uint32_t m_subpasses_count;
  const subpass_description *m_subpasses;
};

/// 为每个附件进行初始化赋值
struct clear_value {
  union color {
    float f[4];
    std::int32_t i[4];
    std::uint32_t u[4];
  };

  struct depth_stencil {
    float depth;
    std::uint32_t stencil;
  };

  color color;
  depth_stencil depth_stencil;
};

class render_pass::state {
public:

  struct begin_info {
    const render_pass &render_pass;
    const frame_buffer &frame_buffer;
    std::uint16_t clear_values_count;
    const clear_value *clear_values;
  };

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

  const subpass_description *m_first_subpass;
  const subpass_description *m_current_subpass;
  const subpass_description *m_last_subpass;
  const std::byte *m_descriptor_set[1 << 8];
  const std::byte *m_vertex_buffer[1 << 8];
  const frame_buffer *m_frame_buffer;

  friend class graphics_pipeline_impl;
};

} // namespace plaid

#endif // PLAID_RENDER_PASS_H_
