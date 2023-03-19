#include <json/member.h>

using namespace plaid::json;

member::member() noexcept : value(value::nullval) {}

member::key::key(std::string_view key_str, string_pool &pool) {
  constexpr auto sso_capacity = sizeof(key) - offsetof(key, m_sso_first) - 1;
  auto key_size = key_str.size();

  m_sso_enabled = key_size <= sso_capacity;
  if (m_sso_enabled) {
    m_sso_length = key_size;
    std::memcpy(&m_sso_first, key_str.data(), key_size);
    (&m_sso_first)[key_size] = '\0';
  } else {
    m_binded = false;
    m_length = key_size;
    m_index = pool.push(key_str);
  }
}

std::string_view member::key::find(const string_pool *pool) const {
  const char *ptr;
  std::size_t len;
  if (m_sso_enabled) {
    ptr = &m_sso_first;
    len = m_sso_length;
  } else if (m_binded) {
    ptr = m_pointer;
    len = m_length;
  } else {
    ptr = (*pool)[m_index];
    len = m_length;
  }
  return {ptr, len};
}

void member::key::bind_pointer(const string_pool &pool) {
  if (!m_sso_enabled) {
    m_pointer = pool[m_index];
    m_binded = true;
  }
}
