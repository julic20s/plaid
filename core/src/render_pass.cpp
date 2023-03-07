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

  m_subpasses_count = info.subpasses_count;
  m_attachments_count = info.attachments_count;
  m_subpasses = copied_subpasses;
  m_attachments = copied_attachments;
}

render_pass::render_pass(render_pass &&mov) noexcept {
  m_attachments_count = mov.m_attachments_count;
  m_subpasses_count = mov.m_subpasses_count;
  m_attachments = mov.m_attachments;
  m_subpasses = mov.m_subpasses;
  mov.m_attachments_count = 0;
  mov.m_subpasses_count = 0;
  mov.m_attachments = nullptr;
  mov.m_subpasses = nullptr;
}

render_pass::~render_pass() {
  auto it = m_subpasses, ed = it + m_subpasses_count;
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
  if (m_subpasses) {
    delete [] m_subpasses;
  }
  if (m_attachments) {
    delete [] m_attachments;
  }
}

render_pass &render_pass::operator=(render_pass &&mov) {
  return *new (this) render_pass(static_cast<render_pass &&>(mov));
}

render_pass::state::state(const begin_info &begin) {
  m_attachment_descriptions = begin.render_pass.m_attachments;
  m_current_subpass = m_first_subpass = begin.render_pass.m_subpasses;
  m_last_subpass = m_first_subpass + begin.render_pass.m_subpasses_count;
  m_frame_buffer = &begin.frame_buffer;
  m_clear_values_count = begin.clear_values_count;
  m_clear_values = begin.clear_values;
}

void render_pass::state::next_subpass() {
  ++m_current_subpass;
  if (m_current_subpass == m_last_subpass) {
    m_current_subpass = m_first_subpass;
  }
}

void render_pass::state::bind_descriptor_set(std::uint8_t binding, const std::byte *buf) {
  m_descriptor_set[binding] = buf;
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
