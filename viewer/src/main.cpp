#include <ctime>

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <plaid.h>

#include "obj_model.h"
#include "triangle.hpp"
#include "window.h"

plaid::render_pass viewer_render_pass;
plaid::graphics_pipeline viewer_pipeline;
plaid::frame_buffer viewer_frame_buffer;
std::unique_ptr<float[]> depth_buffer;
std::uint32_t size;

/// 初始化渲染通道
void initialize_render_pass() {
  plaid::attachment_reference color_attachment_ref{0, plaid::format::bgra8888_unsigned_integer};
  plaid::attachment_reference depth_stencil_attachment{1};
  plaid::subpass_description subpass{
      .color_attachments_count = 1,
      .color_attachments = &color_attachment_ref,
      .depth_stencil_attachment = &depth_stencil_attachment,
  };

  plaid::attachment_description attachments[]{
      {
          // color attachment
          .load_op = plaid::attachment_load_op::clear,
          .store_op = plaid::attachment_store_op::store,
      },
  };

  plaid::render_pass::create_info create_info{
      .attachments_count = 2,
      .subpasses_count = 1,
      .attachments = attachments,
      .subpasses = &subpass,
  };

  viewer_render_pass = plaid::render_pass(create_info);
}

/// 初始化图形管道
void initialize_pipeline() {
  constexpr plaid::vertex_input_binding_description bindings[]{
      {
          .binding = 0,
          .input_rate = plaid::vertex_input_rate::vertex,
          .stride = sizeof(obj_model::vertex),
      }};

  constexpr plaid::vertex_input_attribute_description attrs[]{
      {
          .location = 0,
          .binding = 0,
          .offset = offsetof(obj_model::vertex, pos_index),
      },
  };

  const auto vert = plaid::dsl_shader_module::load<&triangle::vert::main>();
  const auto frag = plaid::dsl_shader_module::load<&triangle::frag::main>();

  plaid::graphics_pipeline::create_info create_info{
      .vertex_input_state{
          .bindings_count = 1,
          .attributes_count = 1,
          .bindings = bindings,
          .attributes = attrs,
      },
      .input_assembly_state{
          .topology = plaid::primitive_topology::triangle_list,
      },
      .shader_stage = {
          .vertex_shader = vert,
          .fragment_shader = frag,
      },
      .render_pass = viewer_render_pass,
  };

  viewer_pipeline = plaid::graphics_pipeline(create_info);
}

void initialize() {
  initialize_render_pass();
  initialize_pipeline();
}

void recreate_frame_buffer(std::uint32_t *color, std::uint32_t width, std::uint32_t height) {
  depth_buffer = std::make_unique<float[]>(size = width * height);
  std::byte *attachements[] = {
      reinterpret_cast<std::byte *>(color),
      reinterpret_cast<std::byte *>(depth_buffer.get()),
  };
  viewer_frame_buffer = plaid::frame_buffer(
      2, attachements, width, height
  );
}

void render(const obj_model &model) {
  plaid::clear_value color_clear{
      .color{
          .u{0, 0, 0, 0},
      },
  };
  plaid::render_pass::state::begin_info begin_info{
      .render_pass = viewer_render_pass,
      .frame_buffer = viewer_frame_buffer,
      .clear_values_count = 1,
      .clear_values = &color_clear,
  };
  for (auto it = depth_buffer.get(), ed = it + size; it != ed; ++it) {
    *it = 0;
  }

  plaid::render_pass::state state(begin_info);
  state.bind_descriptor_set(0, reinterpret_cast<const std::byte *>(model.positions()));
  state.bind_vertex_buffer(0, reinterpret_cast<const std::byte *>(model.vertices()));
  state.draw(viewer_pipeline, model.size(), 1, 0, 0);
  state.next_subpass();
}

void print_fps() {
  static auto previous_tick = clock();
  static auto frame_count = 0;

  ++frame_count;

  auto tick = clock();
  if (tick - previous_tick >= CLOCKS_PER_SEC) {
    std::cout << "fps: " << frame_count << '\n';
    frame_count = 0;
    previous_tick = tick;
  }
}

int main(int argc, const char *argv[]) {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  std::cerr.tie(nullptr);

  std::uint32_t user_width = 800, user_height = 600;
  const char *file = nullptr;
  for (auto it = argv, ed = it + argc; it != ed; ++it) {
    auto str = *it;
    if (str[0] == '-') {
      if (str[1] == 'w' && str[2] == '=') {
        user_width = std::atoi(str + 3);
      } else if (str[1] == 'h' && str[2] == '=') {
        user_height = std::atoi(str + 3);
      } else {
        std::cerr << "Unknown param: " << str << '\n';
        return 0;
      }
    } else {
      file = *it;
    }
  }

  obj_model model(file);

  auto window = window::create("plaid", user_width, user_height);
  if (!window.valid()) {
    return 0;
  }

  window.on_surface_recreate([](class window &w, std::uint32_t width, std::uint32_t height) {
    recreate_frame_buffer(w.surface(), width, height);
  });

  initialize();

  window.show();
  [[likely]] while (!window.should_close()) {
    // 渲染帧
    render(model);
    window.commit();
    print_fps();
    window.poll_events();
  }

  return 0;
}
