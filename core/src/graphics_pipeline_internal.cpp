#include <algorithm>
#include <stdexcept>

#include <plaid/frame_buffer.h>

#include "graphics_pipeline_internal.h"

using namespace plaid;

graphics_pipeline::graphics_pipeline(const graphics_pipeline::create_info &info) {
  m_pointer = new graphics_pipeline_impl(info);
}

graphics_pipeline::graphics_pipeline(graphics_pipeline &&mov) noexcept {
  m_pointer = mov.m_pointer;
  mov.m_pointer = nullptr;
}

graphics_pipeline::~graphics_pipeline() {
  if (m_pointer) {
    delete m_pointer;
  }
}

graphics_pipeline &graphics_pipeline::operator=(graphics_pipeline &&mov) noexcept {
  return *new (this) graphics_pipeline(static_cast<graphics_pipeline &&>(mov));
}

graphics_pipeline_impl::graphics_pipeline_impl(const graphics_pipeline::create_info &info) {
  vertex_assembly = info.input_assembly_state.topology;

  auto &vertex_shader_module = info.shader_stage.vertex_shader;
  auto &fragment_shader_module = info.shader_stage.fragment_shader;

  // 获取顶点着色器入口函数
  m_vertex_shader = vertex_shader_module.entry;
  m_fragment_shader = fragment_shader_module.entry;

  // 把绑定点描述信息放入稀疏表方便快速访问
  const vertex_input_binding_description *vert_in_binding_desc_map[1 << 8];
  {
    auto it = info.vertex_input_state.bindings;
    auto ed = it + info.vertex_input_state.bindings_count;
    for (; it != ed; ++it) {
      vert_in_binding_desc_map[it->binding] = it;
    }
  }

  {
    // 填充两个 [vertex_attribute_detail] 数组
    auto it = info.vertex_input_state.attributes;
    auto ed = it + info.vertex_input_state.attributes_count;
    int per_vert_cnt = 0, per_inst_cnt = 0;
    // 遍历每一个顶点属性，放到对应的数组
    for (; it != ed; ++it) {
      auto &binding = *vert_in_binding_desc_map[it->binding];
      if (binding.input_rate == vertex_input_rate::vertex) {
        m_vertex_input_per_vertex[per_vert_cnt++] = {
            .location = it->location,
            .binding = it->binding,
            .stride = binding.stride,
            .offset = it->offset,
        };
      } else {
        m_vertex_input_per_instance[per_inst_cnt++] = {
            .location = it->location,
            .binding = it->binding,
            .stride = binding.stride,
            .offset = it->offset,
        };
      }
    }
    m_counts.vertex_input_per_vertex = per_vert_cnt;
    m_counts.vertex_input_per_instance = per_inst_cnt;
  }

  {
    struct compare_weights {
      std::uint8_t location;
      std::uint32_t align;
      std::uint32_t size;
    } stage_attrs[1 << 8];

    auto vertex_output_cnt = vertex_shader_module.variables_meta.outputs_count;
    std::uint32_t chunk_size = 0;
    std::uint32_t chunk_align = 0;
    if (vertex_output_cnt) {
      {
        // 把类型信息放入待排序数组
        auto it = vertex_shader_module.variables_meta.outputs;
        auto ed = it + vertex_output_cnt;
        for (auto dst = stage_attrs; it != ed; ++it, ++dst) {
          *dst = {
              .location = it->location,
              .align = it->align,
              .size = it->size,
          };
        }

        // 按照 align 排序，则保证构造出来的结构成员间的 padding 和最小
        // 保证去除内存浪费
        std::sort(
            stage_attrs, stage_attrs + vertex_output_cnt,
            [](const compare_weights &a, const compare_weights &b) {
              return a.align < b.align;
            }
        );
      }

      for (auto it = stage_attrs, ed = stage_attrs + vertex_output_cnt; it != ed; ++it) {
        chunk_size = (chunk_size + it->align - 1) / it->align * it->align;
        // 先把成员偏移量放到记录数组内
        auto offset_ptr = reinterpret_cast<std::byte *>(chunk_size);
        for (auto &out : m_vertex_shader_output) {
          out[it->location] = offset_ptr;
        }
        m_fragment_shader_input[it->location] = offset_ptr;
        chunk_size += it->size;
      }

      chunk_align = stage_attrs[vertex_output_cnt - 1].align;
    }

    shader_stage_variable_description fragment_output_desc[1 << 8];
    auto fragment_output_cnt = fragment_shader_module.variables_meta.outputs_count;
    std::uint32_t fragment_output_size = 0;
    // 片元着色器输出的内存对齐
    std::uint32_t fragment_output_align = 0;
    if (fragment_output_cnt) {
      {
        auto src = fragment_shader_module.variables_meta.outputs;
        std::copy_n(src, fragment_output_cnt, fragment_output_desc);
      }

      using cmp_t = const shader_stage_variable_description;
      std::sort(
          fragment_output_desc, fragment_output_desc + fragment_output_cnt,
          [](cmp_t &a, cmp_t &b) { return a.align < b.align; }
      );

      for (auto it = fragment_output_desc, ed = it + fragment_output_cnt; it != ed; ++it) {
        fragment_output_size = (fragment_output_size + it->align - 1) / it->align * it->align;
        auto offset_ptr = reinterpret_cast<std::byte *>(fragment_output_size);
        m_fragment_shader_output[it->location] = offset_ptr;
        fragment_output_size += it->size;
      }
      fragment_output_align = fragment_output_desc[fragment_output_cnt - 1].align;
    }

    // 片元着色器输出在这块堆内存的偏移
    auto fragment_output_offset = (chunk_size * 4 + fragment_output_align - 1) /
                                  fragment_output_align * fragment_output_align;

    // 整个输出结构的字节大小
    auto allocating_size = fragment_output_offset + fragment_output_size;

    // 申请内存
    m_allocated_memory_align = std::align_val_t((std::max)(chunk_align, fragment_output_align));
    m_allocated_memory = reinterpret_cast<std::byte *>(
        ::operator new(allocating_size, m_allocated_memory_align)
    );

    {
      // 整个输出结构的偏移量
      auto offset = reinterpret_cast<std::ptrdiff_t>(m_allocated_memory);
      for (auto it = stage_attrs, ed = stage_attrs + vertex_output_cnt; it != ed; ++it) {
        for (auto &out : m_vertex_shader_output) {
          out[it->location] += offset;
          offset += chunk_size;
        }
        m_fragment_shader_input[it->location] += offset;
      }

      for (auto it = fragment_output_desc, ed = it + fragment_output_cnt; it != ed; ++it) {
        m_fragment_shader_output[it->location] += fragment_output_offset + offset;
      }
    }
  }

  m_counts.fragment_input = fragment_shader_module.variables_meta.inputs_count;
  m_counts.fragment_output = fragment_shader_module.variables_meta.outputs_count;

  {
    auto src = fragment_shader_module.variables_meta.inputs;
    auto src_ed = src + m_counts.fragment_input;
    std::transform(src, src_ed, m_fragment_input, [](const plaid::shader_stage_variable_description &d) {
      return fragment_input_detail{d.location, d.interpolation};
    });
  }

  {
    auto &subpass = info.render_pass.subpass(info.subpass);

    auto src = fragment_shader_module.variables_meta.outputs;
    auto src_ed = src + m_counts.fragment_output;
    std::transform(src, src_ed, m_fragment_output, [&](const plaid::shader_stage_variable_description &d) {
      auto &attachment = subpass.color_attachments[d.location];
      auto should_store = info.render_pass.attachment(attachment.id).store_op == attachment_store_op::store;
      return fragment_output_detail{
          .location = d.location,
          .attachment_id = attachment.id,
          .attachment_stride = format_size(attachment.format) * should_store,
          .attachment_transition = match_attachment_transition_function(d.format, attachment.format)};
    });
  }
}

