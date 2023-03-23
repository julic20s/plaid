#include "validate.h"

using namespace plaid::json;
using namespace plaid::viewer::gltf;

struct member_field {
  std::string_view name;
  void (*func)(value);
  bool required;
};

namespace validates {

void accessors(value val) {
}

void animations(value val) {
}

void asset(value val) {
}

void buffers(value val) {
}

void buffer_views(value val) {
}

void cameras(value val) {
}

void images(value val) {
}

void materials(value val) {
}

void meshes(value val) {
}

void nodes(value val) {
}

void samplers(value val) {
}

void scene(value val) {
}

void scenes(value val) {
}

void skins(value val) {
}

void textures(value val) {
}

} // namespace validates

void plaid::viewer::gltf::validate(const dom &gltf) {
  if (!gltf.is_object()) {
    throw format_error("gltf must an json object.");
  }

  constexpr member_field fields[] = {
      {"accessors", validates::accessors, false},
      {"animations", validates::animations, false},
      {"asset", validates::asset, true},
      {"buffers", validates::buffers, false},
      {"bufferViews", validates::buffer_views, false},
      {"cameras", validates::cameras, false},
      {"images", validates::images, false},
      {"materials", validates::materials, false},
      {"meshes", validates::meshes, false},
      {"nodes", validates::nodes, false},
      {"samplers", validates::samplers, false},
      {"scene", validates::scene, false},
      {"scenes", validates::scenes, false},
      {"skins", validates::skins, false},
      {"textures", validates::textures, false},
  };

  auto members = gltf.to_member_span();
  auto it = members.begin();
  for (auto &fd : fields) {
    while (it != members.end() && fd.name > std::string_view(it->key)) {
      ++it;
    }

    if (it != members.end() && fd.name == std::string_view(it->key)) {
      fd.func(it->value);
      ++it;
    } else if (fd.required) {
      throw format_error(std::string(fd.name) + " is a required field.");
    }
  }
}
