#pragma once

#include <vector>

#include <plaid/vec.h>

class obj_model {
public:

  struct vertex {
    std::uint32_t pos_index;
    std::uint32_t uv_index;
    std::uint32_t norm_index;
  };

  obj_model(const char *file);

  [[nodiscard]] inline const plaid::vec3 *positions() const noexcept { return m_pos.data(); }

  [[nodiscard]] inline const vertex *vertices() const noexcept { return m_vertices.data(); }

  [[nodiscard]] inline std::uint32_t size() const noexcept { return m_vertices.size(); }

private:

  std::vector<plaid::vec3> m_pos;
  std::vector<plaid::vec2> m_uv;
  std::vector<plaid::vec3> m_norm;
  std::vector<vertex> m_vertices;
};
