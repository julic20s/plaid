#include "graphics_pipeline_internal.h"

using namespace plaid;

render_pass::render_pass(std::uint32_t subpasses_count, const subpass_description *subpasses) {
  if (subpasses_count) {
    m_subpasses_count = subpasses_count;
    m_subpasses = new subpass_description[subpasses_count];
    auto it = m_subpasses, ed = it + subpasses_count;
    for (; it != ed; ++it, ++subpasses) {
      *it = *subpasses;
      if (it->input_attachments_count) {
        auto dst = new attachment_reference[it->input_attachments_count];
        it->input_attachments = dst;
        auto dst_it = dst, dst_ed = dst_it + it->input_attachments_count;
        auto src_it = subpasses->input_attachments;
        for (; dst_it != dst_ed; ++dst_it) {
          *dst_it = *src_it;
        }
      } else {
        it->input_attachments = nullptr;
      }
      if (it->color_attachments_count) {
        auto dst = new attachment_reference[it->color_attachments_count];
        it->color_attachments = dst;
        auto dst_it = dst, dst_ed = dst_it + it->color_attachments_count;
        auto src_it = subpasses->color_attachments;
        for (; dst_it != dst_ed; ++dst_it) {
          *dst_it = *src_it;
        }
      } else {
        it->color_attachments = nullptr;
      }
      if (it->depth_stencil_attachment) {
        it->depth_stencil_attachment = new attachment_reference(*it->depth_stencil_attachment);
      }
    }
  } else {
    m_subpasses_count = 0;
    m_subpasses = nullptr;
  }
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
}

render_pass::state::state(const begin_info &begin) {
  m_current_subpass = m_first_subpass = begin.render_pass.m_subpasses;
  m_last_subpass = m_first_subpass + begin.render_pass.m_subpasses_count;
  m_frame_buffer = &begin.frame_buffer;
}

void render_pass::state::next_subpass() {
  ++m_current_subpass;
  if (m_current_subpass == m_last_subpass) {
    m_current_subpass = m_first_subpass;
  }
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
