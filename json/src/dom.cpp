#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>

#include <json/dom.h>

using namespace plaid::json;

class token_stream {

public:
  token_stream(std::istream &stream) noexcept
      : m_skip_spaces(true), m_cur(0), m_stream(stream) {}

  void skip_spaces(bool value) {
    m_skip_spaces = value;
  }

  bool has_next() {
    while (true) {
      if (m_cur == m_buf.size()) {
        if (!pull()) {
          return false;
        }
      }
      if (m_skip_spaces) {
        while (m_cur != m_buf.size()) {
          if (!std::isspace(m_buf[m_cur])) {
            break;
          }
          ++m_cur;
        }
      }
      if (m_cur != m_buf.size()) {
        return true;
      }
    }
  }

  char next() {
    while (true) {
      if (m_cur == m_buf.size()) {
        if (!pull()) {
          throw std::runtime_error("");
        }
      }
      if (m_skip_spaces) {
        while (m_cur != m_buf.size()) {
          if (!std::isspace(m_buf[m_cur])) {
            break;
          }
          ++m_cur;
        }
      }
      if (m_cur != m_buf.size()) {

        return m_buf[m_cur];
      }
    }
  }

  void consume() {
    if (m_cur != m_buf.size()) {
      ++m_cur;
    } else {
      throw std::runtime_error("");
    }
  }

private:
  bool pull() {
    m_cur = 0;
    m_buf.clear();
    std::getline(m_stream, m_buf);
    return true;
  }

  bool m_skip_spaces;
  std::size_t m_cur;
  std::string m_buf;
  std::istream &m_stream;
};

class dom::parser : virtual value {
private:
  static void expect(const char pat[], token_stream &tokens) {
    for (; *pat; ++pat, tokens.consume()) {
      if (!tokens.has_next() || *pat != tokens.next()) {
        throw std::runtime_error("");
      }
    }
  }

  static void bind_pointers(dom &host, member &dst) {
    dst.key.bind_pointer(host.m_string_pool);
    bind_pointers(host, dst.value);
  }

  static void bind_pointers(dom &host, value &dst) {
    if (dst.is_string()) {
      dst.bind_string_pointer(host.m_string_pool);
    } else if (dst.is_array()) {
      dst.bind_array_pointer(host.m_values);
      for (auto &v : std::span<value>(dst)) {
        bind_pointers(host, v);
      }
    } else if (dst.is_object()) {
      dst.bind_object_pointer(host.m_members);
      for (auto &m : std::span<member>(dst)) {
        bind_pointers(host, m);
      }
    }
  }

public:
  static void parse(dom &host, token_stream &tokens) {
    if (tokens.has_next()) {
      auto ch = tokens.next();
      if (ch == '[') {
        parse_array(host, tokens, host);
      } else if (ch == '{') {
        parse_object(host, tokens, host);
      } else {
        throw std::runtime_error(std::string("Illegal character for the beginning: ") + ch);
      }
    } else {
      throw std::runtime_error("There is no any thing in the file!");
    }
    bind_pointers(host, host);
  }

private:
  static void parse_array(dom &host, token_stream &tokens, value &dst) {
    tokens.consume(); // [
    if (!tokens.has_next()) {
      throw std::runtime_error("Unfinished array.");
    }

    std::vector<value> values;

    while (tokens.has_next()) {
      auto ch = tokens.next();

      if (ch == ']') {
        break;
      }

      if (ch == ',') {
        tokens.consume();
        ch = tokens.next();
      }

      value &current = values.emplace_back(value::nullval);
      parse_value(host, tokens, current);
    }

    tokens.consume(); // ]

    auto start = host.m_values.size();
    host.m_values.reserve(start + values.size());
    std::copy(values.begin(), values.end(), std::back_insert_iterator(host.m_values));
    dst = {start, values.size(), value::array_flag};
  }