graphics_pipeline_impl::~graphics_pipeline_impl() {
  ::operator delete(m_allocated_memory, m_allocated_memory_align);
}

static void clear_by_format(
    format src, format dst,
    std::byte *first, std::byte *last,
    const std::byte *val, std::uint32_t stride
) {
  if (src == dst) {
    for (; first != last; first += stride) {
      std::memcpy(first, val, stride);
    }
  } else {
    // 绑定布局转换函数
    auto trans = match_attachment_transition_function(src, dst);
    for (; first != last; first += stride) {
      trans(val, first);
    }
  }
}

void graphics_pipeline_impl::clear_color_attachment(const render_pass::state &state, attachment_reference ref) {
  // 根据附件类型选定清除值
  auto src_format = format::undefined;
  const std::byte *src = nullptr;
  if (is_float_format(ref.format)) {
    src_format = format::RGBA32f;
    src = reinterpret_cast<const std::byte *>(&state.m_clear_values[ref.id].color.f);
  } else if (is_unsigned_integer_format(ref.format)) {
    src_format = format::RGBA32u;
    src = reinterpret_cast<const std::byte *>(&state.m_clear_values[ref.id].color.u);
  }

  auto dst_stride = format_size(ref.format);
  auto dst = state.m_frame_buffer->attachement(ref.id);
  auto dst_ed = dst + state.m_frame_buffer->width() * state.m_frame_buffer->height() * dst_stride;
  clear_by_format(src_format, ref.format, dst, dst_ed, src, dst_stride);
}

