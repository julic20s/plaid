#include <algorithm>
#include <fstream>
#include <vector>

#include <json/dom.h>
#include <json/except.h>

using namespace plaid::json;

class token_stream {

public:
  token_stream(std::istream &stream) noexcept
      : skip_spaces_(true), cur_(0), stream_(stream) {}

  void skip_spaces(bool value) {
    skip_spaces_ = value;
  }

  bool has_next() {
    while (true) {
      if (cur_ == buf_.size()) {
        if (!pull()) {
          return false;
        }
      }
      if (skip_spaces_) {
        while (cur_ != buf_.size()) {
          if (!std::isspace(buf_[cur_])) {
            break;
          }
          ++cur_;
        }
      }
      if (cur_ != buf_.size()) {
        return true;
      }
    }
  }

  char next() {
    while (true) {
      if (cur_ == buf_.size()) {
        if (!pull()) {
          throw std::runtime_error("");
        }
      }
      if (skip_spaces_) {
        while (cur_ != buf_.size()) {
          if (!std::isspace(buf_[cur_])) {
            break;
          }
          ++cur_;
        }
      }
      if (cur_ != buf_.size()) {
        return buf_[cur_];
      }
    }
  }

  void consume() {
    if (cur_ != buf_.size()) {
      ++cur_;
    } else {
      throw std::runtime_error("");
    }
  }

  [[nodiscard]] std::uint32_t line() noexcept { return line_; }
  [[nodiscard]] std::uint32_t column() noexcept { return column_; }

private:
  bool pull() {
    cur_ = 0;
    buf_.clear();
    std::getline(stream_, buf_);
    return true;
  }

  bool skip_spaces_;
  std::uint32_t line_;
  std::uint32_t column_;
  std::size_t cur_;
  std::string buf_;
  std::istream &stream_;
};

class parser : token_stream {

  void expect(const char pat[]) {
    for (; *pat; ++pat, consume()) {
      if (!has_next() || *pat != next()) {
        throw std::runtime_error("");
      }
    }
  }

  std::size_t parse_string_tokens(char buf[]) {
    std::size_t cnt = 0;
    consume(); // "
    skip_spaces(false);
    bool escape = false;
    for (;; consume()) {
      if (!has_next()) {
        throw std::runtime_error("");
      }

      auto ch = next();
      if (ch == '"') {
        if (!escape) {
          break;
        }
      }
      if (ch == '\\') {
        if (!escape) {
          escape = true;
          continue;
        }
      }
      buf[cnt++] = ch;
      escape = false;
    }
    consume();
    skip_spaces(true);
    return cnt;
  }

  value parse_value() {
    auto ch = next();
    switch (ch) {
      case '{': return parse_object();
      case 't':
      case 'f': return parse_bool();
      case '"': return parse_string();
      case '[': return parse_array();
      case 'n': return nullval;
      default:
        if (ch == '-' || ('0' <= ch && ch <= '9')) {
          return parse_number();
        }
    }
    throw std::runtime_error("Unexpected character!");
  }

  value parse_bool() {
    auto ch = next();
    if (ch == 't') {
      expect("true");
      return true;
    } else if (ch == 'f') {
      expect("false");
      return false;
    }
    throw 0;
  }

  value parse_number() {
    double sgn = 1, number = 0;
    if (next() == '-') {
      sgn = -1;
      consume();
    }

    double div = 1;
    while (true) {
      if (!has_next()) {
        break;
      }
      auto ch = next();

      if ('0' <= ch && ch <= '9') {
        auto add = (ch ^ '0') * div;
        if (div == 1) {
          number = number * 10 + add;
        } else {
          number += add;
          div *= .1;
        }
      } else if (div == 1 && ch == '.') {
        if (div == 1) {
          div = .1;
        } else {
          const char actual[] = {ch, '\0'};
          throw json_parsing_error(line(), column(), "[0-9]", actual);
        }
      } else if (ch == ']' || ch == '}' || ch == ',') {
        break;
      } else {
        const char actual[] = {ch, '\0'};
        throw json_parsing_error(line(), column(), "", actual);
      }

      consume();
    }

    return sgn * number;
  }

  value parse_string() {
    char str_buf[1 << 10];
    auto cnt = parse_string_tokens(str_buf);
    value val = std::string_view(str_buf, cnt);
    if (val.is_short_string_optimized()) {
      return val;
    }
    auto heap_buf = reinterpret_cast<char *>(pool_.aquire(cnt + 1));
    std::memcpy(heap_buf, str_buf, cnt);
    heap_buf[cnt] = '\0';
    return std::string_view(heap_buf, cnt);
  }

  value parse_array() {
    consume(); // [
    if (!has_next()) {
      throw std::runtime_error("Unfinished array.");
    }

    // 保存当前数组所有成员
    std::vector<value> arr_values;

    while (has_next()) {
      auto ch = next();

      if (ch == ']') {
        break;
      }

      if (ch == ',') {
        consume();
        ch = next();
      }

      arr_values.emplace_back(parse_value());
    }

    consume(); // ]

    value *ptr = nullptr;
    if (arr_values.size()) {
      ptr = reinterpret_cast<value *>(pool_.aquire(sizeof(value) * arr_values.size()));
    }
    std::uninitialized_copy(arr_values.begin(), arr_values.end(), ptr);
    return std::span<const value>(ptr, arr_values.size());
  }

  value parse_object() {
    consume(); // {

    std::vector<member> members;
    while (true) {
      if (!has_next()) {
        throw std::runtime_error("Unexcepted end!");
      }

      auto ch = next();
      if (ch == '}') {
        break;
      }

      if (ch == ',') {
        consume();
        ch = next();
      }

      if (ch == '"') {
        members.emplace_back(parse_member());
      } else {
        throw std::runtime_error("");
      }
    }

    consume(); // }

    std::sort(members.begin(), members.end(), [](const member &a, const member &b) {
      return std::string_view(a.key) < std::string_view(b.key);
    });

    member *ptr = nullptr;
    if (members.size()) {
      ptr = reinterpret_cast<member *>(pool_.aquire(sizeof(member) * members.size()));
      std::uninitialized_copy(members.begin(), members.end(), ptr);
    }
    return std::span<const member>(ptr, members.size());
  }

  struct member::key parse_key() {
    char str_buf[1 << 10];
    auto cnt = parse_string_tokens(str_buf);
    class member::key stack_key = std::string_view(str_buf, cnt);
    if (stack_key.is_short_string_optimized()) {
      return stack_key;
    } else {
      auto mem = reinterpret_cast<char *>(pool_.aquire(cnt + 1));
      std::memcpy(mem, str_buf, cnt);
      mem[cnt] = '\0';
      return std::string_view(mem, cnt);
    }
  }

  member parse_member() {
    return {parse_key(), (expect(":"), parse_value())};
  }

public:
  parser(std::istream &stream, chunked_pool &pool) noexcept
      : token_stream(stream), pool_(pool) {}

  value parse() {
    if (has_next()) {
      auto ch = next();
      if (ch == '[') {
        return parse_array();
      } else if (ch == '{') {
        return parse_object();
      } else {
        throw std::runtime_error(std::string("Illegal character for the beginning: ") + ch);
      }
    } else {
      throw std::runtime_error("There is no any thing in the file!");
    }
  }

private:
  chunked_pool &pool_;
};

dom::dom(const char *file) {
  std::ifstream stream(file);
  *static_cast<value *>(this) = parser(stream, pool_).parse();
}
