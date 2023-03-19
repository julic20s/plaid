#pragma once
#ifndef PLAID_JSON_DOM_H_
#define PLAID_JSON_DOM_H_

#include <vector>

#include "member.h"
#include "value.h"

namespace plaid::json {

class dom : value {
public:
  dom(const char *file);

  [[nodiscard]] inline const value &root() const noexcept {
    return *this;
  }

private:
  string_pool m_string_pool;
  /// 成员数组
  std::vector<member> m_members;
  std::vector<value> m_values;

  class parser;
};

} // namespace json

#endif
