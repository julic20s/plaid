#include <atomic>
#include <stdexcept>

#include <plaid/context.h>

#include "graphics_pipeline_internal.h"
#include "render_pass_internal.h"

using namespace plaid;

namespace {
std::atomic_flag render_pass_occupied;
render_pass_impl *current_render_pass;
graphics_pipeline_impl *current_graphics_pipeline;
const std::byte *current_vertex_buffer[1 << 8];
} // namespace

render_pass_state::render_pass_state(plaid::render_pass &pass) {
  if (render_pass_occupied.test_and_set()) {
    throw std::runtime_error("There is a unfinished render pass! You should finish it first.");
  }
  current_render_pass = pass;
}

render_pass_state::~render_pass_state() {
  render_pass_occupied.clear();
}

void render_pass_state::next_subpass() {
  // TODO
}

void render_pass_state::bind_vertex_buffer(std::uint8_t binding, const std::byte *buf) {
  current_vertex_buffer[binding] = buf;
}

void render_pass_state::bind_pipeline(graphics_pipeline &pipeline) {
  current_graphics_pipeline = pipeline;
}

void render_pass_state::draw(
    std::uint32_t vertex_count, std::uint32_t instance_count,
    std::uint32_t first_vertex, std::uint32_t first_instance
) {
  current_graphics_pipeline->draw(
      *current_render_pass, current_vertex_buffer,
      vertex_count, instance_count, first_vertex, first_instance
  );
}
