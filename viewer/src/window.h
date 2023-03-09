#pragma once
#ifndef PLAID_VIEWER_WINDOW_H_
#define PLAID_VIEWER_WINDOW_H_

#include <functional>
#include <string_view>

struct window_state;
struct window_surface;

class window {
private:
  window(window_state *state) : state(state) {}
public:
  window() noexcept : state(nullptr) {}

  window(const window &) = delete;

  window(window &&mov) noexcept {
    state = mov.state;
    mov.state = nullptr;
  }

  ~window() {
    if (valid()) {
      destroy();
    }
  }

  [[nodiscard]] inline bool valid() noexcept { return state != nullptr; }

  /// 显示窗口
  void show();

  /// 是否被请求关闭窗口
  [[nodiscard]] bool should_close();

  // 读取窗口事件
  void poll_events();

  [[nodiscard]] std::uint32_t *surface();

  void clear_surface(std::uint32_t value);

  void on_mouse_wheel(std::function<void(window &, std::int32_t)>);

  void on_surface_recreate(std::function<void(window &, std::uint32_t width, std::uint32_t height)>);

  void on_mouse_move(std::function<void(window &, short, short, short, short)>);

  void commit();

  // 创建窗口
  [[nodiscard]] static window create(
      std::string_view title, std::uint32_t width, std::uint32_t height
  );

  // 销毁窗口
  void destroy();

private:

  window_state *state;

  friend class window_state;
};

#endif // PLAID_VIEWER_WINDOW_H_
