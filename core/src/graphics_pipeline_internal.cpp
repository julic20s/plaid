#include <stdexcept>

#include "graphics_pipeline_internal.h"

using namespace plaid;

graphics_pipeline::graphics_pipeline(const graphics_pipeline::create_info &info) {
  h = wrap(new graphics_pipeline_impl(info));
}

graphics_pipeline::~graphics_pipeline() {
  delete unwrap<graphics_pipeline_impl>(h);
}

graphics_pipeline_impl::graphics_pipeline_impl(const graphics_pipeline::create_info &info) {
  vertex_assembly = info.input_assembly_state.topology;
  vertex_shader = info.shader_stage.vertex_shader->entry;
  fragment_shader = info.shader_stage.fragment_shader->entry;

  
}

void graphics_pipeline_impl::draw(
    std::uint32_t vertex_count, std::uint32_t instance_count,
    std::uint32_t first_vertex, std::uint32_t first_instance
) {
  auto last_vert = first_vertex + vertex_count;
  auto last_inst = first_instance + instance_count;
  switch (vertex_assembly) {
    case primitive_topology::triangle_strip:
      draw_triangle_strip(first_vertex, last_vert, first_instance, last_inst);
      break;
    case primitive_topology::line_strip:
      throw std::runtime_error("Unsupported topology line_strip.");
      break;
  }
}

void graphics_pipeline_impl::draw_triangle_strip(
    std::uint32_t first_vert, std::uint32_t last_vert,
    std::uint32_t first_inst, std::uint32_t last_inst
) {
  [[unlikely]] if (last_vert - first_inst <= 2) {
    return;
  }

  vec4 clip_coords[3];
  for (auto inst = first_inst; inst != last_inst; ++inst) {
    std::uint32_t indices[]{0, 1, 2};
    for (auto i : indices) {
      /// 顶点编号刚好对应数据位置，而下面的 while 循环则不能如此
      invoke_vertex_shader(inst, i, i, clip_coords[i]);
    }

    /// TODO：这里还需要确定顶点顺序，使得面剔除能够正确执行
    /// （虽然现在也还没有写面剔除
    int ping_pong = 0;
    while (1) {
      rasterize_triangle(clip_coords);

      indices[ping_pong] += 3;
      if (indices[ping_pong] == last_vert) break;
      invoke_vertex_shader(inst, indices[ping_pong], ping_pong, clip_coords[ping_pong]);
      ping_pong = (ping_pong + 1) % 3;
    }
  }
}

void graphics_pipeline_impl::invoke_vertex_shader(
    std::uint32_t inst_id, std::uint32_t vert_id,
    std::uint8_t dst, vec4 &clip_coord
){}

void graphics_pipeline_impl::rasterize_triangle(vec4 (&clip_coord)[3]) {
  
}
