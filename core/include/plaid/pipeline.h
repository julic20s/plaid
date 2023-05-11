/// 管道可以同时接受多个顶点缓冲区，每个缓冲区即定义为一个“绑定点” (binding)
/// 每个绑定点可以包含多个着色器属性 (attribute)

#pragma once
#ifndef PLAID_PIPELINE_H_
#define PLAID_PIPELINE_H_

#include "utility.h"

// 前向声明
namespace plaid {
class render_pass;
class shader_module;
enum class primitive_topology : std::uint8_t;
}

namespace plaid {

/// 图形管道缓存
class graphics_pipeline_cache;

/// 图形管道缓存的包装，可移动不可复制，所有图形管道缓存的创建在内部管理
class graphics_pipeline {
public:

  /// 创建指定参数的图形管道
  struct create_info;

  /// 创建一个空的管道指针
  graphics_pipeline() : cache_(nullptr) {}

  graphics_pipeline(graphics_pipeline_cache &cache);

  /// 根据参数创建管道
  /// @param info 图形管道参数
  explicit graphics_pipeline(const create_info &info);

  graphics_pipeline(const graphics_pipeline &) = delete;

  graphics_pipeline(graphics_pipeline &&) noexcept;

  ~graphics_pipeline();

  graphics_pipeline &operator=(graphics_pipeline &&) noexcept;

  /// 转换为管道缓存
  [[nodiscard]] inline operator
  graphics_pipeline_cache &() noexcept {
    return *cache_;
  }

  /// 获取顶点装配状态
  [[nodiscard]] const primitive_topology &vertex_assembly() noexcept;

private:

  graphics_pipeline_cache *cache_;
};

/// 指定管道从顶点缓冲区读入新数据的频率
enum class vertex_input_rate : std::uint8_t {
  /// 每绘制一个顶点，就从缓冲区获取一个新数据
  vertex,
  /// 每绘制一个实例，就从缓冲区获取一个新数据
  instance,
};

/// 描述一个绑定点的规格，管线可以以绑定点为单位绑定顶点缓冲区
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
  triangle_list,
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

/// 面剔除位掩码
struct cull_modes {
  /// 不进行面剔除
  static constexpr cull_mode none = 0;
  /// 剔除正面
  static constexpr cull_mode front = 1;
  /// 剔除背面
  static constexpr cull_mode back = 2;
};

struct graphics_pipeline::create_info {
  /// 顶点输入缓冲区规格
  struct vertex_input_state {
    /// 指明当前顶点输入的绑定点个数
    std::uint8_t bindings_count;
    /// 指明当前顶点输入的属性个数
    std::uint8_t attributes_count;
    /// 绑定点描述数组
    const vertex_input_binding_description *bindings;
    /// 属性描述数组
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
    std::uint8_t viewports_count;
    std::uint8_t scissors_count;
    const viewport *viewports;
    const rect2d *scissors;
  };

  vertex_input_state vertex_input_state;
  input_assembly_state input_assembly_state;
  shader_stages shader_stage;
  rasterization_state rasterization_state;
  viewport_state viewport_state;
  const render_pass &render_pass;
  std::uint8_t subpass;
};

} // namespace plaid

#endif // PLAID_PIPELINE_H_
