#pragma once
#ifndef PLAID_JSON_VALUE_H_
#define PLAID_JSON_VALUE_H_

#include <cstddef>
#include <cstdint>

#include <span>
#include <string_view>

#include "string_pool.h"

namespace plaid::json {
class member;
}

namespace plaid::json {

/// json 值对象，通过 union 保存各种类型的数据
/// 除空类型、布尔值和数字直接包含
/// 字符串、数组、对象有修改权，无内存所有权
class value {
private:
  enum class type : std::uint8_t {
    null,
    boolean,
    number,
    string,
    short_string,
    array,
    object,
  };

  value() noexcept;

public:
  static const value nullval;

  struct object_flag_t {};
  struct array_flag_t {};
  static constexpr object_flag_t object_flag;
  static constexpr array_flag_t array_flag;

 
  value(double) noexcept;
  value(bool) noexcept;
  value(std::string_view, string_pool &);
  value(std::size_t index, std::size_t size, object_flag_t);
  value(std::size_t index, std::size_t size, array_flag_t);

  void bind_string_pointer(string_pool &);

  void bind_object_pointer(std::vector<member> &);

  void bind_array_pointer(std::vector<value> &);

  [[nodiscard]] inline bool is_bool() const noexcept {
    return m_type == type::boolean;
  }

  [[nodiscard]] inline bool is_number() const noexcept {
    return m_type == type::number;
  }

  [[nodiscard]] inline bool is_string() const noexcept {
    return m_type == type::string || m_type == type::short_string;
  }

  [[nodiscard]] inline bool is_object() const noexcept {
    return m_type == type::object;
  }

  [[nodiscard]] inline bool is_array() const noexcept {
    return m_type == type::array;
  }

  [[nodiscard]] inline operator bool() const noexcept { return m_bool; }
  [[nodiscard]] inline operator double() const noexcept { return m_number; }

  [[nodiscard]] inline operator std::string_view() const noexcept {
    if (m_type == type::short_string) {
      return {&m_short_string_first, m_short_length};
    } else {
      return {m_string, m_extra};
    }
  }

  [[nodiscard]] inline operator std::span<value>() noexcept {
    return {m_array, m_extra};
  }

  [[nodiscard]] inline operator std::span<const value>() const noexcept {
    return {m_array, m_extra};
  }

  [[nodiscard]] operator std::span<const member>() const noexcept;

  [[nodiscard]] operator std::span<member>() noexcept;

  [[nodiscard]] inline bool as_bool() const noexcept { return *this; }

  [[nodiscard]] inline double as_number() const noexcept { return *this; }

  [[nodiscard]] inline std::string_view as_string() const noexcept { return *this; }

  [[nodiscard]] inline std::span<const value> as_const_array() const noexcept { return *this; }

  [[nodiscard]] inline std::span<value> as_array() noexcept { return *this; }
  /// 获取对象成员的值
  [[nodiscard]] const value &operator[](const char key[]) const noexcept;

  /// 获取数组元素的值
  [[nodiscard]] const value &operator[](std::uint32_t index) const;

  [[nodiscard]] inline const std::uint32_t size() const noexcept {
    return m_type == type::short_string ? m_short_length : m_extra;
  }

  [[nodiscard]] bool operator==(const value &rhs) const;

private:
  /// 记录节点类型
  enum class type m_type;
  /// 针对 short_string 类型记录字符串长度
  /// 短字符串长度可以在 [0, 13] 区间
  std::uint8_t m_short_length;
  /// 标记短字符串起始位置
  char m_short_string_first;
  /// 针对 array/object 记录成员数量,
  /// 针对 string 记录字符串长度,
  /// 针对 short_string 作为内容的一部分
  /// 其余情况作为保留位, 其值未定义
  std::uint32_t m_extra;
  union {
    bool m_bool;
    double m_number;
    /// 指向 string 字符串开头
    char *m_string;
    /// 指向 array 第一个成员的指针
    value *m_array;
    /// 指向 object 第一个成员的指针
    member *m_object;
    std::size_t m_index;
  };
};
} // namespace json

#endif
