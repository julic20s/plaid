#pragma once
#ifndef PLAID_JSON_DOM_H_
#define PLAID_JSON_DOM_H_

#include <cstddef>

#include <span>
#include <string_view>
#include <vector>

namespace json {

class dom {
private:
  struct member;
  class parser;

public:
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

  public:
    [[nodiscard]] inline operator bool() const noexcept { return m_bool; }
    [[nodiscard]] inline operator double() const noexcept { return m_number; }

    [[nodiscard]] inline operator std::string_view() const noexcept {
      if (m_type == type::short_string) {
        return {&m_short_string_first, m_short_length};
      } else {
        return {m_string, m_extra};
      }
    }

    [[nodiscard]] inline operator std::span<const value>() const noexcept {
      return {m_array, m_extra};
    }

    /// 获取对象成员的值
    [[nodiscard]] const value &operator[](const char *key) const noexcept;

    /// 获取数组元素的值
    [[nodiscard]] const value &operator[](std::uint32_t index) const;

    [[nodiscard]] inline const std::uint32_t size() const noexcept {
      return m_type == type::short_string ? m_short_length : m_extra;
    }

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
      std::ptrdiff_t m_offset;
    };

    friend class parser;
    friend class dom;
  };

  static constexpr auto s_value_sso_max = sizeof(value) - offsetof(value, m_extra) - 1;

  dom(const char *file);

  [[nodiscard]] const value &root() const { return m_root; }

private:
  /// json 成员对象
  struct member {
    bool short_string;
    char short_string_first;
    union {
      char *key;
      std::ptrdiff_t offset;
    };
    value value;

    [[nodiscard]] inline const char *key_pointer(char *ptr = nullptr) const noexcept {
      return short_string ? &short_string_first : ptr + offset;
    }
  };

  static constexpr auto s_member_sso_max =
      sizeof(member) - offsetof(member, value) - offsetof(member, short_string_first) - 1;

  /// 根节点
  value m_root;
  /// 字符串池
  std::vector<char> m_string_pool;
  /// 成员数组
  std::vector<member> m_members;
  std::vector<value> m_values;

  static value s_null_value;
};

} // namespace json

#endif
