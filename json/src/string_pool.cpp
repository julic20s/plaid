#include <json/string_pool.h>

using namespace plaid::json;

std::size_t string_pool::push(std::string_view str) {
  auto index = m_pool.size();
  m_pool.resize(index + str.size() + 1);
  std::memcpy(m_pool.data(), str.data(), str.size());
  m_pool.back() = '\0';
  return index;
}

char *string_pool::operator[](std::size_t index) noexcept {
  return m_pool.data() + index;
}

const char *string_pool::operator[](std::size_t index) const noexcept {
  return m_pool.data() + index;
}
