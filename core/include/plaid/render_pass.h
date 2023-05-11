#pragma once
#ifndef PLAID_RENDER_PASS_H_
#define PLAID_RENDER_PASS_H_

#include <cstddef>
#include <cstdint>

#include "format.h"

namespace plaid {
class frame_buffer;
class graphics_pipeline;
} // namespace plaid

namespace plaid {

/// 指定附件的加载偏好
enum class attachment_load_op : std::uint8_t {
  // 保留之前的内容
  load = 0,
  // 清除之前的内容，清除值将由 clear_value 指定
  clear = 1,
  // 之前的内容不会被写入
  dont_care = 2,
};

/// 指定附件的写入偏好
enum class attachment_store_op : std::uint8_t {
  // 内容将被写入附件
  store = 0,
  // 内容将不会被写入附件
  dont_care = 1,
};

struct attachment_description {
  attachment_load_op load_op;
  attachment_store_op store_op;
  attachment_load_op stencil_load_op;
  attachment_store_op stencil_store_op;
};

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
  std::uint8_t input_attachments_count;
  /// 指明颜色附件的总数目
  std::uint8_t color_attachments_count;
  /// 输入附件数组
  const attachment_reference *input_attachments;
  /// 颜色附件数组
  const attachment_reference *color_attachments;
  /// 深度附件
  const attachment_reference *depth_stencil_attachment;
};

struct subpass_dependency {
  std::uint8_t src_subpass;
  std::uint8_t dst_subpass;
};

/// 记录一个多通道渲染
class render_pass {
public:

  struct create_info;

  /// 创建一个空的多通道渲染
  render_pass()
      : attachments_count_(0),
        subpasses_count_(0),
        attachments_(nullptr),
        subpasses_(nullptr) {}

  /// 根据参数创建多通道渲染
  /// @param info 创建参数
  explicit render_pass(const create_info &info);

  render_pass(const render_pass &) = delete;

  render_pass(render_pass &&mov) noexcept;

  ~render_pass();

  render_pass &operator=(render_pass &&mov) noexcept;

  /// 获得指定附件的描述
  /// @param index 附件编号
  [[nodiscard]] inline const attachment_description &attachment(std::uint8_t index) const {
    return attachments_[index];
  }

  /// 获得指定子通道的描述
  /// @param index 子通道的编号
  [[nodiscard]] inline const subpass_description &subpass(std::uint8_t index) const {
    return subpasses_[index];
  }

  /// 记录渲染通道状态
  class state;

private:

  std::uint8_t attachments_count_;
  std::uint8_t subpasses_count_;
  const attachment_description *attachments_;
  const subpass_description *subpasses_;
};

struct render_pass::create_info {
  std::uint8_t attachments_count;
  std::uint8_t subpasses_count;
  std::uint8_t dependencies_count;
  attachment_description *attachments;
  subpass_description *subpasses;
  subpass_dependency *dependencies;
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
    std::uint8_t clear_values_count;
    const clear_value *clear_values;
  };

  state(const begin_info &);

  /// 绑定描述符集
  void bind_descriptor_set(std::uint8_t binding, const std::byte *);

  /// 绑定顶点缓冲区
  void bind_vertex_buffer(std::uint8_t binding, const std::byte *);

  /// 根据当前渲染通道状态，绘制一帧
  void draw(
      graphics_pipeline &,
      std::uint32_t vertices_count, std::uint32_t instances_count,
      std::uint32_t first_vertex, std::uint32_t first_instance
  );

  /// 根据当前渲染通道状态和索引缓冲区，绘制一帧
  void draw_indexed(
      graphics_pipeline &,
      std::uint32_t indices_count, std::uint32_t instances_count,
      std::uint32_t first_index, std::int32_t vertex_offset,
      std::uint32_t first_instance
  );

  /// 移动状态到下一个渲染子通道
  void next_subpass();

private:

  const attachment_description *attachment_descriptions_;
  const subpass_description *first_subpass_;
  const subpass_description *current_subpass_;
  const subpass_description *last_subpass_;
  const std::byte *descriptor_set_[1 << 8];
  const std::byte *vertex_buffer_[1 << 8];
  const frame_buffer *frame_buffer_;

  std::uint8_t clear_values_count_;
  const clear_value *clear_values_;

  friend class graphics_pipeline_cache;
};

} // namespace plaid

#endif // PLAID_RENDER_PASS_H_
