#pragma once
#ifndef PLAID_JSON_VALUE_H_
#define PLAID_JSON_VALUE_H_

#include <cstddef>
#include <cstdint>

#include <span>
#include <stdexcept>
#include <string_view>

namespace plaid::json {
class member;
}

namespace plaid::json {

/// json 值对象，通过 union 保存各种类型的数据
/// 除空类型、布尔值和数字直接包含
/// 字符串、数组、对象有修改权，无内存所有权
class value {
public:
  constexpr value() noexcept;
  constexpr value(bool) noexcept;
  constexpr value(double) noexcept;
  constexpr value(std::string_view) noexcept;
  constexpr value(std::span<const value>) noexcept;
  constexpr value(std::span<const member>) noexcept;

  [[nodiscard]] constexpr bool is_short_string_optimized() const noexcept;

  [[nodiscard]] constexpr bool is_bool() const noexcept;
  [[nodiscard]] constexpr bool is_number() const noexcept;
  [[nodiscard]] constexpr bool is_string() const noexcept;
  [[nodiscard]] constexpr bool is_object() const noexcept;
  [[nodiscard]] constexpr bool is_array() const noexcept;

  [[nodiscard]] constexpr operator bool() const noexcept;
  [[nodiscard]] constexpr operator double() const noexcept;
  [[nodiscard]] constexpr operator std::string_view() const noexcept;
  [[nodiscard]] constexpr operator std::span<const value>() const noexcept;
  [[nodiscard]] constexpr operator std::span<const member>() const noexcept;

  [[nodiscard]] constexpr bool to_bool() const noexcept;
  [[nodiscard]] constexpr double to_double() const noexcept;
  [[nodiscard]] constexpr std::string_view to_string_view() const noexcept;
  [[nodiscard]] constexpr std::span<const value> to_value_span() const noexcept;
  [[nodiscard]] constexpr std::span<const member> to_member_span() const noexcept;

  [[nodiscard]] constexpr const value &operator[](const char key[]) const;
  [[nodiscard]] constexpr const value &operator[](std::uint32_t index) const;

  [[nodiscard]] constexpr std::uint32_t size() const noexcept;

  [[nodiscard]] const std::byte *data() const noexcept;

  friend constexpr bool operator==(const value &, const value &) noexcept;

private:
  /// 记录节点类型
  enum class type : std::uint8_t {
    nullval,
    boolean,
    number,
    string,
    short_string,
    array,
    object,
  } type_;

  /// 针对 short_string 类型记录字符串长度
  /// 短字符串长度可以在 [0, 13] 区间
  std::uint8_t short_string_length_;
  /// 标记短字符串起始位置
  char short_string_first_;
  /// 针对 array/object 记录成员数量,
  /// 针对 string 记录字符串长度,
  /// 针对 short_string 作为内容的一部分
  /// 其余情况作为保留位, 其值未定义
  std::uint32_t size_;

  union {
    bool bool_;
    double number_;
    /// 指向 string 字符串开头
    const char *string_;
    /// 指向 array 第一个成员的指针
    const value *array_;
    /// 指向 object 第一个成员的指针
    const member *object_;
  };
};

/// json 成员对象
struct member {
  class key {
  public:
    key() = default;

    constexpr key(std::string_view str);

    friend constexpr bool operator==(const key &, const key &) noexcept;

    [[nodiscard]] constexpr operator std::string_view() const noexcept;

    [[nodiscard]] constexpr bool is_short_string_optimized() const noexcept;

  private:
    /// 是否启用了短字符串优化
    bool sso_enabled_;
    /// 记录短字符串长度
    std::uint8_t sso_length_;
    /// 标记短字符串的第一个字符
    char sso_first_;
    /// 记录长度
    std::uint32_t length_;
    /// 指向关键字的指针
    const char *pointer_;
  };

  friend constexpr bool operator==(const member &, const member &) noexcept;

