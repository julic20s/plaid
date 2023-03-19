#pragma once

#include "string_pool.h"
#include "value.h"

namespace json {

/// json 成员对象
struct member {
  struct key {
    key() = default;

    key(std::string_view str, string_pool &);

    /// 在给定的字符串池中根据索引查找对应关键字，如果已经绑定字符串池，传入的字符串池指针将被忽略
    [[nodiscard]] std::string_view find(const string_pool *) const;

    /// 绑定关键字指针，如果给出的字符串池被过早销毁，则关键字将可能失效
    void bind_pointer(const string_pool &);

  private:
    /// 是否启用了短字符串优化
    bool m_sso_enabled;
    /// 记录短字符串长度
    std::uint8_t m_sso_length;
    /// 标记短字符串的第一个字符
    char m_sso_first;
    /// 是否已经绑定指针
    bool m_binded;
    /// 记录长度
    std::uint32_t m_length;
    union {
      /// 指向关键字的指针
      const char *m_pointer;
      /// 关键字在某一字符串池中的索引
      std::ptrdiff_t m_index;
    };
  };

  member() noexcept;
  
  key key;
  value value;
};

} // namespace json
