#include <algorithm>

#include "graphics_pipeline_internal.h"

using namespace plaid;

render_pass::render_pass(const create_info &info) {
  subpass_description *copied_subpasses = nullptr;
  if (info.subpasses_count) {
    auto subpass = info.subpasses;
    copied_subpasses = new subpass_description[info.subpasses_count];
    for (auto it = copied_subpasses, ed = it + info.subpasses_count; it != ed; ++it, ++subpass) {
      *it = *subpass;
      if (it->input_attachments_count) {
        auto dst = new attachment_reference[it->input_attachments_count];
        std::copy_n(subpass->input_attachments, it->input_attachments_count, dst);
        it->input_attachments = dst;
      }
      if (it->color_attachments_count) {
        auto dst = new attachment_reference[it->color_attachments_count];
        std::copy_n(subpass->color_attachments, it->color_attachments_count, dst);
        it->color_attachments = dst;
      }
      if (it->depth_stencil_attachment) {
        it->depth_stencil_attachment = new attachment_reference(*it->depth_stencil_attachment);
      }
    }
  }

  attachment_description *copied_attachments = nullptr;
  if (info.attachments_count) {
    auto attachment = info.attachments;
    copied_attachments = new attachment_description[info.attachments_count];
    for (auto it = copied_attachments, ed = it + info.attachments_count; it != ed; ++it, ++attachment) {
      *it = *attachment;
    }
  }

  subpasses_count_ = info.subpasses_count;
  attachments_count_ = info.attachments_count;
  subpasses_ = copied_subpasses;
  attachments_ = copied_attachments;
}

render_pass::render_pass(render_pass &&mov) noexcept {
  attachments_count_ = mov.attachments_count_;
  subpasses_count_ = mov.subpasses_count_;
  attachments_ = mov.attachments_;
  subpasses_ = mov.subpasses_;
  mov.attachments_count_ = 0;
  mov.subpasses_count_ = 0;
  mov.attachments_ = nullptr;
  mov.subpasses_ = nullptr;
}

render_pass::~render_pass() {
  auto it = subpasses_, ed = it + subpasses_count_;
  for (; it != ed; ++it) {
    if (it->input_attachments_count) {
      delete [] it->input_attachments;
    }
    if (it->color_attachments_count) {
      delete [] it->color_attachments;
    }
    if (it->depth_stencil_attachment) {
      delete it->depth_stencil_attachment;
    }
  }
  if (subpasses_) {
    delete [] subpasses_;
  }
  if (attachments_) {
    delete [] attachments_;
  }
}

render_pass &render_pass::operator=(render_pass &&mov) noexcept {
  return *new (this) render_pass(static_cast<render_pass &&>(mov));
}

render_pass::state::state(const begin_info &begin) {
  attachment_descriptions_ = begin.render_pass.attachments_;
  current_subpass_ = first_subpass_ = begin.render_pass.subpasses_;
  last_subpass_ = first_subpass_ + begin.render_pass.subpasses_count_;
  frame_buffer_ = &begin.frame_buffer;
  clear_values_count_ = begin.clear_values_count;
  clear_values_ = begin.clear_values;
}

void render_pass::state::next_subpass() {
  ++current_subpass_;
  if (current_subpass_ == last_subpass_) {
    current_subpass_ = first_subpass_;
  }
}

void render_pass::state::bind_descriptor_set(std::uint8_t binding, const std::byte *buf) {
  descriptor_set_[binding] = buf;
}

void render_pass::state::bind_vertex_buffer(std::uint8_t binding, const std::byte *buf) {
  vertex_buffer_[binding] = buf;
}

void render_pass::state::draw(
    graphics_pipeline &pipeline,
    std::uint32_t vertex_count, std::uint32_t instance_count,
    std::uint32_t first_vertex, std::uint32_t first_instance
) {
  graphics_pipeline_cache &cache = pipeline;
  cache.draw(
      *this, vertex_count, instance_count, first_vertex, first_instance
  );
}

void render_pass::state::draw_indexed(
  graphics_pipeline &pipeline,
  std::uint32_t indices_count, std::uint32_t instances_count,
  std::uint32_t first_index, std::int32_t vertex_offset,
  std::uint32_t first_instance
) {
  graphics_pipeline_cache &cache = pipeline;
  cache.draw_indexed(
    *this, indices_count, instances_count, first_index, vertex_offset, first_instance
  );
}