  key key;
  value value;
};

constexpr value::value() noexcept
    : type_(type::nullval),
      short_string_first_(0),
      short_string_length_(0),
      size_(0),
      number_(0) {}

constexpr value::value(bool val) noexcept
    : type_(type::boolean),
      short_string_first_(0),
      short_string_length_(0),
      size_(1),
      bool_(val) {}

constexpr value::value(double val) noexcept
    : type_(type::number),
      short_string_first_(0),
      short_string_length_(0),
      size_(1),
      number_(val) {}

constexpr value::value(std::string_view val) noexcept
    : type_(type::string),
      short_string_first_(0),
      short_string_length_(val.length()),
      size_(val.length()),
      string_(val.data()) {
  constexpr auto sso_capacity = sizeof(value) - offsetof(value, short_string_first_) - 1;
  if (sso_capacity >= val.size()) {
    type_ = type::short_string;
    std::copy(val.begin(), val.end(), &short_string_first_);
    (&short_string_first_)[val.size()] = '\0';
  }
}

constexpr value::value(std::span<const value> val) noexcept
    : type_(type::array),
      short_string_first_(0),
      short_string_length_(0),
      size_(val.size()),
      array_(val.data()) {}

constexpr value::value(std::span<const member> val) noexcept
    : type_(type::object),
      short_string_first_(0),
      short_string_length_(0),
      size_(val.size()),
      object_(val.data()) {}

constexpr bool value::is_short_string_optimized() const noexcept {
  return type_ == type::short_string;
}

constexpr bool value::is_bool() const noexcept {
  return type_ == type::boolean;
}

constexpr bool value::is_number() const noexcept {
  return type_ == type::number;
}

constexpr bool value::is_string() const noexcept {
  return type_ == type::string || is_short_string_optimized();
}

constexpr bool value::is_object() const noexcept {
  return type_ == type::object;
}

constexpr bool value::is_array() const noexcept {
  return type_ == type::array;
}

constexpr value::operator bool() const noexcept {
  return bool_;
}

constexpr value::operator double() const noexcept {
  return number_;
}

constexpr value::operator std::string_view() const noexcept {
  if (is_short_string_optimized()) {
    return {&short_string_first_, short_string_length_};
  } else {
    return {string_, size_};
  }
}

constexpr value::operator std::span<const value>() const noexcept {
  return {array_, size_};
}

constexpr value::operator std::span<const member>() const noexcept {
  return {object_, size_};
}

constexpr bool value::to_bool() const noexcept {
  return *this;
}

constexpr double value::to_double() const noexcept {
  return *this;
}

constexpr std::string_view value::to_string_view() const noexcept {
  return *this;
}

constexpr std::span<const value> value::to_value_span() const noexcept {
  return *this;
}

constexpr std::span<const member> value::to_member_span() const noexcept {
  return *this;
}

constexpr const value &value::operator[](const char key[]) const {
  if (!is_object()) {
    throw std::runtime_error("Key finding is only allowed for a object.");
  }

  std::string_view kstr(key);

  auto l = object_, r = object_ + size_ - 1;
  while (l <= r) {
    auto mid = l + (r - l) / 2;
    auto cmp = kstr.compare(std::string_view(mid->key));
    if (cmp == 0) {
      return mid->value;
    }
    if (cmp > 0) {
      l = mid + 1;
    } else {
      r = mid - 1;
    }
  }
  throw std::runtime_error("Not found the member with given key.");
}

constexpr const value &value::operator[](std::uint32_t index) const {
  return array_[index];
}

constexpr std::uint32_t value::size() const noexcept {
  return is_short_string_optimized() ? short_string_length_ : size_;
}

[[nodiscard]] constexpr bool operator==(const value &lhs, const value &rhs) noexcept {
  using ty = value::type;
  if (lhs.type_ != rhs.type_) {
    return false;
  }
  switch (lhs.type_) {
    case ty::nullval: return true;
    case ty::boolean: return lhs.bool_ == rhs.bool_;
    case ty::number: return lhs.number_ == rhs.number_;
    case ty::short_string:
    case ty::string: return lhs.to_string_view() == rhs.to_string_view();
    default: break;
  }
  if (lhs.size_ != rhs.size_) {
    return false;
  }

  if (lhs.type_ == ty::array) {
    if (lhs.array_ == rhs.array_) {
      return true;
    }
    auto it = rhs.to_value_span().begin();
    for (auto &v : lhs.to_value_span()) {
      if (*it != v) {
        return false;
      }
      ++it;
    }
    return true;
  } else if (lhs.type_ == ty::object) {
    auto it = rhs.to_member_span().begin();
    for (auto &m : lhs.to_member_span()) {
      if (*it != m) {
        return false;
      }
      ++it;
    }
    return true;
  }

  return false;
}

[[nodiscard]] constexpr bool operator==(const class member::key &lhs, const class member::key &rhs) noexcept {
  if (lhs.sso_enabled_ != rhs.sso_enabled_) {
    return false;
  }
  if (lhs.sso_enabled_) {
    std::string_view l(&lhs.sso_first_, lhs.sso_length_);
    std::string_view r(&rhs.sso_first_, rhs.sso_length_);
    return l == r;
  } else {
    std::string_view l(lhs.pointer_, lhs.length_);
    std::string_view r(rhs.pointer_, rhs.length_);
    return l == r;
  }
}

[[nodiscard]] constexpr bool operator==(const member &lhs, const member &rhs) noexcept {
  return lhs.key == rhs.key && lhs.value == rhs.value;
}

constexpr member::key::key(std::string_view str) {
  constexpr auto sso_capacity = sizeof(key) - offsetof(key, sso_first_) - 1;
  sso_enabled_ = str.size() <= sso_capacity;
  if (sso_enabled_) {
    sso_length_ = str.size();
    std::memcpy(&sso_first_, str.data(), sso_length_);
    (&sso_first_)[sso_length_] = '\0';
  } else {
    length_ = str.size();
    pointer_ = str.data();
  }
}

constexpr bool member::key::is_short_string_optimized() const noexcept {
  return sso_enabled_;
}

constexpr member::key::operator std::string_view() const noexcept {
  if (sso_enabled_) {
    return {&sso_first_, sso_length_};
  } else {
    return {pointer_, length_};
  }
}

constexpr value nullval;

} // namespace plaid::json

#endif
