#pragma once
#ifndef PLAID_GRAPHICS_PIPELINE_INTERNAL_H_
#define PLAID_GRAPHICS_PIPELINE_INTERNAL_H_

#include <plaid/pipeline.h>
#include <plaid/vec.h>

namespace plaid {

class graphics_pipeline_impl {
public:
  graphics_pipeline_impl(const graphics_pipeline::create_info &);

  void draw(
      std::uint32_t vertex_count, std::uint32_t instance_count,
      std::uint32_t first_vertex, std::uint32_t first_instance
  );

private:
  void draw_triangle_strip(
      std::uint32_t first_vert, std::uint32_t last_vert,
      std::uint32_t first_inst, std::uint32_t last_inst
  );

  void invoke_vertex_shader(std::uint32_t inst_id, std::uint32_t vert_id, std::uint8_t dst, vec4 &);

  void rasterize_triangle(vec4 (&)[3]);

public:
  viewport viewport;
  primitive_topology vertex_assembly;

private:
  shader_module::entry_function *vertex_shader;
  shader_module::entry_function *fragment_shader;
  std::byte *shader_generated_resource;
  std::uint32_t vertex_shader_output_size;
  std::byte *descriptor_set_map[1 << 8];
  const std::byte *vertex_input_map[1 << 8];
};

} // namespace plaid

#endif // PLAID_GRAPHICS_PIPELINE_INTERNAL_H_
