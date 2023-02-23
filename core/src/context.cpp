#include <atomic>
#include <stdexcept>

#include <plaid/context.h>

#include "graphics_pipeline_internal.h"

using namespace plaid;

namespace {
std::atomic_flag render_pass_occupied;
graphics_pipeline_impl *current_graphics_pipeline;
} // namespace

render_pass_state::render_pass_state(plaid::render_pass &) {
  if (render_pass_occupied.test_and_set()) {
    throw std::runtime_error("There is a unfinished render pass! You should finish it first.");
  }
}

render_pass_state::~render_pass_state() {
  render_pass_occupied.clear();
}

void render_pass_state::next_subpass() {
  // TODO
}

void render_pass_state::bind_vertex_buffer(std::uint8_t binding, const std::byte *buf) {
  current_graphics_pipeline->bind_vertex_buffer(binding, buf);
}

void render_pass_state::bind_pipeline(graphics_pipeline &pipeline) {
  current_graphics_pipeline = unwrap<graphics_pipeline_impl>(pipeline.handle());
}

void render_pass_state::draw(
    std::uint32_t vertex_count, std::uint32_t instance_count,
    std::uint32_t first_vertex, std::uint32_t first_instance
) {
  current_graphics_pipeline->draw(vertex_count, instance_count, first_vertex, first_instance);
}
