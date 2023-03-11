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

  /// 按照给定顶点范围执行绘制
  /// @param vertex_count 要绘制的顶点总数
  /// @param instances_count 要绘制的实例总数
  /// @param first_vertex 第一个顶点的编号
  /// @param first_instance 第一个实例的编号
  void draw(
      const plaid::render_pass::state &,
      std::uint32_t vertex_count, std::uint32_t instances_count,
      std::uint32_t first_vertex, std::uint32_t first_instance
  );

  /// 按照索引缓冲区的顺序执行绘制
  /// @param indices_count 要绘制的顶点总数
  /// @param instances_count 要绘制的实例总数
  /// @param first_index 第一个顶点索引的编号
  /// @param vertex_offset 顶点编号偏移
  /// @param first_instance 第一个实例的编号
  void draw_indexed(
      const plaid::render_pass::state &,
      std::uint32_t indices_count, std::uint32_t instances_count,
      std::uint32_t first_vertex, std::int32_t vertex_offset,
      std::uint32_t first_instance
  );

private:

  /// 将采用颜色清除值重置附件
  static void clear_color_attachment(const render_pass::state &, attachment_reference);

  /// 采用模板清除值重置附件
  static void clear_stencil_attachment(const render_pass::state &, attachment_reference);

  /// 绘制 (n / 3) 个三角形，不对顶点进行重用
  /// @param first_vert 第一个顶点编号
  /// @param last_vert 最后一个顶点之后的顶点编号 (不绘制)
  /// @param first_inst 第一个实例的编号
  /// @param last_inst 最后一个实例之后的实例 (不绘制)
  void draw_triangle_list(
      const render_pass::state &,
      std::uint32_t first_vert, std::uint32_t last_vert,
      std::uint32_t first_inst, std::uint32_t last_inst
  );

  /// 绘制 (n - 2) 个三角形，第 2 ~ (n - 1) 个顶点存在共用
  /// @param first_vert 第一个顶点编号
  /// @param last_vert 最后一个顶点之后的顶点编号 (不绘制)
  /// @param first_inst 第一个实例的编号
  /// @param last_inst 最后一个实例之后的实例 (不绘制)
  void draw_triangle_strip(
      const render_pass::state &,
      std::uint32_t first_vert, std::uint32_t last_vert,
      std::uint32_t first_inst, std::uint32_t last_inst
  );

  /// 更新逐顶点属性
  /// @param vertex_buffer 顶点缓冲区列表
  /// @param inst_id 顶点 ID
  void obtain_next_vertex_attribute(const const_memory_array<1 << 8> &vertex_buffer, std::uint32_t vert_id);

  /// 更新逐实例属性
  /// @param vertex_buffer 顶点缓冲区列表
  /// @param inst_id 实例 ID
  void obtain_next_instance_attributes(const const_memory_array<1 << 8> &vertex_buffer, std::uint32_t inst_id);

  /// 执行顶点着色器
  /// @param descriptor_set 描述符集列表
  /// @param output 接收顶点着色器输出变量列表
  /// @param clip_coord 接收裁剪空间坐标
  void invoke_vertex_shader(
      const const_memory_array<1 << 8> &descriptor_set,
      const memory_array<1 << 8> &output, vec4 &clip_coord
  );

  /// 光栅化三角形
  void rasterize_triangle(const render_pass::state &, const vec4 *const (&)[3]);

  /// 执行片元着色器
  /// @param fragcoord 片元屏幕坐标
  /// @param index 片元在每个附件中的索引
  /// @param weight 三个顶点的权重
  void invoke_fragment_shader(const render_pass::state &, vec3 fragcoord, std::uint32_t index, const float (&weight)[3]);

public:

  viewport viewport;

  /// 顶点装配模式
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
  /// 顶点着色器输入变量的地址索引表，一个数组下标就对应一个变量编号
  const std::byte *m_vertex_shader_input[1 << 8];
  /// 顶点着色器输出变量的地址索引表，一个数组下标就对应一个变量编号
  std::byte *m_vertex_shader_output[3][1 << 8];

  /// 着色器输入变量元属性
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
  /// 片元着色器输入变量的地址索引表，一个数组下标就对应一个变量编号
  std::byte *m_fragment_shader_input[1 << 8];
  /// 片元着色器输出变量的地址索引表，一个数组下标就对应一个变量编号
  std::byte *m_fragment_shader_output[1 << 8];

  /// 片元着色器输入变量元属性
  struct fragment_input_detail {
    /// 变量编号
    std::uint8_t location;
    /// 插值函数
    shader_stage_variable_description::interpolation_function *interpolation;
  };
  /// 保存片元着色器变量元属性
  fragment_input_detail m_fragment_input[1 << 8];

  /// 片元着色器输出变量元属性
  struct fragment_output_detail {
    /// 变量编号
    std::uint8_t location;
    /// 目标附件编号
    std::uint8_t attachment_id;
    /// 目标附件单位长度
    std::uint32_t attachment_stride;
    /// 内存布局变换函数
    attachment_transition_function *attachment_transition;
  };
  /// 保存片元着色器变量元属性
  fragment_output_detail m_fragment_output[1 << 8];
};

} // namespace plaid

#endif // PLAID_GRAPHICS_PIPELINE_INTERNAL_H_
