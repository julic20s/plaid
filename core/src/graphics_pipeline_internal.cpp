#include <algorithm>
#include <stdexcept>

#include <plaid/frame_buffer.h>

#include "graphics_pipeline_internal.h"

using namespace plaid;

graphics_pipeline::graphics_pipeline(const graphics_pipeline::create_info &info) {
  m_pointer = new graphics_pipeline_impl(info);
}

graphics_pipeline::~graphics_pipeline() {
  if (m_pointer) {
    delete m_pointer;
  }
}

graphics_pipeline_impl::graphics_pipeline_impl(const graphics_pipeline::create_info &info) {
  vertex_assembly = info.input_assembly_state.topology;

  auto &vertex_shader_module = info.shader_stage.vertex_shader;
  auto &fragment_shader_module = info.shader_stage.fragment_shader;

  // 获取顶点着色器入口函数
  vertex_shader = vertex_shader_module.entry;
  fragment_shader = fragment_shader_module.entry;

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
        vertex_input_per_vertex_attributes[per_vert_cnt++] = {
            .location = it->location,
            .binding = it->binding,
            .stride = binding.stride,
            .offset = it->offset,
        };
      } else {
        vertex_input_per_instance_attributes[per_inst_cnt++] = {
            .location = it->location,
            .binding = it->binding,
            .stride = binding.stride,
            .offset = it->offset,
        };
      }
    }
    vertex_input_per_vertex_attributes_count = per_vert_cnt;
    vertex_input_per_instance_attributes_count = per_inst_cnt;
  }

  {
    // 为顶点着色器输出分配内存

    struct compare_weights {
      std::uint8_t location;
      std::uint32_t align;
      std::uint32_t size;
    } sorted_attrs[1 << 8];

    auto vs_output_cnt = vertex_shader_module.variables_meta.outputs_count;

    {
      // 把类型信息放入待排序数组
      auto it = vertex_shader_module.variables_meta.outputs;
      auto ed = it + vs_output_cnt;
      for (auto dst = sorted_attrs; it != ed; ++it, ++dst) {
        *dst = {
            .location = it->location,
            .align = it->align,
            .size = it->size,
        };
      }

      // 按照 align 排序，则保证构造出来的结构成员间的 padding 和最小
      // 保证去除内存浪费
      std::sort(
          sorted_attrs, sorted_attrs + vs_output_cnt,
          [](const compare_weights &a, const compare_weights &b) {
            return a.align < b.align;
          }
      );
    }

    shader_stage_variable_description fragment_shader_output_desc[1 << 8];
    auto fs_out_cnt = fragment_shader_module.variables_meta.outputs_count;
    {
      auto it = fragment_shader_module.variables_meta.outputs;
      auto ed = it + fs_out_cnt;
      auto out_it = fragment_shader_output_desc;
      for (; it != ed; ++it, ++out_it) {
        *out_it = *it;
      }
      using cmp_t = const shader_stage_variable_description;
      std::sort(
          fragment_shader_output_desc, fragment_shader_output_desc + fs_out_cnt,
          [](cmp_t &a, cmp_t &b) { return a.align < b.align; }
      );
    }

    std::size_t vertex_shader_output_size = 0;

    for (auto it = sorted_attrs, ed = sorted_attrs + vs_output_cnt; it != ed; ++it) {
      vertex_shader_output_size = (vertex_shader_output_size + it->align - 1) / it->align * it->align;
      // 先把成员偏移量放到记录数组内
      auto offset_ptr = reinterpret_cast<std::byte *>(vertex_shader_output_size);
      for (auto &out : vertex_shader_output) {
        out[it->location] = offset_ptr;
      }
      fragment_shader_input[it->location] = offset_ptr;
      vertex_shader_output_size += it->size;
    }

    std::size_t fragment_shader_output_size = 0;
    for (auto it = fragment_shader_output_desc, ed = it + fs_out_cnt; it != ed; ++it) {
      fragment_shader_output_size = (fragment_shader_output_size + it->align - 1) / it->align * it->align;
      auto offset_ptr = reinterpret_cast<std::byte *>(fragment_shader_output_size);
      fragment_shader_output[it->location] = offset_ptr;
      fragment_shader_output_size += it->size;
    }

    // 片元着色器输出的内存对齐
    auto fs_align = fragment_shader_output_desc[fs_out_cnt - 1].align;
    // 片元着色器输出在这块堆内存的偏移
    auto fs_offset = (vertex_shader_output_size * 4 + fs_align - 1) / fs_align * fs_align;
    // 整个输出结构的内存对齐
    auto tot_align = (std::max)(fs_align, sorted_attrs[vs_output_cnt - 1].align);
    // 整个输出结构的字节大小
    auto tot_size = fs_offset + fragment_shader_output_size;

    // 申请内存
    stage_shader_variables_resource_align = std::align_val_t(tot_align);
    stage_shader_variables_resource = reinterpret_cast<std::byte *>(
        ::operator new(tot_size, stage_shader_variables_resource_align)
    );

    for (auto it = sorted_attrs, ed = sorted_attrs + vs_output_cnt; it != ed; ++it) {
      // 整个输出结构的偏移量
      auto offset = reinterpret_cast<std::ptrdiff_t>(stage_shader_variables_resource);
      for (auto &o : vertex_shader_output) {
        o[it->location] += offset;
        offset += vertex_shader_output_size;
      }
      fragment_shader_input[it->location] += offset;
    }

    for (auto it = fragment_shader_output_desc, ed = it + fs_out_cnt; it != ed; ++it) {
      auto offset = reinterpret_cast<std::ptrdiff_t>(stage_shader_variables_resource);
      fragment_shader_output[it->location] += fs_offset + offset;
    }
  }

  {
    // 把片元着色器的输入结构保存下来
    fragment_attributes_count = fragment_shader_module.variables_meta.inputs_count;
    auto it = fragment_shader_module.variables_meta.inputs;
    auto ed = it + fragment_attributes_count;
    auto attr_it = fragment_attributes;
    for (; it != ed; ++it) {
      attr_it->location = it->location;
      attr_it->interpolation = it->interpolation;
    }
  }

  {
    auto &subpass = info.render_pass.subpass(info.subpass);
    fragment_out_count = fragment_shader_module.variables_meta.outputs_count;
    auto it = fragment_shader_module.variables_meta.outputs;
    auto ed = it + fragment_out_count;
    auto out_it = fragment_shader_out_details;
    for (; it != ed; ++it, ++out_it) {
      auto &attachment = subpass.color_attachments[it->location];
      out_it->location = it->location;
      out_it->attachment_id = attachment.id;
      out_it->attachment_stride = format_size(attachment.format);
      out_it->attachment_transition = match_attachment_transition_function(it->format, attachment.format);
    }
  }
}

