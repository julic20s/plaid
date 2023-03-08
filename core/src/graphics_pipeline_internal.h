#pragma once
#ifndef PLAID_GRAPHICS_PIPELINE_INTERNAL_H_
#define PLAID_GRAPHICS_PIPELINE_INTERNAL_H_

#include <plaid/pipeline.h>
#include <plaid/render_pass.h>
#include <plaid/shader.h>
#include <plaid/vec.h>

#include "attachment_transition.h"

namespace plaid {

class graphics_pipeline_impl {
public:

  graphics_pipeline_impl(const graphics_pipeline::create_info &);

  ~graphics_pipeline_impl();

  void draw(
      plaid::render_pass::state &,
      std::uint32_t vertex_count, std::uint32_t instance_count,
      std::uint32_t first_vertex, std::uint32_t first_instance
  );

private:

  static void clear_color_attachment(render_pass::state &, attachment_reference);

  static void clear_depth_attachment(render_pass::state &, attachment_reference);

  void draw_triangle_list(
      render_pass::state &,
      std::uint32_t first_vert, std::uint32_t last_vert,
      std::uint32_t first_inst, std::uint32_t last_inst
  );

  void draw_triangle_strip(
      render_pass::state &,
      std::uint32_t first_vert, std::uint32_t last_vert,
      std::uint32_t first_inst, std::uint32_t last_inst
  );

  /// 更新逐顶点属性
  void obtain_next_vertex_attribute(const std::byte *(&vertex_buffer)[1 << 8], std::uint32_t vert_id);

  /// 更新逐实例属性
  void obtain_next_instance_attributes(const std::byte *(&vertex_buffer)[1 << 8], std::uint32_t inst_id);

  /// 执行顶点着色器
  void invoke_vertex_shader(const std::byte *(&descriptor_set)[1 << 8], std::byte *(&output)[1 << 8], vec4 &clip_coord);

  void rasterize_triangle(render_pass::state &, const vec4 *const (&)[3]);

  void invoke_fragment_shader(render_pass::state &, std::uint32_t index, const float (&weight)[3]);

public:

  viewport viewport;
  primitive_topology vertex_assembly;

private:
  struct {
    /// 顶点着色器逐顶点属性数量
    std::uint16_t vertex_input_per_vertex;
    /// 顶点着色器逐实例属性数量
    std::uint16_t vertex_input_per_instance;
    /// 片元着色器属性数量
    std::uint16_t fragment_input;
    /// 片元着色器输出数量
    std::uint16_t fragment_output;
    /// 颜色附件数量
    std::uint16_t color_attachments;
  } m_counts;

  /// 动态申请出的内存
  std::byte *m_allocated_memory;
  /// 动态申请出的内存的对齐字节数
  std::align_val_t m_allocated_memory_align;
  /// 申请出的内存分为 4 + 1 块，此值表示前 4 块，每一块的字节数
  std::uint32_t m_allocated_memory_chunk_size;

  /// 顶点着色器入口函数
  shader_module::entry_function *m_vertex_shader;
  /// 顶点着色器入口函数参数 1
  const std::byte *m_vertex_shader_input[1 << 8];
  /// 顶点着色器入口函数参数 2
  std::byte *m_vertex_shader_output[3][1 << 8];

  struct vertex_input_detail {
    /// 属性的编号
    std::uint8_t location;
    /// 绑定点编号
    std::uint8_t binding;
    /// 顶点缓冲区数据项字节数
    std::uint32_t stride;
    /// 数据偏移量
    std::uint32_t offset;
  };
  /// 保存顶点着色器逐顶点属性
  vertex_input_detail m_vertex_input_per_vertex[1 << 8];
  /// 保存顶点着色器逐实例属性
  vertex_input_detail m_vertex_input_per_instance[1 << 8];

  /// 片元着色器入口函数
  shader_module::entry_function *m_fragment_shader;
  /// 片元着色器入口函数参数 1
  std::byte *m_fragment_shader_input[1 << 8];
  /// 片元着色器入口函数参数 2
  std::byte *m_fragment_shader_output[1 << 8];

  struct fragment_input_detail {
    /// 属性的编号
    std::uint8_t location;
    /// 插值函数
    shader_stage_variable_description::interpolation_function *interpolation;
  };
  /// 保存片元着色器属性
  fragment_input_detail m_fragment_input[1 << 8];

  /// 保存所有片元着色器输出
  struct fragment_output_detail {
    /// 着色器输出变量编号
    std::uint8_t location;
    /// 目标附件编号
    std::uint8_t attachment_id;
    std::uint32_t attachment_stride;
    /// 内存布局变换函数
    attachment_transition_function *attachment_transition;
  };
  fragment_output_detail m_fragment_output[1 << 8];
};

} // namespace plaid

#endif // PLAID_GRAPHICS_PIPELINE_INTERNAL_H_
