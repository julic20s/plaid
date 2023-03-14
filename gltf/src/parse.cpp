#include <string_view>

#include <gltf/parse.h>

#include "json.h"
#include "trie.h"

using namespace gltf;
using namespace gltf::internal;

static version parse_version(std::string_view str) {
  version res{};
  auto it = str.begin();
  for (; it != str.end(); ++it) {
    if (*it == '.') break;
    res.mojar = res.mojar * 10 + (*it ^ '0');
  }

  for (++it; it != str.end(); ++it) {
    res.minor = res.minor * 10 + (*it ^ '0');
  }
  return res;
}

template <class Ty>
struct traits {
  static void parse_object(gltf::header &h, std::istream &stream) {
    if (!stream.good()) {
      return;
    }
    stream >> json::object_begin;
    auto &trie = Ty::trie();
    while (true) {
      if (!stream.good()) {
        return;
      }
      if (!json::has_char_token<json::quote>(stream)) {
        break;
      }
      json::json_string key;
      stream >> key;
      if (!stream.good()) {
        return;
      }
      auto ptr = trie.find(key.c_str());
      if (ptr == nullptr) {
        stream.setstate(std::istream::failbit);
        return;
      }

      stream >> json::key_separator;
      if (!stream.good()) {
        return;
      }

      Ty::parse_field(h, stream, ptr);

      if (!json::has_char_token<json::comma>(stream)) {
        break;
      }
    }
    stream >> json::object_end;
  }
};

namespace fields {

class asset {
private:

  static constexpr auto copyright = "copyright";
  static constexpr auto generator = "generator";
  static constexpr auto version = "version";
  static constexpr auto min_version = "minVersion";
  static constexpr auto extensions = "extensions";
  static constexpr auto extras = "extras";

public:

  static const trie &trie() {
    static const class trie inst = {
        copyright, generator, version, min_version, extensions, extras};
    return inst;
  }

  static void parse_field(header &dst, std::istream &stream, const char *field) {
    json::json_string str;
    if (field == copyright) {
      if (stream >> str) {
        dst.asset.copyright = str;
      }
    } else if (field == generator) {
      if (stream >> str) {
        dst.asset.generator = str;
      }
    } else if (field == version) {
      if (stream >> str) {
        dst.asset.version = parse_version(str);
      }
    } else if (field == min_version) {
      if (stream >> str) {
        dst.asset.min_version = parse_version(str);
      }
    } else if (field == extensions) {
    } else if (field == extras) {
    }
  }
};

class root {
private:

  static constexpr auto asset = "asset";
  static constexpr auto scenes = "scenes";
  static constexpr auto buffers = "buffers";
  static constexpr auto camera = "camera";
  static constexpr auto buffer_views = "bufferViews";

public:

  static const trie &trie() {
    static const class trie inst = {asset, scenes, buffers, camera, buffers};
    return inst;
  }

  static void parse_field(header &h, std::istream &stream, const char *field) {
    if (field == asset) {
      traits<class asset>::parse_object(h, stream);
    } else if (field == scenes) {
    }
  }
};

} // namespace fields

void gltf::parse(std::istream &stream, header &dst) {
  traits<fields::root>::parse_object(dst, stream);
}
