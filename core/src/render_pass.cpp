#include "graphics_pipeline_internal.h"

using namespace plaid;

struct render_pass::subpass {

};

render_pass::render_pass(std::uint32_t subpasses_count, const subpass_description *subpasses) {
  if (subpasses_count) {
    m_subpasses_count = subpasses_count;
    m_subpasses = new subpass[subpasses_count];
  } else {
    m_subpasses_count = 0;
    m_subpasses = nullptr;
  }
}

render_pass::~render_pass() {
  if (m_subpasses) {
    delete [] m_subpasses;
  }
}

render_pass::state::state(const begin_info &begin) {
  m_frame_buffer = &begin.frame_buffer;
}

void render_pass::state::next_subpass() {
  // TODO
}

void render_pass::state::bind_vertex_buffer(std::uint8_t binding, const std::byte *buf) {
  m_vertex_buffer[binding] = buf;
}

void render_pass::state::draw(
    graphics_pipeline_impl *pipeline,
    std::uint32_t vertex_count, std::uint32_t instance_count,
    std::uint32_t first_vertex, std::uint32_t first_instance
) {
  pipeline->draw(
      *this, vertex_count, instance_count, first_vertex, first_instance
  );
}