void graphics_pipeline_impl::clear_depth_attachment(const render_pass::state &state, attachment_reference ref) {
  // 根据附件类型选定清除值
  auto src_format = format::undefined;
  const std::byte *src = nullptr;
  if (is_float_format(ref.format)) {
    src_format = format::R32f;
    src = reinterpret_cast<const std::byte *>(&state.m_clear_values[ref.id].depth_stencil.depth);
  }

  auto dst = state.m_frame_buffer->attachement(ref.id);
  auto dst_stride = format_size(ref.format);
  auto dst_ed = dst + state.m_frame_buffer->width() * state.m_frame_buffer->height() * dst_stride;
  clear_by_format(src_format, ref.format, dst, dst_ed, src, dst_stride);
}

void graphics_pipeline_impl::draw(
    const render_pass::state &state,
    std::uint32_t vertex_count, std::uint32_t instance_count,
    std::uint32_t first_vertex, std::uint32_t first_instance
) {
  auto width = state.m_frame_buffer->width();
  auto height = state.m_frame_buffer->height();
  [[unlikely]] if (!width || !height) {
    return;
  }

  {
    // 对所有颜色附件应用清除值
    auto it = state.m_current_subpass->color_attachments;
    auto ed = it + state.m_current_subpass->color_attachments_count;
    for (; it != ed; ++it) {
      auto &desc = state.m_attachment_descriptions[it->id];
      if (desc.load_op == attachment_load_op::clear) {
        clear_color_attachment(state, *it);
      }
    }
  }

  {
    // 对深度附件应用清除值
    auto ref = *state.m_current_subpass->depth_stencil_attachment;
    auto &desc = state.m_attachment_descriptions[ref.id];
    if (desc.stencil_load_op == attachment_load_op::clear) {
      clear_depth_attachment(state, ref);
    }
  }

  auto last_vert = first_vertex + vertex_count;
  auto last_inst = first_instance + instance_count;
  switch (vertex_assembly) {
    case primitive_topology::triangle_list:
      draw_triangle_list(state, first_vertex, last_vert, first_instance, last_inst);
      break;
    case primitive_topology::triangle_strip:
      draw_triangle_strip(state, first_vertex, last_vert, first_instance, last_inst);
      break;
    case primitive_topology::line_strip:
      throw std::runtime_error("Unsupported topology line_strip.");
      break;
  }
}

