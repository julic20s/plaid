#pragma once
#ifndef PLAID_CONTEXT_H_
#define PLAID_CONTEXT_H_

#include <cstdint>

#include "pipeline.h"
#include "render_pass.h"

namespace plaid {

/// 记录渲染通道状态，同一进程不能同时存在多个 [render_pass_state]
class render_pass_state {
public:

  /// 在上一个 [render_pass_state] 对象未析构时调用本构造函数将导致
  /// throw std::runtime_error
  render_pass_state(render_pass &);

  render_pass_state(const render_pass_state &) = delete;
  render_pass_state(render_pass_state &&) = delete;

  ~render_pass_state();

  /// 绑定描述符集
  void bind_descriptor_set();

  /// 绑定顶点缓冲区
  void bind_vertex_buffer(std::uint8_t binding, const std::byte *);

  /// 绑定图形管道
  void bind_pipeline(graphics_pipeline &);

  /// 根据当前渲染通道状态，绘制一帧
  void draw(
      std::uint32_t vertex_count, std::uint32_t instance_count,
      std::uint32_t first_vertex, std::uint32_t first_instance
  );

  /// 移动状态到下一个渲染子通道
  void next_subpass();
};

} // namespace plaid

#endif // PLAID_CONTEXT_H_
