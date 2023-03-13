#pragma once

#include <istream>
#include <stdexcept>
#include <string>

namespace gltf::internal {

namespace json {

struct char_token {
  char ch;
};

template <class Char, class Traits, class Alloc>
struct basic_json_string : public std::basic_string<Char, Traits, Alloc> {

  template <class... Args>
  basic_json_string(Args &&...args)
      : std::basic_string<Char, Traits, Alloc>(
            std::forward<Args>(args)...
        ) {}
};

using json_string = basic_json_string<
    std::string::value_type,
    std::string::traits_type,
    std::string::allocator_type>;

constexpr char_token object_begin{'{'};
constexpr char_token object_end{'}'};
constexpr char_token array_begin{'['};
constexpr char_token array_end{']'};
constexpr char_token key_separator{':'};
constexpr char_token comma{','};
constexpr char_token quote{'"'};

template <class Char, class Traits>
[[nodiscard]] bool has_key(std::basic_istream<Char, Traits> &stream) {
  using istream = std::basic_istream<Char, Traits>;
  using ctype = std::ctype<Char>;

  typename istream::iostate state = istream::goodbit;

  if (typename istream::sentry(stream)) {
    const ctype &fac = std::use_facet<ctype>(stream.getloc());
    auto meta = stream.rdbuf()->sgetc();
    while (1) {
      if (Traits::eq_int_type(Traits::eof(), meta)) {
        return false;
      }
      if (fac.is(ctype::space, Traits::to_char_type(meta))) {
        meta = stream.rdbuf()->snextc();
        continue;
      }
      return Traits::eq_int_type(json::quote.ch, Traits::to_char_type(meta));
    }
  }
  return false;
}

} // namespace json

class json_parsing_error : public std::runtime_error {
private:
  template <class Expected, class Actual>
  static std::string error_message(Expected &&expected, Actual &&actual) {
    return "Expected: \"" + std::to_string(std::forward<Expected>(expected)) +
           "\"; Actual: \"" + std::to_string(std::forward<Actual>(actual)) + "\";";
  }

public:
  template <class... Args>
  constexpr json_parsing_error(Args &&...args) noexcept
      : std::runtime_error(error_message(std::forward<Args>(args)...)) {}
};

} // namespace gltf::internal

template <class Char, class Traits>
decltype(auto) operator>>(
    std::basic_istream<Char, Traits> &stream,
    gltf::internal::json::char_token token
) {
  using istream = std::basic_istream<Char, Traits>;
  using ctype = std::ctype<Char>;

  typename istream::iostate state = istream::goodbit;
  bool found = false;

  if (typename istream::sentry(stream)) {
    const ctype &fac = std::use_facet<typename istream::_Ctype>(stream.getloc());

    typename Traits::int_type meta = stream.rdbuf()->sgetc();

    for (;; meta = stream.rdbuf()->snextc()) {
      if (Traits::eq_int_type(Traits::eof(), meta)) {
        state |= istream::eofbit;
        break;
      } else if (!fac.is(ctype::space, Traits::to_char_type(meta))) {
        // FIXME: 宽字符适配
        found = Traits::eq_int_type(token.ch, Traits::to_char_type(meta));
        meta = stream.rdbuf()->snextc();
        break;
      }
    }
  }

  if (!found) {
    state |= istream::failbit;
  }
  stream.setstate(state);
  return stream;
}

template <class Char, class Traits, class Alloc>
decltype(auto) operator>>(
    std::basic_istream<Char, Traits> &stream,
    gltf::internal::json::basic_json_string<Char, Traits, Alloc> &str
) {
  stream >> gltf::internal::json::quote;

  using istream = std::basic_istream<Char, Traits>;
  using ctype = std::ctype<Char>;

  typename istream::iostate state = istream::goodbit;

  if (typename istream::sentry(stream)) {
    const ctype &fac = std::use_facet<typename istream::_Ctype>(stream.getloc());

    typename Traits::int_type meta = stream.rdbuf()->sgetc();

    bool escape = false;
    bool exit = false;
    for (; !exit; meta = stream.rdbuf()->snextc()) {
      if (Traits::eq_int_type(Traits::eof(), meta)) {
        state |= istream::failbit | istream::eofbit;
        break;
      } else if (Traits::eq_int_type('"', Traits::to_char_type(meta))) {
        if (!escape) {
          exit = true;
          continue;
        }
      } else if (Traits::eq_int_type('\\', Traits::to_char_type(meta))) {
        if (!escape) {
          escape = true;
          continue;
        }
      }
      str.push_back(Traits::to_char_type(meta));
      escape = false;
    }
  }

  stream.setstate(state);

  return stream;
}
