/// 管道可以同时接受多个顶点缓冲区，每个缓冲区即定义为一个“绑定点” (binding)
/// 每个绑定点可以包含多个着色器属性 (attribute)

#pragma once
#ifndef PLAID_PIPELINE_H_
#define PLAID_PIPELINE_H_

#include <cstdint>

#include "render_pass.h"
#include "shader.h"
#include "utility.h"

namespace plaid {

/// 指定管道从顶点缓冲区读入新数据的频率
enum class vertex_input_rate : std::uint8_t {
  /// 每绘制一个顶点，就从缓冲区获取一个新数据
  vertex,
  /// 每绘制一个实例，就从缓冲区获取一个新数据
  instance,
};

/// 描述一个绑定点的规格
struct vertex_input_binding_description {
  /// 此对象所描述的顶点输入绑定点的编号
  std::uint8_t binding;
  /// 输入速率
  vertex_input_rate input_rate;
  /// 顶点缓冲区数据项字节数
  std::uint32_t stride;
};

/// 描述一个属性的规格
struct vertex_input_attribute_description {
  /// 对应着色器内 location 的值
  std::uint8_t location;
  /// 这个属性所属的绑定点的编号
  std::uint8_t binding;
  /// 数据项起始位置到这个属性起始位置的字节偏移量
  std::uint32_t offset;
};

/// 顶点装配方式
enum class primitive_topology : std::uint8_t {
  /// 绘制折线
  line_strip,
  /// 绘制三角形
  triangle_strip,
};

/// 描述视口
struct viewport {
  /// 视口左上角在颜色附件中的位置
  float x, y;
  /// 视口尺寸
  float width, height;
  /// 深度裁剪
  float min_depth, max_depth;
};

/// 多边形的绘制模式
enum class polygon_mode : std::uint8_t {
  fill,
  line,
};

using cull_mode = std::uint8_t;

/// 面剔除
struct cull_modes {
  static constexpr cull_mode none = 0;
  static constexpr cull_mode front = 1;
  static constexpr cull_mode back = 2;
};

class graphics_pipeline_impl;

/// 图形管道句柄
class graphics_pipeline {
public:
  struct create_info;

  graphics_pipeline() : m_pointer(nullptr) {}

  explicit graphics_pipeline(const create_info &);

  graphics_pipeline(const graphics_pipeline &) = delete;

  graphics_pipeline(graphics_pipeline &&mov) noexcept {
    m_pointer = mov.m_pointer;
    mov.m_pointer = nullptr;
  }

  ~graphics_pipeline();

  graphics_pipeline &operator=(graphics_pipeline &&mov) noexcept {
    m_pointer = mov.m_pointer;
    mov.m_pointer = nullptr;
    return *this;
  }

  [[nodiscard]] inline operator graphics_pipeline_impl *() noexcept { return m_pointer; }

private:
  graphics_pipeline_impl *m_pointer;
};

struct graphics_pipeline::create_info {
  /// 顶点输入缓冲区规格
  struct vertex_input_state {
    std::uint16_t bindings_count;
    std::uint16_t attributes_count;
    const vertex_input_binding_description *bindings;
    const vertex_input_attribute_description *attributes;
  };

  /// 顶点输入装配模式
  struct input_assembly_state {
    primitive_topology topology;
  };

  /// 着色器规格
  struct shader_stages {
    const shader_module &vertex_shader;
    const shader_module &fragment_shader;
  };

  /// 光栅化参数设置
  struct rasterization_state {
    bool depth_clamp;
    bool rasterizer_discard;
    polygon_mode polygon_mode;
    cull_mode cull_mode;
  };

  /// 视口状态
  struct viewport_state {
    std::uint16_t viewports_count;
    std::uint16_t scissors_count;
    const viewport *viewports;
    const rect2d *scissors;
  };

  vertex_input_state vertex_input_state;
  input_assembly_state input_assembly_state;
  shader_stages shader_stage;
  rasterization_state rasterization_state;
  viewport_state viewport_state;
  render_pass &render_pass;
  std::uint8_t subpass;
};

} // namespace plaid

#endif // PLAID_PIPELINE_H_
