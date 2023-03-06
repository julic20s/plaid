#include <fstream>
#include <iosfwd>
#include <sstream>
#include <string>
#include <stdexcept>

#include "obj_model.h"

obj_model::obj_model(const char *file) {
  std::ifstream stream(file);

  std::string line;
  char trash;
  while (stream) {
    std::getline(stream, line);
    [[unlikely]] if (line.starts_with("s ") || line.starts_with("g ") || line.starts_with('#') || line.starts_with("mtllib ")) {
      continue;
    }

    std::istringstream buf(line.c_str());
    if (line.starts_with("v ")) {
      buf >> trash;
      m_pos.emplace_back();
      auto &v = m_pos.back();
      buf >> v.x >> v.y >> v.z;
    } else if (line.starts_with("vt ")) {
      buf >> trash >> trash;
      m_uv.emplace_back();
      auto &uv_pos = m_uv.back();
      buf >> uv_pos.x >> uv_pos.y;
    } else if (line.starts_with("vn ")) {
      buf >> trash >> trash;
      m_norm.emplace_back();
      auto &n = m_norm.back();
      buf >> n.x >> n.y >> n.z;
    } else if (line.starts_with("f ")) {
      buf >> trash;
      int cnt = 0;
      vertex v;
      while (buf >> v.pos_index >> trash >> v.uv_index >> trash >> v.pos_index) {
        m_vertices.push_back(v);
        ++cnt;
      }
      if (cnt != 3) {
        throw std::logic_error("Not a triangle face!");
      }
    }
  }
}