static vec4 line_insertion(const vec4 &l, const vec4 &a, const vec4 &b) {
  auto da = dot(a, l), db = dot(b, l);
  auto weight = da / (da - db);
  return a * (1 - weight) + b * weight;
}

static int clip_triangle(const vec4 (&src)[3], vec4 dst[]) {
  static constexpr vec4 clip_planes[]{
      // near
      {0, 0, 1, 0},
      // far
      {0, 0, -1, 1},
      // left
      {1, 0, 0, 1},
      // right
      {-1, 0, 0, 1},
      // top
      {0, 1, 0, 1},
      // bottom
      {0, -1, 0, 1},
  };

  vec4 queue[2][6];
  {
    auto it = queue[0];
    for (auto &v : src) {
      *it = v;
      ++it;
    }
  }
  int cnt[2]{3};
  int pre = 0;
  for (auto &clip : clip_planes) {
    auto now = pre ^ 1;
    cnt[now] = 0;
    for (int i = 0; i != cnt[pre]; ++i) {
      auto &current = queue[pre][i];
      auto &previous = queue[pre][(i + cnt[pre] - 1) % cnt[pre]];
      if (dot(clip, current) >= 0) {
        if (dot(clip, previous) < 0) {
          queue[now][cnt[now]++] = line_insertion(clip, previous, current);
        }
        queue[now][cnt[now]++] = current;
      } else if (dot(clip, previous) >= 0) {
        queue[now][cnt[now]++] = line_insertion(clip, previous, current);
      }
    }
    pre = now;
  }

  std::copy_n(queue[pre], cnt[pre], dst);
  return cnt[pre];
}

void graphics_pipeline_impl::draw_triangle_list(
    const render_pass::state &state,
    std::uint32_t first_vert, std::uint32_t last_vert,
    std::uint32_t first_inst, std::uint32_t last_inst
) {
  [[unlikely]] if (last_vert - first_inst <= 2) {
    return;
  }

  auto &vertex_buffer = state.m_vertex_buffer;

  vec4 clip_coords[3];
  // 采用 IMR 模式，每当完成相邻三个顶点的顶点着色器之后马上进行图元的光栅化
  // 顶点属性的更新采用如下方法：分别使用 [vertex_input_per_vertex_attributes]
  // 和 [vertex_input_per_instance_attributes] 记录顶点属性在顶点缓冲区的位置，
  // 绘制同一实例的不同顶点，只需要更新逐顶点数据
  for (auto inst = first_inst; inst != last_inst; ++inst) {
    obtain_next_instance_attributes(vertex_buffer, inst);

    std::uint32_t indices[]{0, 1, 2};

    vec4 clipped[6];
    vec4 *target[3];
    target[0] = clipped;
    while (1) {
      auto it = m_vertex_shader_output;
      auto coord_it = clip_coords;
      for (auto i : indices) {
        obtain_next_vertex_attribute(vertex_buffer, i);
        invoke_vertex_shader(
            state.m_descriptor_set,
            *it,
            *coord_it
        );
        ++it, ++coord_it;
      }

      auto vertex_cnt = clip_triangle(clip_coords, clipped);
      for (auto i = 1; i <= vertex_cnt - 2; ++i) {
        target[1] = clipped + i;
        target[2] = clipped + i + 1;
        rasterize_triangle(state, target);
      }

      auto start = indices[2];
      if (start + 3 >= last_vert) break;
      indices[0] = start + 1;
      indices[1] = start + 2;
      indices[2] = start + 3;
    }
  }
}

