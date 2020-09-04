#include "os_win32.h"

#include "render_dx11.h"

namespace CS300
{

OS_Win32::OS_Win32(LPCTSTR title, int width, int height)
  : should_close_window_{ false },
    window_handle_{ nullptr },
    width_{ width }, height_{ height },
    render_{nullptr},

    window_procedure_callback_helper_{this, &OS_Win32::WindowProcedure}
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
    width_, height_,
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

  ShowWindow(window_handle_, true);
  UpdateWindow(window_handle_);
}

void OS_Win32::HandleMessages()
{
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
  switch (message)
  {
  case WM_KEYDOWN:
  {
    if (w_param == VK_ESCAPE)
    {
      if (MessageBox(nullptr, TEXT("Are you sure you want to exit?"),
        TEXT("Really?"), MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES)
      {
        DestroyWindow(handle_window);
      }
    }
    return 0;
  }

  case WM_DESTROY:
  {
    PostQuitMessage(0);
    return 0;
  }

  case WM_SIZE:
  {
    if (render_)
    {
      render_->ResizeFramebuffer(*this);
    }
    break;
  }

  default:
    break;
  }
  return DefWindowProc(handle_window, message, w_param, l_param);
}

void OS_Win32::AttachRender(Render_DX11* render)
{
  render_ = render;
}

double OS_Win32::GetTime() const
{
  return timer_.GetTime();
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
