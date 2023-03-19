#pragma once

#include <string_view>
#include <vector>

namespace plaid::json {

class string_pool {
public:
  [[nodiscard]] std::size_t push(std::string_view);
  
  [[nodiscard]] const char *operator[](std::size_t index) const noexcept;

  [[nodiscard]] char *operator[](std::size_t index) noexcept;

private:
  std::vector<char> m_pool;
};

} // namespace json