  static void parse_value(dom &host, token_stream &tokens, value &dst) {
    auto ch = tokens.next();
    switch (ch) {
      case '{':
        parse_object(host, tokens, dst);
        break;
      case 't':
      case 'f':
        parse_boolean(host, tokens, dst);
        break;
      case '"':
        parse_string(host, tokens, dst);
        break;
      case '[':
        parse_array(host, tokens, dst);
        break;
      case 'n':
        dst = value::nullval;
        break;
      default:
        if (ch == '-' || ('0' <= ch && ch <= '9')) {
          parse_number(host, tokens, dst);
        } else {
          throw std::runtime_error("Unexpected character!");
        }
    }
  }

  static void parse_boolean(dom &host, token_stream &tokens, value &dst) {
    auto ch = tokens.next();
    if (ch == 't') {
      expect("true", tokens);
      dst = true;
    } else if (ch == 'f') {
      expect("false", tokens);
      dst = false;
    }
  }

  static void parse_number(dom &host, token_stream &tokens, value &dst) {
    double sgn = 1, number = 0;
    if (tokens.next() == '-') {
      sgn = -1;
      tokens.consume();
    }

    double div = 1;
    while (true) {
      if (!tokens.has_next()) {
        break;
      }
      auto ch = tokens.next();

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
          throw std::runtime_error("");
        }
      } else if (ch == ']' || ch == '}' || ch == ',') {
        break;
      } else {
        throw std::runtime_error("");
      }

      tokens.consume();
    }

    dst = sgn * number;
  }

  static void parse_string(dom &host, token_stream &tokens, value &dst) {
    char str_buf[1 << 10];
    auto cnt = parse_string_tokens(host, tokens, str_buf);
    dst = value(std::string_view(str_buf, cnt), host.m_string_pool);
  }

  static std::size_t parse_string_tokens(dom &host, token_stream &tokens, char dst[]) {
    std::size_t cnt = 0;
    tokens.consume(); // "
    tokens.skip_spaces(false);
    bool escape = false;
    for (;; tokens.consume()) {
      if (!tokens.has_next()) {
        throw std::runtime_error("");
      }

      auto ch = tokens.next();
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
      dst[cnt++] = ch;
      escape = false;
    }
    tokens.consume();
    tokens.skip_spaces(true);
    return cnt;
  }

  static void parse_object(dom &host, token_stream &tokens, value &dst) {
    tokens.consume(); // {

    std::vector<member> members;
    while (true) {
      if (!tokens.has_next()) {
        throw std::runtime_error("Unexcepted end!");
      }

      auto ch = tokens.next();
      if (ch == '}') {
        break;
      }

      if (ch == ',') {
        tokens.consume();
        ch = tokens.next();
      }

      auto &current = members.emplace_back();
      if (ch == '"') {
        parse_member(host, tokens, current);
      }
    }

    tokens.consume(); // }

    auto pool = &host.m_string_pool;
    std::sort(members.begin(), members.end(), [pool](const member &a, const member &b) {
      return a.key.find(pool) < b.key.find(pool);
    });

    auto old_size = host.m_members.size();
    host.m_members.reserve(old_size + members.size());
    std::copy(members.begin(), members.end(), std::back_insert_iterator(host.m_members));

    dst = {old_size, members.size(), value::object_flag};
  }

  static void parse_member(dom &host, token_stream &tokens, member &dst) {
    parse_key(host, tokens, dst.key);
    expect(":", tokens);
    parse_value(host, tokens, dst.value);
  }

  static void parse_key(dom &host, token_stream &tokens, struct member::key &key) {
    char str_buf[1 << 10];
    auto cnt = parse_string_tokens(host, tokens, str_buf);
    key = {{str_buf, cnt}, host.m_string_pool};
  }
};

dom::dom(const char *file) : value(value::nullval) {
  std::ifstream stream(file);
  token_stream ts(stream);
  parser::parse(*this, ts);
}
