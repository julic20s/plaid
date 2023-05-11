#include <cstdio>

#include <algorithm>
#include <vector>

#include <json/dom.h>
#include <json/except.h>

using namespace plaid::json;

class token_stream {

  static constexpr std::size_t buffer_size_ = 4096;

public:
  token_stream(const char *file)
      : skip_spaces_(true), maybe_(true), cur_(0), end_(0) {
    file_ = fopen(file, "r");
    if (file_ == nullptr) {
      throw std::runtime_error("");
    }
  }

  void skip_spaces(bool value) {
    skip_spaces_ = value;
  }

  char peek() {
    for (;;) {
      if (cur_ == end_) {
        if (maybe_) {
          pull();
          continue;
        }
        return EOF;
      }

      if (buf_[cur_] == '\n') {
        ++line_;
        column_ = 0;
      } else {
        ++column_;
      }

      if (skip_spaces_ && std::isspace(buf_[cur_])) {
        ++cur_;
        continue;
      }

      break;
    }

    return buf_[cur_];
  }

  void consume() {
    if (peek() == EOF) {
      throw std::runtime_error("");
    }
    ++cur_;
  }

  [[nodiscard]] std::uint32_t line() noexcept { return line_; }
  [[nodiscard]] std::uint32_t column() noexcept { return column_; }

private:
  void pull() {
    if (!maybe_) {
      return;
    }
    constexpr auto cap = std::extent_v<decltype(buf_)>;
    auto cnt = fread(buf_, 1, cap, file_);
    if (cnt < cap) {
      maybe_ = false;
    }
    cur_ = 0;
  }

  bool skip_spaces_, maybe_;
  std::uint32_t line_, column_;
  std::uint32_t cur_, end_;
  FILE *file_;
  char buf_[buffer_size_];
};

class parser : token_stream {

  void expect(const char pat[]) {
    for (; *pat; ++pat, consume()) {
      if (*pat != peek()) {
        throw std::runtime_error("");
      }
    }
  }

  [[nodiscard]] std::size_t read_string_tokens_(char buf[]) {
    consume(); // "
    skip_spaces(false);

    std::size_t cnt = 0;
    for (bool escape = false;; consume()) {
      auto ch = peek();

      if (ch == EOF) {
        throw std::runtime_error("");
      }

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
    buf[cnt] = '\0';

    consume(); // "
    skip_spaces(true);
    return cnt;
  }

  [[nodiscard]] value value_() {
    auto ch = peek();
    switch (ch) {
      case '{': return object_();
      case 't':
      case 'f': return bool_();
      case '"': return string_();
      case '[': return array_();
      case 'n': return nullval;
      default:
        if (ch == '-' || ('0' <= ch && ch <= '9')) {
          return number_();
        }
    }
    throw std::runtime_error("Unexpected character!");
  }

  [[nodiscard]] value bool_() {
    auto ch = peek();
    if (ch == 't') {
      expect("true");
      return true;
    } else if (ch == 'f') {
      expect("false");
      return false;
    }
    char str[2] {ch};
    throw json_parsing_error(line(), column(), "true/false", str);
  }

  [[nodiscard]] value number_() {
    double sgn = 1, number = 0;
    if (peek() == '-') {
      sgn = -1;
      consume();
    }

    double div = 1;
    while (true) {
      auto ch = peek();
      if (ch == EOF) {
        break;
      }

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

  value string_() {
    char str_buf[1 << 10];
    auto cnt = read_string_tokens_(str_buf);
    value val = std::string_view(str_buf, cnt);
    if (val.is_short_string_optimized()) {
      return val;
    }
    auto heap_buf = reinterpret_cast<char *>(pool_.aquire(cnt + 1));
    std::memcpy(heap_buf, str_buf, cnt + 1);
    return std::string_view(heap_buf, cnt);
  }

  value array_() {
    consume(); // [
  
    // 保存当前数组所有成员
    std::vector<value> arr_values;


    for (bool first = true;;) {
      auto ch = peek();

      if (ch == EOF) {
        throw std::runtime_error("Unfinished array.");
      }

      if (ch == ']') {
        break;
      }

      if (!first && ch == ',') {
        consume();
      }

      arr_values.emplace_back(value_());
      first = false;
    }

    consume(); // ]

    value *ptr = nullptr;
    if (arr_values.size()) {
      ptr = reinterpret_cast<value *>(pool_.aquire(sizeof(value) * arr_values.size()));
    }
    std::uninitialized_copy(arr_values.begin(), arr_values.end(), ptr);
    return std::span<const value>(ptr, arr_values.size());
  }

  value object_() {
    consume(); // {

    std::vector<member> members;
    while (true) {
      auto ch = peek();
      if (ch == EOF) {
        throw std::runtime_error("Unexcepted end!");
      }
      
      if (ch == '}') {
        break;
      }

      if (ch == '"') {
        members.emplace_back(member_());
      } else {
        throw std::runtime_error("");
      }

      if (peek() == ',') {
        consume();
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

  struct member::key key_() {
    char str_buf[1 << 10];
    auto cnt = read_string_tokens_(str_buf);
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

  member member_() {
    return {key_(), (expect(":"), value_())};
  }

public:
  parser(const char *file, chunked_pool &pool) noexcept
      : token_stream(file), pool_(pool) {}

  value parse() {
    switch (auto ch = peek()) {
      case '[': return array_();
      case '{': return object_();
      case EOF: throw std::runtime_error("There is no any thing in the file!");
      default: throw std::runtime_error(std::string("Illegal character for the beginning: ") + ch);
    }
  }

private:
  chunked_pool &pool_;
};

dom::dom(const char *file) {
  *static_cast<value *>(this) = parser(file, pool_).parse();
}
