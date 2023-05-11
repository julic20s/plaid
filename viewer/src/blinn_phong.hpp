#include <plaid/shader.h>

namespace blinn_phong {

struct vert : plaid::vertex_shader {

  binding<0>::uniform<plaid::vec3[]> positions;
  binding<1>::uniform<plaid::vec3[]> normals;
  binding<2>::uniform<plaid::mat4> mvp;

  location<0>::in<std::int32_t> pos_index;

  location<1>::out<plaid::vec3> normal;

  void main() {
    auto i = get(pos_index);
    auto pos = get(positions)[i];
    *gl_position = get(mvp) * plaid::vec4{pos.x, pos.y, pos.z, 1};

    get(normal) = get(normals)[i];
  }
};

struct frag : plaid::fragment_shader {

  binding<3>::uniform<plaid::vec3> view;

  location<1>::in<plaid::vec3> normal;

  location<0>::out<plaid::vec3> final_color;

  void main() {
    constexpr plaid::vec3 ambient = {1, 1, 1};
    constexpr plaid::vec3 light_color = {1, 1, 1};
    constexpr plaid::vec3 light = {0, 0, -1};

    auto n = plaid::norm(get(normal));
    auto half = plaid::norm(get(view) + n);

    auto diffuse = (std::max)(0.f, dot(n, light)) * light_color;
    auto specular = (std::max)(0.f, dot(half, light)) * light_color;
    get(final_color) = ambient / 3 +
                       diffuse / 3 +
                       specular / 3;
  }
};

} // namespace blinn_phong
