#include <json/dom.h>

using namespace plaid::json;

const value value::nullval;

value::value() noexcept : m_type(type::null) {}

value::value(double v) noexcept : m_type(type::number), m_number(v) {}

value::value(bool v) noexcept : m_type(type::boolean), m_bool(v) {}

value::value(std::string_view str, string_pool &pool) {
  constexpr auto sso_capacity = sizeof(value) - offsetof(value, m_short_string_first) - 1;
  auto size = str.size();
  if (size <= sso_capacity) {
    m_type = type::short_string;
    m_short_length = size;
    std::memcpy(&m_short_string_first, str.data(), size);
    (&m_short_string_first)[size] = '\0';
  } else {
    m_type = type::string;
    m_extra = size;
    m_index = pool.push(str);
  }
}

value::value(std::size_t index, std::size_t size, object_flag_t)
    : m_type(type::object), m_extra(size), m_index(index) {}

value::value(std::size_t index, std::size_t size, array_flag_t)
    : m_type(type::array), m_extra(size), m_index(index) {}

void value::bind_string_pointer(string_pool &pool) {
  if (m_type == type::string) {
    m_string = pool[m_index];
  }
}

void value::bind_array_pointer(std::vector<value> &src) {
  if (m_type == type::array) {
    m_array = src.data() + m_index;
  }
}

void value::bind_object_pointer(std::vector<member> &src) {
  if (m_type == type::object) {
    m_object = src.data() + m_index;
  }
}

value::operator std::span<member>() noexcept {
  return {m_object, m_extra};
}

value::operator std::span<const member>() const noexcept {
  return {m_object, m_extra};
}

const value &value::operator[](const char key[]) const noexcept {
  if (m_type == type::object) {
    std::int64_t l = 0, r = m_extra - 1;
    while (l <= r) {
      auto mid = (l + r) >> 1;
      auto cmp = m_object[mid].key.find(nullptr).compare(key);
      if (!cmp) {
        return m_object[mid].value;
      }
      if (cmp < 0) {
        l = mid + 1;
      } else {
        r = mid - 1;
      }
    }
  }
  return value::nullval;
}

const value &value::operator[](std::uint32_t index) const {
  return m_array[index];
};

bool value::operator==(const value &rhs) const {
  if (m_type != rhs.m_type) {
    return false;
  }
  switch (m_type) {
    case type::null: return true;
    case type::boolean: return m_bool == rhs.m_bool;
    case type::number: return m_number == rhs.m_number;
    case type::short_string:
    case type::string: return as_string() == rhs.as_string();
    default: break;
  }
  if (m_extra != rhs.m_extra) {
    return false;
  }

  if (m_type == type::array) {
    auto it = rhs.as_const_array().begin();
    for (auto &v : as_const_array()) {
      if (*it != v) {
        return false;
      }
      ++it;
    }
    return true;
  } else if (m_type == type::object) {
    auto it = std::span<const member>(rhs).begin();
    for (auto &m : std::span<const member>(*this)) {
      if (*it != m) {
        return false;
      }
      ++it;
    }
    return true;
  }

  return false;
}
