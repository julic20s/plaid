#pragma once
#ifndef PLAID_GRAPHICS_PIPELINE_INTERNAL_H_
#define PLAID_GRAPHICS_PIPELINE_INTERNAL_H_

#include <plaid/pipeline.h>
#include <plaid/render_pass.h>
#include <plaid/vec.h>

namespace plaid {

class graphics_pipeline_impl {
public:

  graphics_pipeline_impl(const graphics_pipeline::create_info &);

  ~graphics_pipeline_impl();

  /// 绑定顶点缓冲区
  void bind_vertex_buffer(std::uint8_t binding, const std::byte *);

  void draw(
      plaid::render_pass::state &,
      std::uint32_t vertex_count, std::uint32_t instance_count,
      std::uint32_t first_vertex, std::uint32_t first_instance
  );

private:

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
  void invoke_vertex_shader(std::uint8_t dst, vec4 &);

  void rasterize_triangle(vec4 (&)[3]);

public:

  viewport viewport;
  primitive_topology vertex_assembly;

private:
  /// 顶点着色器逐顶点属性数量
  std::uint16_t vertex_input_per_vertex_attributes_count;
  /// 顶点着色器逐实例属性数量
  std::uint16_t vertex_input_per_instance_attributes_count;

  /// 顶点着色器入口函数
  shader_module::entry_function *vertex_shader;

  /// 保存描述符绑定点数据指针，顶点着色器入口函数参数 0
  const std::byte *descriptor_set_map[1 << 8];
  /// 顶点着色器入口函数参数 1
  const std::byte *vertex_shader_input[1 << 8];
  /// 顶点着色器入口函数参数 2
  std::byte *vertex_shader_output[3][256];

  /// 顶点着色器输出变量的堆内存指针
  std::byte *vertex_shader_output_resource;
  std::align_val_t vertex_shader_output_resource_align;

  /// 每个顶点着色器输出变量组的字节数，一共有三组，则
  /// [vertex_shader_output_resource] 指向的缓冲区大小为
  /// [vertex_shader_output_size] * 3;
  std::uint32_t vertex_shader_output_size;

  struct vertex_attribute_detail {
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
  vertex_attribute_detail vertex_input_per_vertex_attributes[1 << 8];

  /// 保存顶点着色器逐实例属性
  vertex_attribute_detail vertex_input_per_instance_attributes[1 << 8];

  /// 片元着色器入口函数
  shader_module::entry_function *fragment_shader;
};

} // namespace plaid

#endif // PLAID_GRAPHICS_PIPELINE_INTERNAL_H_
