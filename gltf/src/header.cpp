#include <array>
#include <queue>
#include <vector>

#include <gltf/header.h>

#include "json.h"

using namespace gltf;
using namespace gltf::internal;

class key_matcher {

  static void insert_string_to_trie(key_matcher &m, const char *str, int id) {
    int p = 0;
    for (auto it = str; *it; ++it) {
      if (!m.m_trie[p][*it]) {
        m.m_trie.resize(++m.m_cnt + 1);
        m.m_trie[p][*it] = m.m_cnt;
      }
      p = m.m_trie[p][*it];
    }
    m.m_id.resize(p + 1);
    m.m_id[p] = id;
  }

public:

  key_matcher(std::initializer_list<const char *> list) : m_trie(8), m_id(8), m_cnt(0) {
    std::size_t i = 0;
    for (auto k : list) {
      insert_string_to_trie(*this, k, i++);
    }
    m_trie.shrink_to_fit();
    m_id.shrink_to_fit();
  }

  int match(const char *str) const {
    if (!str[0]) {
      return -1;
    }

    int p = 0;
    for (auto it = str; *it; ++it) {
      if (!m_trie[p][*it]) {
        return -1;
      }
      p = m_trie[p][*it];
    }
    return m_id[p];
  }

private:

  std::vector<std::array<int, 128>> m_trie;
  std::vector<int> m_id;
  int m_cnt;
};

static const key_matcher g_root_matcher({
    "asset",
    "scenes",
    "buffers",
    "camera",
    "bufferViews",
});

bool header::load(header &h, std::istream &stream) {
  stream >> json::object_begin;
  while (true) {
    if (stream.fail() || stream.eof()) {
      return false;
    }
    if (!json::has_key(stream)) {
      break;
    }
    json::json_string key;
    stream >> key;
    if (stream.fail() || stream.eof()) {
      return false;
    }
    auto index = g_root_matcher.match(key.c_str());
    if (index == -1) {
      return false;
    }

    // TODO
  }
  stream >> json::object_end;
  return stream.good();
}
