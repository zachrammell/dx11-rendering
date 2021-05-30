#include "OS_Win32.h"

#include "imgui_impl_win32.h"

#include "graphics/render_dx11.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace IE::Platform
{

OS_Win32::OS_Win32(LPCTSTR title, int width, int height)
  : should_close_window_{ false },
  window_handle_{ nullptr },
  width_{}, height_{},
  render_{ nullptr },

  window_procedure_callback_helper_{ this, &OS_Win32::WindowProcedure }
{
  LPCTSTR window_class_name = TEXT("CS300ProjWndClass");
  HICON icon = LoadIcon(NULL, IDI_APPLICATION);
  HMODULE instance_handle = GetModuleHandle(nullptr);

  WNDCLASSEX wc
  {
    sizeof(WNDCLASSEX),
    CS_HREDRAW | CS_VREDRAW,
    window_procedure_callback_helper_.callback,
    0,
    0,
    instance_handle,
    icon,
    LoadCursor(nullptr, IDC_ARROW),
    (HBRUSH)(COLOR_WINDOW + 2),
    nullptr,
    window_class_name,
    icon
  };

  if (!RegisterClassEx(&wc))
  {
    MessageBox(nullptr, TEXT("Error registering class"),
               TEXT("Error"), MB_OK | MB_ICONERROR);
    __debugbreak();
  }

  window_handle_ = CreateWindowEx(
    WS_EX_ACCEPTFILES | WS_EX_APPWINDOW,
    window_class_name,
    title,
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT,
    width, height,
    nullptr,
    nullptr,
    instance_handle,
    nullptr
  );

  if (!window_handle_)
  {
    MessageBox(nullptr, TEXT("Error creating window"),
               TEXT("Error"), MB_OK | MB_ICONERROR);
    __debugbreak();
  }

  {
    RECT rc;

    GetWindowRect(window_handle_, &rc);

    int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
    int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;

    SetWindowPos(window_handle_, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  }

  // Update stored width and height to represent only the client area
  {
    RECT window_rect;
    GetClientRect(window_handle_, &window_rect);
    width_ = (window_rect.right - window_rect.left);
    height_ = (window_rect.bottom - window_rect.top);
  }
}

OS_Win32::~OS_Win32()
{
  DestroyWindow(window_handle_);
}

void OS_Win32::HandleMessages()
{
  mouse_data_.dx = mouse_data_.dy = mouse_data_.scroll_dy = 0;
  MSG msg{};
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
  {
    if (msg.message == WM_QUIT)
    {
      should_close_window_ = true;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

bool OS_Win32::ShouldClose() const
{
  return should_close_window_;
}

HWND OS_Win32::GetWindowHandle() const
{
  return window_handle_;
}

int OS_Win32::GetWidth() const
{
  return width_;
}

int OS_Win32::GetHeight() const
{
  return height_;
}

LRESULT OS_Win32::WindowProcedure(HWND handle_window,
                                  UINT message,
                                  WPARAM w_param,
                                  LPARAM l_param)
{
  ImGui_ImplWin32_WndProcHandler(handle_window, message, w_param, l_param);

  switch (message)
  {
  case WM_KEYDOWN:
  {
    if (w_param == VK_ESCAPE)
    {
      if (MessageBox(nullptr, TEXT("Are you sure you want to exit?"),
                     TEXT("Really?"), MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES)
      {
        should_close_window_ = true;
      }
    }
    
    char c = LOWORD(MapVirtualKeyW(w_param, MAPVK_VK_TO_CHAR));
    if (c == '\0')
      c = LOWORD(w_param);
    if (c < 0)
      return 1;
    key_states_[c].pressed = true;
    
    return 0;
  }

  case WM_KEYUP:
  {
    char c = LOWORD(MapVirtualKeyW(w_param, MAPVK_VK_TO_CHAR));
    if (c == '\0')
      c = LOWORD(w_param);
    if (c < 0)
      return 1;
    key_states_[c].pressed = false;
    return 0;
  }

  case WM_CLOSE:
  {
    should_close_window_ = true;
    return 0;
  }

  case WM_SIZE:
  {
    if (render_)
    {
      width_ = LOWORD(l_param);
      height_ = HIWORD(l_param);
      render_->ResizeFramebuffer(*this);
    }
    break;
  }

  case WM_MOUSEMOVE:
  {
    POINTS mouse_pos = MAKEPOINTS(l_param);
    mouse_data_.dx = mouse_pos.x - mouse_data_.x;
    mouse_data_.x = mouse_pos.x;
    mouse_data_.dy = mouse_pos.y - mouse_data_.y;
    mouse_data_.y = mouse_pos.y;
    break;
  }

  case WM_RBUTTONDOWN:
  {
    mouse_data_.right = true;
    break;
  }

  case WM_LBUTTONDOWN:
  {
    mouse_data_.left = true;
    break;
  }

  case WM_RBUTTONUP:
  {
    mouse_data_.right = false;
    break;
  }

  case WM_LBUTTONUP:
  {
    mouse_data_.left = false;
    break;
  }

  case WM_MOUSEWHEEL:
  {
    mouse_data_.scroll_dy = GET_WHEEL_DELTA_WPARAM(w_param) / WHEEL_DELTA;
    break;
  }

  default:
    break;
  }
  return DefWindowProc(handle_window, message, w_param, l_param);
}

void OS_Win32::AttachRender(Graphics::GfxDevice_DX11* render)
{
  render_ = render;
}

void OS_Win32::Show()
{
  ShowWindow(window_handle_, SW_SHOWDEFAULT);
  UpdateWindow(window_handle_);
}

double OS_Win32::GetTime() const
{
  return timer_.GetTime();
}

OS_Win32::MouseData OS_Win32::GetMouseData() const
{
  return mouse_data_;
}

OS_Win32::KeyState OS_Win32::GetKeyState(char c) const
{
  if (islower(c))
    c = toupper(c);
  return key_states_.at(c);
}

OS_Win32::Timer::Timer()
{
  LARGE_INTEGER perf_count;
  QueryPerformanceCounter(&perf_count);
  start_perf_count_ = perf_count.QuadPart;
  LARGE_INTEGER perf_freq;
  QueryPerformanceFrequency(&perf_freq);
  perf_counter_frequency_ = perf_freq.QuadPart;
}

double OS_Win32::Timer::GetTime() const
{
  LARGE_INTEGER perfCount;
  QueryPerformanceCounter(&perfCount);
  return static_cast<double>(perfCount.QuadPart - start_perf_count_) / static_cast<double>(perf_counter_frequency_);
}

}
