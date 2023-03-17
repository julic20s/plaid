#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>

#include <json/dom.h>

using namespace json;

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

/// 检验字符串
static void expect(const char pat[], token_stream &tokens) {
  for (; *pat; ++pat, tokens.consume()) {
    if (!tokens.has_next() || *pat != tokens.next()) {
      throw std::runtime_error("");
    }
  }
}

class dom::parser {

  static void update_pointers(dom &host, member &dst) {
    if (!dst.short_string) {
      dst.key = host.m_string_pool.data() + dst.offset;
    }
    update_pointers(host, dst.value);
  }

  static void update_pointers(dom &host, value &dst) {
    if (dst.m_type == value::type::string) {
      dst.m_string = host.m_string_pool.data() + dst.m_offset;
    } else if (dst.m_type == value::type::array) {
      dst.m_array = host.m_values.data() + dst.m_offset;
      for (auto it = dst.m_array, ed = it + dst.m_extra; it != ed; ++it) {
        update_pointers(host, *it);
      }
    } else if (dst.m_type == value::type::object) {
      dst.m_object = host.m_members.data() + dst.m_offset;
      for (auto it = dst.m_object, ed = it + dst.m_extra; it != ed; ++it) {
        update_pointers(host, *it);
      }
    }
  }

public:
  static void parse(dom &host, token_stream &tokens) {
    if (tokens.has_next()) {
      auto ch = tokens.next();
      if (ch == '[') {
        parse_array(host, tokens, host.m_root);
      } else if (ch == '{') {
        parse_object(host, tokens, host.m_root);
      } else {
        throw std::runtime_error(std::string("Illegal character for the beginning: ") + ch);
      }
    } else {
      throw std::runtime_error("There is no any thing in the file!");
    }
    update_pointers(host, host.m_root);
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

      value &current = values.emplace_back();
      parse_value(host, tokens, current);
    }

    tokens.consume(); // ]

    auto start = host.m_values.size();
    host.m_values.reserve(start + dst.m_extra);
    std::copy(
        values.begin(), values.end(),
        std::back_insert_iterator(host.m_values)
    );

    dst.m_type = dom::value::type::array;
    dst.m_extra = values.size();
    dst.m_offset = start;
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
        parse_string_value(host, tokens, dst);
        break;
      case '[':
        parse_array(host, tokens, dst);
        break;
      case 'n':
        dst.m_type = value::type::null;
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
      dst.m_bool = true;
    } else if (ch == 'f') {
      expect("false", tokens);
      dst.m_bool = false;
    }
    dst.m_type = value::type::boolean;
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
        number = number * 10 + (ch ^ '0') * div;
        if (div < 1) {
          div *= .1;
        }
      } else if (div == 1 && ch == '.') {
        div *= .1;
      } else if (ch == ']' || ch == '}' || ch == ',') {
        break;
      } else {
        throw std::runtime_error("");
      }

      tokens.consume();
    }

    dst.m_type = value::type::number;
    dst.m_number = sgn * number;
  }

  static void parse_string_value(dom &host, token_stream &tokens, value &dst) {
    char str_buf[1 << 10];
    auto cnt = parse_string(host, tokens, str_buf);
    char *str_dst;
    if (cnt <= s_value_sso_max) {
      str_dst = &dst.m_short_string_first;
      dst.m_short_length = cnt;
      dst.m_type = value::type::short_string;
    } else {
      auto start = host.m_string_pool.size();
      host.m_string_pool.resize(start + cnt + 1);
      dst.m_extra = cnt;
      dst.m_type = value::type::string;
      dst.m_offset = start;
      str_dst = host.m_string_pool.data() + start;
    }
    std::memcpy(str_dst, str_buf, cnt);
    str_dst[cnt] = '\0';
  }

  static int parse_string(dom &host, token_stream &tokens, char dst[]) {
    int cnt = 0;
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

    auto mem = host.m_string_pool.data();
    std::sort(members.begin(), members.end(), [mem](const member &a, const member &b) {
      return std::strcmp(a.key_pointer(mem), b.key_pointer(mem)) < 0;
    });

    auto start = host.m_members.size();
    host.m_members.reserve(host.m_members.size() + members.size());
    std::copy(members.begin(), members.end(), std::back_insert_iterator(host.m_members));

    dst.m_type = value::type::object;
    dst.m_extra = members.size();
    dst.m_offset = start;
  }

  static void parse_member(dom &host, token_stream &tokens, member &dst) {
    parse_key(host, tokens, dst);
    expect(":", tokens);
    parse_value(host, tokens, dst.value);
  }

  static void parse_key(dom &host, token_stream &tokens, member &dst) {
    char str_buf[1 << 10];
    auto cnt = parse_string(host, tokens, str_buf);
    dst.short_string = cnt <= s_member_sso_max;
    if (dst.short_string) {
      std::memcpy(&dst.short_string_first, str_buf, cnt);
      (&dst.short_string_first)[cnt] = '\0';
    } else {
      auto start = host.m_string_pool.size();
      host.m_string_pool.resize(start + cnt + 1);
      dst.key = reinterpret_cast<char *>(start);
      std::memcpy(host.m_string_pool.data() + start, str_buf, cnt);
      host.m_string_pool.back() = '\0';
    }
  }
};

dom::dom(const char *file) {
  std::ifstream stream(file);
  token_stream ts(stream);
  parser::parse(*this, ts);
}
