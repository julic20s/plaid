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
  constexpr plaid::vertex_input_binding_description bindings[]{
      {
          .binding = 0,
          .input_rate = plaid::vertex_input_rate::vertex,
          .stride = sizeof(vertex),
      }};

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

  const auto vert = plaid::dsl_shader_module::load<&triangle::vert::main>();
  const auto frag = plaid::dsl_shader_module::load<&triangle::frag::main>();

  plaid::graphics_pipeline::create_info create_info{
      .vertex_input_state{
          .bindings_count = 1,
          .attributes_count = 2,
          .bindings = bindings,
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
  static const vertex triangle[] = {
      {{0, -.5}, {1, 0, 0}},
      {{-.5, .5}, {0, 1, 0}},
      {{.5f, .5}, {0, 0, 1}},
  };
  plaid::render_pass::state state(viewer_render_pass);
  state.bind_vertex_buffer(0, reinterpret_cast<const std::byte *>(triangle));
  state.draw(viewer_pipeline, 3, 1, 0, 0);
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
