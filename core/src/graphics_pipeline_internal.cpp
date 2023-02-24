#include <algorithm>
#include <stdexcept>

#include "graphics_pipeline_internal.h"

using namespace plaid;

graphics_pipeline::graphics_pipeline(const graphics_pipeline::create_info &info) {
  ptr = new graphics_pipeline_impl(info);
}

graphics_pipeline::~graphics_pipeline() {
  if (ptr) {
    delete ptr;
  }
}

graphics_pipeline_impl::graphics_pipeline_impl(const graphics_pipeline::create_info &info) {
  vertex_assembly = info.input_assembly_state.topology;

  // 获取顶点着色器入口函数
  vertex_shader = info.shader_stage.vertex_shader->entry;

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

  auto vs_output_cnt = info.shader_stage.vertex_shader->variables_meta.outputs_count;
  if (vs_output_cnt) {
    // 为顶点着色器输出分配内存

    struct compare_weights {
      std::uint8_t location;
      std::uint32_t align;
      std::uint32_t size;
    } sorted_attrs[1 << 8];

    {
      // 把类型信息放入待排序数组
      auto it = info.shader_stage.vertex_shader->variables_meta.outputs;
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

    std::size_t output_size = 0;
    for (auto it = sorted_attrs, ed = sorted_attrs + vs_output_cnt; it != ed; ++it) {
      output_size = (output_size + it->align - 1) / it->align * it->align;
      for (auto &o : vertex_shader_output) {
        // 先把成员偏移量放到记录数组内
        o[it->location] = reinterpret_cast<std::byte *>(output_size);
      }
      output_size += it->size;
    }

    // 申请内存
    vertex_shader_output_resource_align = std::align_val_t(sorted_attrs[vs_output_cnt - 1].align);
    vertex_shader_output_resource = reinterpret_cast<std::byte *>(
        ::operator new(output_size * 3, vertex_shader_output_resource_align)
    );

    for (auto it = sorted_attrs, ed = sorted_attrs + vs_output_cnt; it != ed; ++it) {
      // 整个输出结构的偏移量
      auto offset = reinterpret_cast<std::ptrdiff_t>(vertex_shader_output_resource);
      for (auto &o : vertex_shader_output) {
        o[it->location] += offset;
        offset += output_size;
      }
    }
  }
}

graphics_pipeline_impl::~graphics_pipeline_impl() {
  ::operator delete(vertex_shader_output_resource, vertex_shader_output_resource_align);
}

void graphics_pipeline_impl::draw(
    render_pass_impl &render_pass,
    const std::byte *(&vertex_buffer)[1 << 8],
    std::uint32_t vertex_count, std::uint32_t instance_count,
    std::uint32_t first_vertex, std::uint32_t first_instance
) {
  auto last_vert = first_vertex + vertex_count;
  auto last_inst = first_instance + instance_count;
  switch (vertex_assembly) {
    case primitive_topology::triangle_strip:
      draw_triangle_strip(render_pass, vertex_buffer, first_vertex, last_vert, first_instance, last_inst);
      break;
    case primitive_topology::line_strip:
      throw std::runtime_error("Unsupported topology line_strip.");
      break;
  }
}

void graphics_pipeline_impl::draw_triangle_strip(
    render_pass_impl &render_pass,
    const std::byte *(&vertex_buffer)[1 << 8],
    std::uint32_t first_vert, std::uint32_t last_vert,
    std::uint32_t first_inst, std::uint32_t last_inst
) {
  [[unlikely]] if (last_vert - first_inst <= 2) {
    return;
  }

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
      invoke_vertex_shader(i, clip_coords[i]);
    }

    // TODO：这里还需要确定顶点顺序，使得面剔除能够正确执行
    // （虽然现在也还没有写面剔除
    int ping_pong = 0;
    while (1) {
      rasterize_triangle(clip_coords);

      indices[ping_pong] += 3;
      if (indices[ping_pong] == last_vert) break;
      obtain_next_vertex_attribute(vertex_buffer, indices[ping_pong]);
      invoke_vertex_shader(ping_pong, clip_coords[ping_pong]);
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

void graphics_pipeline_impl::invoke_vertex_shader(std::uint8_t dst, vec4 &clip_coord) {
  // 只有一个内置变量，即裁剪空间坐标
  auto mutable_builtin = reinterpret_cast<std::byte *>(&clip_coord);
  vertex_shader(descriptor_set_map, vertex_shader_input, vertex_shader_output[dst], &mutable_builtin);
}

void graphics_pipeline_impl::rasterize_triangle(vec4 (&clip_coord)[3]) {
}
