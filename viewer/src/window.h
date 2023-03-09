#pragma once
#ifndef PLAID_VIEWER_WINDOW_H_
#define PLAID_VIEWER_WINDOW_H_

#include <functional>
#include <string_view>

/// 窗口句柄类
class window {
private:

  class delegate;

  window(delegate *ptr) : m_ptr(ptr) {}

public:

  /// 回调响应类
  struct events {
    /// 当缓冲表面发生重建时被调用
    /// @param width 新的宽度
    /// @param height 新的高度
    virtual void surface_recreate(window &, std::uint32_t width, std::uint32_t height) {};

    /// 当用户使用鼠标滚轮时被调用
    /// @param distance 滚动距离
    virtual void mouse_wheel(window &, std::int16_t distance) {};

    /// 当用户移动鼠标时被调用
    /// @param x 鼠标水平位置
    /// @param y 鼠标竖直位置
    virtual void mouse_move(window &, short x, short y) {};
  };

  window() noexcept : m_ptr(nullptr) {}

  window(const window &) = delete;

  window(window &&mov) noexcept {
    m_ptr = mov.m_ptr;
    mov.m_ptr = nullptr;
  }

  ~window() {
    if (valid()) {
      destroy();
    }
  }

  /// 指示窗口是否存在
  [[nodiscard]] inline bool valid() noexcept {
    return m_ptr != nullptr;
  }

  /// 显示窗口
  void show();

  /// 是否被请求关闭窗口
  [[nodiscard]] bool should_close();

  // 读取窗口事件
  void poll_events();

  /// 获取缓冲表面
  [[nodiscard]] std::uint32_t *surface();

  /// 将呈现表面标记为失效，缓冲表面将被加载
  void invalidate();

  /// 创建窗口
  [[nodiscard]] static window create(
      std::string_view title, std::uint32_t width, std::uint32_t height
  );

  /// 销毁窗口
  void destroy();

  /// 绑定窗口事件回调
  void bind(events &);

private:

  delegate *m_ptr;
};

#endif // PLAID_VIEWER_WINDOW_H_
