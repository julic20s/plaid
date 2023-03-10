#ifdef PLAID_VIEWER_WIN32

#include <ShellScalingApi.h>
#include <Windows.h>
#include <Windowsx.h>

#include "window.h"

/// WIN32 窗口注册类名
static constexpr auto window_class_name = TEXT("plaid_window");

/// 提供一个默认空回调
static window::events g_default_events;

class window::delegate {
private:

  static constexpr auto window_state_properties = TEXT("plaid_window_state");

  static void setup_dpi_awareness_if_possible() {
    // 需要 Windows 8.1
    auto lib = LoadLibrary(TEXT("Shcore.dll"));
    if (!lib) return;
    reinterpret_cast<decltype(SetProcessDpiAwareness) *>(
        GetProcAddress(lib, "SetProcessDpiAwareness")
    )(PROCESS_SYSTEM_DPI_AWARE);
    FreeLibrary(lib);
  }

  static void register_window_class() {
    auto instance = GetModuleHandle(nullptr);
    const WNDCLASSEX wnd_class{
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = handle_window_message,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = instance,
        .hIcon = LoadIcon(instance, IDI_APPLICATION),
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)),
        .lpszMenuName = nullptr,
        .lpszClassName = window_class_name,
        .hIconSm = wnd_class.hIcon,
    };

    RegisterClassEx(&wnd_class);
  }

  [[nodiscard]] static delegate *get(HWND hwnd) {
    return reinterpret_cast<delegate *>(GetProp(hwnd, window_state_properties));
  }

  static LRESULT CALLBACK handle_window_message(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    auto state = window::delegate::get(hwnd);
    if (!state) {
      return DefWindowProc(hwnd, message, wparam, lparam);
    }
    auto events = state->events;

    window wrapper(state);

    LRESULT ret = 0;
    switch (message) {
      case WM_MOUSEWHEEL:
        events->mouse_wheel(wrapper, HIWORD(wparam));
        break;
      case WM_MOUSEMOVE: {
        events::mouse_movement mov{
            .x = static_cast<std::int32_t>(GET_X_LPARAM(lparam)),
            .y = static_cast<std::int32_t>(GET_Y_LPARAM(lparam)),
            .flag = static_cast<std::uint32_t>(wparam),
        };
        events->mouse_move(wrapper, mov);
        break;
      }
      [[unlikely]] case WM_CLOSE:
        state->should_close = true;
        break;
      [[unlikely]] case WM_SIZE:
        state->recreate_surface(wrapper, LOWORD(lparam), HIWORD(lparam));
        break;
      default:
        ret = DefWindowProc(hwnd, message, wparam, lparam);
        break;
    }

    /// 提前置空防止被 window 析构函数销毁窗口
    wrapper.m_ptr = nullptr;

    return ret;
  }

public:

  static void initialize_if_need() {
    static bool initialized = false;
    if (!initialized) {
      register_window_class();
      setup_dpi_awareness_if_possible();
      initialized = true;
    }
  }

  delegate(HWND hwnd) noexcept : should_close(false), hwnd(hwnd), events(&g_default_events) {
    SetProp(hwnd, window_state_properties, this);
    auto hdc = GetDC(hwnd);
    buffer_dc = CreateCompatibleDC(hdc);
    ReleaseDC(hwnd, hdc);
  }

  ~delegate() {
    RemoveProp(hwnd, window_state_properties);
    DeleteDC(buffer_dc);
  }

  void recreate_surface(window &wnd, LONG width, LONG height) {
    BITMAPINFOHEADER bitmap_header{
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = width,
        .biHeight = -height,
        .biPlanes = 1,
        .biBitCount = 32,
        .biCompression = BI_RGB,
    };

    auto bitmap = CreateDIBSection(
        buffer_dc, reinterpret_cast<BITMAPINFO *>(&bitmap_header),
        DIB_RGB_COLORS, reinterpret_cast<void **>(&surface), nullptr, 0
    );

    DeleteObject(SelectObject(buffer_dc, bitmap));

    this->width = width;
    this->height = height;

    events->surface_recreate(wnd, width, height);
  }

  bool should_close;
  std::uint32_t width;
  std::uint32_t height;
  std::uint32_t *surface;
  events *events;
  const HWND hwnd;
  HDC buffer_dc;
};

window window::create(
    std::string_view title, std::uint32_t width, std::uint32_t height
) {
  delegate::initialize_if_need();

#ifdef UNICODE
  wchar_t title_data[256];
  {
    auto len = title.length();
    MultiByteToWideChar(
        CP_UTF8, 0, title.data(), len, title, len
    );
    title[len] = L'\0';
  }
#else
  const char *title_data = title.data();
#endif

  LONG actual_width = width, actual_height = height;
  {
    RECT rect = {0, 0, actual_width, actual_height};
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, 0);
    actual_width = rect.right - rect.left;
    actual_height = rect.bottom - rect.top;
  }

  auto hwnd = CreateWindowEx(
      0,
      window_class_name,
      title_data,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT,
      actual_width, actual_height,
      nullptr,
      nullptr,
      GetModuleHandle(nullptr),
      nullptr
  );

  if (!hwnd) return nullptr;

  return new delegate(hwnd);
}

void window::show() {
  ShowWindow(m_ptr->hwnd, SW_SHOW);
}

bool window::should_close() {
  return m_ptr->should_close;
}

std::uint32_t *window::surface() {
  return m_ptr->surface;
}

void window::invalidate() {
  auto hdc = GetDC(m_ptr->hwnd);
  BitBlt(hdc, 0, 0, m_ptr->width, m_ptr->height, m_ptr->buffer_dc, 0, 0, SRCCOPY);
  ReleaseDC(m_ptr->hwnd, hdc);
}

void window::poll_events() {
  MSG msg;
  if (PeekMessage(&msg, m_ptr->hwnd, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void window::destroy() {
  DestroyWindow(m_ptr->hwnd);
  delete m_ptr;
  m_ptr = nullptr;
}

void window::bind(events &target) {
  m_ptr->events = &target;
}

#endif