void rgb323232_float_to_rgb888_integer(
    const std::byte *src, std::byte *dst
) {
  auto final_color = reinterpret_cast<const vec3 *>(src);
  auto r = std::uint32_t(final_color->x * 0xff);
  auto g = std::uint32_t(final_color->y * 0xff);
  auto b = std::uint32_t(final_color->z * 0xff);
  *reinterpret_cast<std::uint32_t *>(dst) = r << 16 | g << 8 | b;
}

graphics_pipeline_impl::fragment_shader_out_detail::attachment_transition_function *
graphics_pipeline_impl::match_attachment_transition_function(format src, format dst) {
  if (src == format::rgb323232_float) {
    if (dst == format::rgb888_integer) {
      return rgb323232_float_to_rgb888_integer;
    }
  }
  return nullptr;
}

graphics_pipeline_impl::~graphics_pipeline_impl() {
  ::operator delete(stage_shader_variables_resource, stage_shader_variables_resource_align);
}

void graphics_pipeline_impl::draw(
    render_pass::state &state,
    std::uint32_t vertex_count, std::uint32_t instance_count,
    std::uint32_t first_vertex, std::uint32_t first_instance
) {
  auto last_vert = first_vertex + vertex_count;
  auto last_inst = first_instance + instance_count;
  switch (vertex_assembly) {
    case primitive_topology::triangle_strip:
      draw_triangle_strip(state, first_vertex, last_vert, first_instance, last_inst);
      break;
    case primitive_topology::line_strip:
      throw std::runtime_error("Unsupported topology line_strip.");
      break;
  }
}

constexpr vec4 clip_planes[]{
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {-1, 0, .5f, 1},
    {1, 0, .5f, 1},
    {0, -1, .5f, 1},
    {0, 1, .5f, 1},
};

static vec4 line_insertion(const vec4 &l, const vec4 &a, const vec4 &b) {
  auto da = dot(a, l), db = dot(b, l);
  auto weight = da / (da - db);
  return a * weight + b * (1 - weight);
}

static int clip_triangle(const vec4 (&src)[3], vec4 dst[]) {
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
      auto &previous = queue[pre][(i + 2) % 3];
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

  for (auto it = queue[pre], ed = it + cnt[pre]; it != ed; ++it, ++dst) {
    *dst = *it;
  }

  return cnt[pre];
}

