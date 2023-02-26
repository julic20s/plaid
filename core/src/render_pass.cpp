#include "graphics_pipeline_internal.h"

using namespace plaid;

render_pass::~render_pass() {

}

render_pass::state::state(const plaid::render_pass &pass) {
}

void render_pass::state::next_subpass() {
  // TODO
}

void render_pass::state::bind_vertex_buffer(std::uint8_t binding, const std::byte *buf) {
  m_vertex_buffer[binding] = buf;
}

void render_pass::state::bind_pipeline(graphics_pipeline &pipeline) {
  m_graphics_pipeline = pipeline;
}

void render_pass::state::draw(
    std::uint32_t vertex_count, std::uint32_t instance_count,
    std::uint32_t first_vertex, std::uint32_t first_instance
) {
  m_graphics_pipeline->draw(
      *this, vertex_count, instance_count, first_vertex, first_instance
  );
}