void graphics_pipeline_impl::draw_triangle_strip(
    const render_pass::state &state,
    std::uint32_t first_vert, std::uint32_t last_vert,
    std::uint32_t first_inst, std::uint32_t last_inst
) {
  [[unlikely]] if (last_vert - first_inst <= 2) {
    return;
  }

  auto &vertex_buffer = state.m_vertex_buffer;

  vec4 clip_coords[3];
  // 采用 IMR 模式，每当完成相邻三个顶点的顶点着色器之后马上进行图元的光栅化
  // 顶点属性的更新采用如下方法：分别使用 [vertex_input_per_vertex_attributes]
  // 和 [vertex_input_per_instance_attributes] 记录顶点属性在顶点缓冲区的位置，
  // 绘制同一实例的不同顶点，只需要更新逐顶点数据
  for (auto inst = first_inst; inst != last_inst; ++inst) {
    obtain_next_instance_attributes(vertex_buffer, inst);

    std::uint32_t indices[]{0, 1, 2};
    // 需要先单独处理前三个顶点的顶点着色器
    for (auto i : indices) {
      // 顶点编号刚好对应数据位置，而下面的 while 循环则不能如此
      obtain_next_vertex_attribute(vertex_buffer, i);
      invoke_vertex_shader(state.m_descriptor_set, m_vertex_shader_output[i], clip_coords[i]);
    }

    vec4 clipped[6];
    vec4 *target[3];
    target[0] = clipped;

    int ping_pong = 0;
    while (1) {

      auto vertex_cnt = clip_triangle(clip_coords, clipped);
      for (auto i = 1; i <= vertex_cnt - 2; ++i) {
        target[1] = clipped + i;
        target[2] = clipped + i + 1;
        rasterize_triangle(state, target);
      }

      indices[ping_pong] += 3;
      if (indices[ping_pong] == last_vert) break;
      obtain_next_vertex_attribute(vertex_buffer, indices[ping_pong]);
      invoke_vertex_shader(
          state.m_descriptor_set,
          m_vertex_shader_output[ping_pong],
          clip_coords[ping_pong]
      );
      ping_pong = (ping_pong + 1) % 3;
    }
  }
}

void graphics_pipeline_impl::obtain_next_vertex_attribute(
    const const_memory_array<1 << 8> &vertex_buffer, std::uint32_t vert_id
) {
  auto it = m_vertex_input_per_vertex;
  auto ed = it + m_counts.vertex_input_per_vertex;
  for (; it != ed; ++it) {
    auto ptr = vertex_buffer[it->binding] + it->stride * vert_id + it->offset;
    m_vertex_shader_input[it->location] = ptr;
  }
}

void graphics_pipeline_impl::obtain_next_instance_attributes(
    const const_memory_array<1 << 8> &vertex_buffer, std::uint32_t inst_id
) {
  auto it = m_vertex_input_per_instance;
  auto ed = it + m_counts.vertex_input_per_instance;
  for (; it != ed; ++it) {
    auto ptr = vertex_buffer[it->binding] + it->stride * inst_id + it->offset;
    m_vertex_shader_input[it->location] = ptr;
  }
}

void graphics_pipeline_impl::invoke_vertex_shader(
    const const_memory_array<1 << 8> &descriptor_set,
    const memory_array<1 << 8> &output,
    vec4 &clip_coord
) {
  // 只有一个内置变量，即裁剪空间坐标
  auto mutable_builtin = reinterpret_cast<memory>(&clip_coord);
  m_vertex_shader(descriptor_set, m_vertex_shader_input, output, &mutable_builtin);
}

