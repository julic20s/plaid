#pragma once

#include <stdexcept>
#include <string>

namespace plaid::json {

class json_parsing_error : public std::runtime_error {
public:
  json_parsing_error(
      std::uint32_t line, std::uint32_t column,
      const char *expected, const char *actual
  ) : std::runtime_error{
          "JSON parsing error at (" +
              std::to_string(line) + ", " + std::to_string(column) +
              "); Expected: " + expected + "; Actual: " + actual + ";",
      },
      line_(line), column_(column) {}

  [[nodiscard]] std::uint32_t line() const noexcept;
  [[nodiscard]] std::uint32_t column() const noexcept;

private:
  std::uint32_t line_;
  std::uint32_t column_;
};

} // namespace plaid::json