void graphics_pipeline_impl::draw_triangle_strip(
    render_pass::state &state,
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
      invoke_vertex_shader(state.m_descriptor_set, vertex_shader_output[i], clip_coords[i]);
    }

    vec4 clipped[6];
    vec4 *target[3];
    target[0] = clipped;

    // TODO：这里还需要确定顶点顺序，使得面剔除能够正确执行
    // （虽然现在也还没有写面剔除
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
          vertex_shader_output[ping_pong],
          clip_coords[ping_pong]
      );
      ping_pong = (ping_pong + 1) % 3;
    }
  }
}

void graphics_pipeline_impl::obtain_next_vertex_attribute(
    const std::byte *(&vertex_buffer)[1 << 8], std::uint32_t vert_id
) {
  auto it = vertex_input_per_vertex_attributes;
  auto ed = it + vertex_input_per_vertex_attributes_count;
  for (; it != ed; ++it) {
    auto ptr = vertex_buffer[it->binding] + it->stride * vert_id + it->offset;
    vertex_shader_input[it->location] = ptr;
  }
}

void graphics_pipeline_impl::obtain_next_instance_attributes(
    const std::byte *(&vertex_buffer)[1 << 8], std::uint32_t inst_id
) {
  auto it = vertex_input_per_instance_attributes;
  auto ed = it + vertex_input_per_instance_attributes_count;
  for (; it != ed; ++it) {
    auto ptr = vertex_buffer[it->binding] + it->stride * inst_id + it->offset;
    vertex_shader_input[it->location] = ptr;
  }
}

void graphics_pipeline_impl::invoke_vertex_shader(
    const std::byte *(&descriptor_set)[1 << 8],
    std::byte *(&output)[1 << 8],
    vec4 &clip_coord
) {
  // 只有一个内置变量，即裁剪空间坐标
  auto mutable_builtin = reinterpret_cast<std::byte *>(&clip_coord);
  vertex_shader(descriptor_set, vertex_shader_input, output, &mutable_builtin);
}

void graphics_pipeline_impl::rasterize_triangle(render_pass::state &state, const vec4 *const (&clip_coord)[3]) {
  auto width = state.m_frame_buffer->width();
  auto height = state.m_frame_buffer->height();
  [[unlikely]] if (!width || !height) {
    return;
  }

  vec2 view[3];
  float z[3];
  {
    auto *view_it = view;
    auto *z_it = z;
    for (auto v : clip_coord) {
      view_it->x = (v->x / v->w + 1.f) / 2 * width;
      view_it->y = (v->y / v->w + 1.f) / 2 * height;
      *z_it = v->z / v->w;
      ++view_it;
      ++z_it;
    }
  }

  auto l = width - 1, t = height - 1, r = 0u, b = 0u;
  for (auto &v : view) {
    l = (std::min)(l, static_cast<decltype(l)>(v.x));
    t = (std::min)(t, static_cast<decltype(l)>(v.y));
    r = (std::max)(r, static_cast<decltype(l)>(v.x));
    b = (std::max)(b, static_cast<decltype(l)>(v.y));
  }

  auto ab = view[1] - view[0];
  auto ac = view[2] - view[0];
  auto m = cross(ab, ac);
  if (!m) {
    return;
  }
  m = 1 / m;

  auto pa = view[0] - vec2{l + .5f, t + .5f};
  auto um = cross(ac, pa);
  auto vm = cross(pa, ab);

  auto depth_stencil_attachment = state.m_frame_buffer->attachement(
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
        if (cz > *pre_z) {
          *pre_z = cz;
          invoke_fragment_shader(state, y * width + x, weight);
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
    render_pass::state &state,
    std::uint32_t index, const float (&weight)[3]
) {
  {
    auto it = fragment_attributes, ed = it + fragment_attributes_count;
    for (; it != ed; ++it) {
      auto location = it->location;
      const std::byte *src[3] = {
          vertex_shader_output[0][location],
          vertex_shader_output[1][location],
          vertex_shader_output[2][location],
      };
      it->interpolation(src, weight, fragment_shader_input[location]);
    }
  }
  using const_input = const std::byte *(&)[1 << 8];
  auto &compat_fragment_shader_input = const_cast<const_input>(fragment_shader_input);
  fragment_shader(
      state.m_descriptor_set, compat_fragment_shader_input,
      fragment_shader_output, nullptr
  );

  auto frame_buffer = state.m_frame_buffer;
  auto it = fragment_shader_out_details, ed = it + fragment_out_count;
  for (; it != ed; ++it) {
    auto ptr = frame_buffer->attachement(it->attachment_id) + index * it->attachment_stride;
    it->attachment_transition(fragment_shader_output[it->location], ptr);
  }
}
