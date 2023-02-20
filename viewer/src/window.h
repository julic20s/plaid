#pragma once
#ifndef PLAID_VIEWER_WINDOW_H_
#define PLAID_VIEWER_WINDOW_H_

struct window_state;

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

  // 创建窗口
  [[nodiscard]] static window create(
      std::string_view title, std::uint32_t width, std::uint32_t height
  );

  // 销毁窗口
  void destroy();

private:
  window_state *state;
};

#endif // PLAID_VIEWER_WINDOW_H_
