#ifdef PLAID_VIEWER_WIN32

#include "window.h"

struct window_state {
  bool should_close;
  HWND hwnd;

  static constexpr auto window_state_properties = TEXT("plaid_window_state");

  void inject(HWND hwnd) {
    this->hwnd = hwnd;
    SetProp(hwnd, window_state_properties, this);
  }

  [[nodiscard]] static window_state &get(HWND hwnd) {
    return *reinterpret_cast<window_state *>(GetProp(hwnd, window_state_properties));
  }
};

static auto window_class_name = TEXT("plaid_window");

static LRESULT CALLBACK handle_window_message(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  auto &state = window_state::get(hwnd);

  if (message == WM_CLOSE) {
    state.should_close = true;
    return 0;
  } else {
    return DefWindowProc(hwnd, message, wparam, lparam);
  }
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

static void setup_dpi_awareness_if_possible() {
  // 需要 Windows 8.1
  auto lib = LoadLibrary(TEXT("Shcore.dll"));
  if (!lib) return;
  reinterpret_cast<decltype(SetProcessDpiAwareness) *>(
      GetProcAddress(lib, "SetProcessDpiAwareness")
  )(PROCESS_SYSTEM_DPI_AWARE);
  FreeLibrary(lib);
}

window window::create(
    std::string_view title, std::uint32_t width, std::uint32_t height
) {
  static bool initialized = false;
  if (!initialized) {
    register_window_class();
    setup_dpi_awareness_if_possible();
    initialized = true;
  }

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
      WS_EX_OVERLAPPEDWINDOW,
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

  auto state = new window_state{
      .should_close = false,
  };
  state->inject(hwnd);
  return state;
}

void window::show() {
  ShowWindow(state->hwnd, SW_SHOW);
}

bool window::should_close() {
  return state->should_close;
}

void window::poll_events() {
  MSG msg;
  if (PeekMessage(&msg, state->hwnd, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void window::destroy() {
  DestroyWindow(state->hwnd);
  delete state;
  state = nullptr;
}

#endif
