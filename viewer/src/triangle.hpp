#include <plaid/shader.h>

namespace triangle {

struct vert : plaid::vertex_shader {
  location<0>::in<plaid::vec2> position;
  location<1>::in<plaid::vec3> color;

  location<0>::out<plaid::vec3> frag_color;

  void main() {
    auto pos = position.get(this);
    gl_position = {pos.x, pos.y, 0, 1};
    frag_color.get(this) = color.get(this);
  }
};

struct frag : plaid::fragment_shader {

  location<0>::in<plaid::vec3> frag_color;
  location<0>::out<plaid::vec3> final_color;

  void main() {
    final_color.get(this) = frag_color.get(this);
  }
};

} // namespace triangle
