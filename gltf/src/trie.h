#pragma once

#include <array>
#include <vector>

namespace gltf::internal {

class trie {
private:

  void grow_if_need(std::uint32_t required) {
    if (m_trie.capacity() < required) {
      m_trie.resize(required * 2, {});
      m_val.resize(required * 2, {});
    }
  }

public:

  trie(std::initializer_list<const char *> list) : m_trie{{}}, m_val(0), m_cnt(0) {
    for (auto str : list) {
      insert(str);
    }
  }

  void insert(const char *str) {
    std::uint32_t u = 0;
    for (auto v = str; *v; ++v) {
      if (!m_trie[u][*v]) {
        grow_if_need(++m_cnt + 1);
        m_trie[u][*v] = m_cnt;
      }
      u = m_trie[u][*v];
    }
    m_val[u] = str;
  }

  const char *find(const char *str) const {
    std::uint32_t u = 0;
    for (auto v = str; *v; ++v) {
      if (!m_trie[u][*v]) {
        return nullptr;
      }
      u = m_trie[u][*v];
    }
    return m_val[u];
  }

private:

  std::uint32_t m_cnt;
  std::vector<std::array<std::uint32_t, 128>> m_trie;
  std::vector<const char *> m_val;
};
} // namespace gltf::internal