void graphics_pipeline_impl::rasterize_triangle(
  const render_pass::state &state,
  const vec4 *const (&clip_coord)[3]
) {
  auto frame = state.m_frame_buffer;
  auto width = frame->width();
  auto height = frame->height();

  vec2 view[3];
  float z[3];
  {
    auto *view_it = view;
    auto *z_it = z;
    for (auto v : clip_coord) {
      // CLIP -> NDC -> VIEW
      // [-w, w] -> [-1, 1] -> [0, width]
      view_it->x = (v->x / v->w + 1.f) / 2 * width;
      // [-w, w] -> [-1, 1] -> [0, height]
      view_it->y = (v->y / v->w + 1.f) / 2 * height;
      // [0, w] -> [0, 1]
      *z_it = v->z / v->w;
      ++view_it;
      ++z_it;
    }
  }

  auto ab = view[1] - view[0];
  auto ac = view[2] - view[0];

  {
    /// 面剔除
    vec3 ab3 = {ab.x, ab.y, z[1] - z[0]};
    vec3 ac3 = {ac.x, ac.y, z[2] - z[0]};
    if (dot(cross(ab3, ac3), {0, 0, 1}) >= 0) {
      return;
    }
  }

  auto l = width - 1, t = height - 1, r = 0u, b = 0u;
  for (auto &v : view) {
    l = (std::min)(l, static_cast<decltype(l)>(v.x));
    t = (std::min)(t, static_cast<decltype(l)>(v.y));
    r = (std::max)(r, static_cast<decltype(l)>(v.x));
    b = (std::max)(b, static_cast<decltype(l)>(v.y));
  }
  // 有些三角形裁剪之后，顶点刚好落在边缘上，上面算出来会超出帧缓冲范围
  r = (std::min)(r, width - 1);
  b = (std::min)(b, height - 1);

  auto m = cross(ab, ac);
  if (!m) {
    return;
  }
  m = 1 / m;

  auto pa = view[0] - vec2{l + .5f, t + .5f};
  auto um = cross(ac, pa);
  auto vm = cross(pa, ab);

  auto depth_stencil_attachment = frame->attachement(
      state.m_current_subpass->depth_stencil_attachment->id
  );

  for (auto y = t; y <= b; ++y) {
    auto um_first = um, vm_first = vm;
    for (auto x = l; x <= r; ++x) {
      auto u = um * m, v = vm * m;
      if (u >= 0 && v >= 0 && u + v <= 1) {
        auto p = 1 - u - v;
        auto k = 1 / (p * z[0] + u * z[1] + v * z[2]);
        float weight[3]{
            p * z[0] * k,
            u * z[1] * k,
            v * z[2] * k,
        };
        auto cz = z[0] * weight[0] + z[1] * weight[1] + z[2] * weight[2];
        auto pre_z = reinterpret_cast<float *>(depth_stencil_attachment);
        pre_z += y * width + x;
        if (cz < *pre_z) {
          *pre_z = cz;
          invoke_fragment_shader(state, {float(x), float(y), cz}, y * width + x, weight);
        }
      }
      um += ac.y;
      vm -= ab.y;
    }
    um = um_first - ac.x;
    vm = vm_first + ab.x;
  }
}

void graphics_pipeline_impl::invoke_fragment_shader(
    const render_pass::state &state,
    vec3 fragcoord,
    std::uint32_t index, const float (&weight)[3]
) {
  {
    auto it = m_fragment_input, ed = it + m_counts.fragment_input;
    for (; it != ed; ++it) {
      auto location = it->location;
      const std::byte *src[3] = {
          m_vertex_shader_output[0][location],
          m_vertex_shader_output[1][location],
          m_vertex_shader_output[2][location],
      };
      it->interpolation(src, weight, m_fragment_shader_input[location]);
    }
  }
  auto mutable_builtin = reinterpret_cast<memory>(&fragcoord);
  m_fragment_shader(
      state.m_descriptor_set, m_fragment_shader_input,
      m_fragment_shader_output, &mutable_builtin
  );

  auto frame = state.m_frame_buffer;
  auto it = m_fragment_output, ed = it + m_counts.fragment_output;
  for (; it != ed; ++it) {
    if (!it->attachment_stride) {
      continue;
    }
    auto ptr = frame->attachement(it->attachment_id) + index * it->attachment_stride;
    it->attachment_transition(m_fragment_shader_output[it->location], ptr);
  }
}
