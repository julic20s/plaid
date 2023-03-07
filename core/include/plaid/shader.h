#pragma once
#ifndef PLAID_SHADER_H_
#define PLAID_SHADER_H_

#include <cstddef>
#include <cstdint>

#include <type_traits>

#include "format.h"

#ifdef PLAID_SHADER_DSL
#include "mat.h"
#include "vec.h"
#endif

namespace plaid {

/// 定义一个着色器输入/输出变量的规格
struct shader_stage_variable_description {
  /// 变量插值函数
  using interpolation_function = void(
      const std::byte *(&src)[3],
      const float (&weight)[3],
      std::byte *dst
  );

  /// 属性的格式
  format format;
  /// 属性的编号
  std::uint8_t location;
  /// 属性的字节大小
  std::uint32_t size;
  /// 属性的字节对齐大小
  std::uint32_t align;
  /// 属性插值函数，只对片元着色器输入有效
  interpolation_function *interpolation;
};

/// 定义一个着色器内所有变量的规格
struct shader_variables_meta {
  std::uint16_t inputs_count;
  std::uint16_t outputs_count;
  shader_stage_variable_description *inputs;
  shader_stage_variable_description *outputs;
};

/// 保存着色器信息，管道对象可以据此为属性分配内存
struct shader_module {
  /// 着色器入口函数
  using entry_function = void(
      const std::byte *(&uniform)[1 << 8],
      const std::byte *(&input)[1 << 8],
      std::byte *(&output)[1 << 8],
      std::byte **mutable_builtin
  );

  virtual ~shader_module() {}

  shader_variables_meta variables_meta;
  entry_function *entry;
};

#ifdef PLAID_SHADER_DSL

/// 使用 DSL 能以接近 GLSL 等着色器语言的书写方式来完成着色器的编写
/// 并自动生成对应的 [shader_module]
class shader {
public:
  /// 着色器属性语法糖，应该注意的是，所有的属性都占用 1 个 location 而不像
  /// GLSL 一个属性会占用多个 location
  template <std::uint8_t>
  struct location;

  /// 着色器常量语法糖
  template <std::uint8_t>
  struct binding;

  /// 用来生成着色器类的入口函数
  template <class Ty, void (Ty::*Entry)()>
  static void entry(
      const std::byte *(&uniform)[1 << 8],
      const std::byte *(&input)[1 << 8],
      std::byte *(&output)[1 << 8],
      std::byte **mutable_builtin
  );

private:
  const std::byte **uniform;
  const std::byte **input;
  std::byte **output;
};

/// 顶点着色器基类
struct vertex_shader : shader {
  vec4 *gl_position;
};

/// 片元着色器基类
struct fragment_shader : shader {};

template <class Ty, void (Ty::*Entry)()>
void shader::entry(
    const std::byte *(&uniform)[1 << 8],
    const std::byte *(&input)[1 << 8],
    std::byte *(&output)[1 << 8],
    std::byte **mutable_builtin
) {
  Ty shader;
  shader.uniform = uniform;
  shader.input = input;
  shader.output = output;
  if constexpr (std::is_base_of_v<vertex_shader, Ty>) {
    shader.gl_position = reinterpret_cast<vec4 *>(mutable_builtin[0]);
  }
  (shader.*Entry)();
}

// 匹配不同类型变量的 format
template <class Ty>
struct attribute_format_matcher;

class dsl_shader_module : public shader_module {
private:
  template <auto>
  struct constructor;

public:
  /// 生成将指定的着色器类成员函数作为着色器入口函数的着色器模块
  template <auto Entry>
  static dsl_shader_module load() {
    dsl_shader_module inst;
    // 这里需要萃取出具体的成员函数，直接利用偏特化实现了这个功能
    constructor<Entry>::construct(inst);
    return inst;
  }

  dsl_shader_module() = default;

  dsl_shader_module(const dsl_shader_module &) = delete;

  dsl_shader_module(dsl_shader_module &&mov) noexcept {
    attributes_description_memory = mov.attributes_description_memory;
    entry = mov.entry;
    variables_meta = mov.variables_meta;
    mov.entry = nullptr;
    mov.attributes_description_memory = nullptr;
  }

