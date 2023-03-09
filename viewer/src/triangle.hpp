#include <plaid/shader.h>

namespace triangle {

struct vert : plaid::vertex_shader {

  binding<0>::uniform<plaid::vec3[]> positions;
  binding<1>::uniform<plaid::mat4x4> mvp;

  location<0>::in<std::uint32_t> pos_index;

  void main() {
    auto i = get(pos_index);
    auto pos = get(positions)[i];
    *gl_position = get(mvp) * plaid::vec4{pos.x, pos.y, pos.z, 1};
  }
};

struct frag : plaid::fragment_shader {

  location<0>::out<plaid::vec3> final_color;

  void main() {
    get(final_color) = plaid::vec3 {1, 1, 1} * gl_fragcoord->z;
  }
};

} // namespace triangle
