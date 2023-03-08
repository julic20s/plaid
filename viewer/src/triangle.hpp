#include <plaid/shader.h>

namespace triangle {

struct vert : plaid::vertex_shader {

  binding<0>::uniform<plaid::vec3[]> positions;

  location<0>::in<std::uint32_t> pos_index;

  void main() {
    auto i = get(pos_index);
    auto pos = get(positions)[i];
    *gl_position = plaid::vec4{pos.x, pos.y, pos.z, 50};
  }
};

struct frag : plaid::fragment_shader {

  location<0>::out<plaid::vec3> final_color;

  void main() {
    get(final_color) = {1, 1, 1};
  }
};

} // namespace triangle
