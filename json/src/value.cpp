#include <json/dom.h>

using namespace json;

dom::value dom::s_null_value;

const dom::value &dom::value::operator[](const char *key) const noexcept {
  if (m_type != type::object) {
    return s_null_value;
  }
  if (!m_extra) {
    return s_null_value;
  }
  std::int32_t l = 0, r = m_extra - 1;
  while (l <= r) {
    auto mid = (l + r) >> 1;
    auto mk = m_object[mid].key_pointer();
    auto cmp = std::strcmp(key, mk);
    if (!cmp) {
      return m_object[mid].value;
    }
    if (cmp > 0) {
      l = mid + 1;
    } else {
      r = mid - 1;
    }
  }
  return s_null_value;
}

const dom::value &dom::value::operator[](std::uint32_t index) const {
  return m_array[index];
};