  ~dsl_shader_module() {
    if (attributes_description_memory) {
      delete attributes_description_memory;
    }
  }

private:
  /// 记录堆内存指针
  shader_stage_variable_description *attributes_description_memory;
};

template <class Ty, void (Ty::*Entry)(void)>
class dsl_shader_module::constructor<Entry> {
private:
  static bool generate_variables_meta(dsl_shader_module &p, auto &&...args) {
    // 不断构造，因为所有的 in/out/uniform 都能接受一个dsl_shader_module & 作为构造参数
    // 所以当不能再构造的时候，arg的数量就是着色器成员的数量
    constexpr auto constructiable = requires { Ty{{}, args...}; };
    if constexpr (constructiable) {
      // 再增加一个参数就不能构造了，说明递归到这一层时参数已经能够填满所有的成员
      if (!generate_variables_meta(p, p, args...)) {
        // 注意这些数组并不在堆内存上
        shader_stage_variable_description inputs[256];
        shader_stage_variable_description outputs[256];
        p.variables_meta = {
            .inputs_count = 0,
            .outputs_count = 0,
            .inputs = inputs,
            .outputs = outputs,
        };
        {
          Ty shader{{}, args...};
        }

        const auto count = p.variables_meta.inputs_count + p.variables_meta.outputs_count;
        if (count == 0) {
          p.variables_meta.inputs = p.variables_meta.outputs = nullptr;
          p.attributes_description_memory = nullptr;
          return true;
        }

        // 为着色器属性描述分配内存
        p.attributes_description_memory = new shader_stage_variable_description[count];

        // 拷贝栈上数组的数据到堆内存
        auto dst = p.attributes_description_memory;
        auto src = p.variables_meta.inputs;
        p.variables_meta.inputs = dst;
        for (auto ed = dst + p.variables_meta.inputs_count; dst != ed; ++dst, ++src) {
          *dst = *src;
        }

        src = p.variables_meta.outputs;
        p.variables_meta.outputs = dst;
        for (auto ed = dst + p.variables_meta.outputs_count; dst != ed; ++dst, ++src) {
          *dst = *src;
        }
      }
    }
    return constructiable;
  }

public:
  static void construct(dsl_shader_module &m) {
    generate_variables_meta(m);
    m.entry = shader::entry<Ty, Entry>;
  }
};

template <std::uint8_t Loc>
struct shader::location {
  template <class Ty>
  class in {
  private:
    template <class MTy>
    static inline void interpolation(
        const std::byte *(&src)[3], const float (&weight)[3], std::byte *dst
    ) {
      constexpr auto n = std::extent_v<Ty>;
      // 如果是数组类型就递归进行插值
      if constexpr (n >= 2) {
        using element_t = std::remove_extent_t<Ty>;
        constexpr auto stride = sizeof(element_t);
        const std::byte *arr_src[] = {src[0], src[1], src[2]};

        for (std::size_t i = 0; i != n; ++i) {
          interpolation<element_t>(arr_src, weight, dst);
          for (auto &p : arr_src) p += stride;
          dst += stride;
        }
      } else {
        auto &typed = reinterpret_cast<const Ty *(&)[3]>(src);
        *reinterpret_cast<Ty *>(dst) =
            *typed[0] * weight[0] +
            *typed[1] * weight[1] +
            *typed[2] * weight[2];
      }
    }

  public:
    in() = default;

    in(dsl_shader_module &m) noexcept {
      // 把自身属性填入数组
      auto &dst = m.variables_meta.inputs_count;
      const_cast<shader_stage_variable_description &>(m.variables_meta.inputs[dst++]) =
          {
              .location = Loc,
              .size = sizeof(Ty),
              .align = alignof(Ty),
              .interpolation = interpolation<Ty>,
          };
    }

    [[nodiscard]] static inline const Ty &
    get(shader *host) noexcept {
      return *reinterpret_cast<const Ty *>(host->input[Loc]);
    }
  };

  template <class Ty>
  struct out {
    out() = default;

    out(dsl_shader_module &m) noexcept {
      // 把自身属性填入数组
      auto &dst = m.variables_meta.outputs_count;
      const_cast<shader_stage_variable_description &>(m.variables_meta.outputs[dst++]) =
          {
              .format = attribute_format_matcher<Ty>::format,
              .location = Loc,
              .size = sizeof(Ty),
              .align = alignof(Ty),
          };
    }

    [[nodiscard]] static inline Ty &
    get(shader *host) noexcept {
      return *reinterpret_cast<Ty *>(host->output[Loc]);
    }
  };
};

template <std::uint8_t Bd>
struct shader::binding {
  template <class Ty>
  struct uniform {
    uniform() = default;

    uniform(dsl_shader_module &m) noexcept {
      // 显然着色器常量是由外部提供的，但为了保证能够让上面的 constructor 类
      // 能够正常构造，特添加这个函数
    }

    [[nodiscard]] inline static const Ty &
    get(shader *shader) noexcept {
      return *reinterpret_cast<const Ty *>(shader->uniform[Bd]);
    }
  };
};

template <>
struct attribute_format_matcher<vec2> {
  static constexpr auto format = format::RG32f;
};

template <>
struct attribute_format_matcher<vec3> {
  static constexpr auto format = format::RGB32f;
};

template <>
struct attribute_format_matcher<vec4> {
  static constexpr auto format = format::RGBA32f;
};

#endif

} // namespace plaid

#endif // PLAID_SHADER_H_
