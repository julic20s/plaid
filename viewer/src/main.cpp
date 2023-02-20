#include <plaid.h>

#include "triangle.hpp"
#include "window.h"

/// 顶点数据
struct vertex {
  plaid::vec2 position;
  plaid::vec3 color;
};

plaid::render_pass viewer_render_pass;
plaid::graphics_pipeline viewer_pipeline;

/// 初始化渲染通道
void initialize_render_pass() {
}

/// 初始化图形管道
void initialize_pipeline() {
  constexpr plaid::vertex_input_attribute_description attrs[]{
      {
          .location = 0,
          .binding = 0,
          .offset = offsetof(vertex, position),
      },
      {
          .location = 1,
          .binding = 0,
          .offset = offsetof(vertex, color),
      },
  };

  auto vert = plaid::dsl_shader_module::load<&triangle::vert::main>();
  auto frag = plaid::dsl_shader_module::load<&triangle::frag::main>();

  plaid::graphics_pipeline::create_info create_info{
      .vertex_input_state{
          .attributes_count = 2,
          .attributes = attrs,
      },
      .input_assembly_state{
          .topology = plaid::primitive_topology::triangle_strip,
      },
      .shader_stage = {
          .vertex_shader = &vert,
          .fragment_shader = &frag,
      },
  };

  viewer_pipeline = plaid::graphics_pipeline(create_info);
}

void initialize() {
  initialize_render_pass();
  initialize_pipeline();
}

void render() {
  plaid::render_pass_state state(viewer_render_pass);
  state.bind_pipeline(viewer_pipeline);
  state.draw(3, 1, 0, 0);
  state.next_subpass();
}

int main(int argc, const char *argv[]) {
  // TODO：解析参数，根据参数确定窗口大小
  std::uint32_t user_width = 800, user_height = 600;

  auto window = window::create("plaid", user_width, user_height);
  if (!window.valid()) {
    return 0;
  }

  initialize();

  window.show();
  while (!window.should_close()) {
    /// 渲染帧
    render();
    window.poll_events();
  }

  return 0;
}
