#pragma once

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#define UNICODE
#include <Windows.h>

#include "CallbackHelper.h"

#include <array>

namespace IE::Graphics
{
  class GfxDevice_DX11;
}

namespace IE::Platform
{

class OS_Win32
{
public:
  explicit OS_Win32(LPCTSTR title, int width, int height);
  ~OS_Win32();
  void HandleMessages();
  bool ShouldClose() const;

  HWND GetWindowHandle() const;

  int GetWidth() const;
  int GetHeight() const;

  LRESULT WindowProcedure(HWND handle_window,
    UINT message,
    WPARAM w_param,
    LPARAM l_param);

  void AttachRender(Graphics::GfxDevice_DX11* render);
  void Show();

  double GetTime() const;

  struct MouseData
  {
    int left = false, right = false;
    int x, y;
    int dx = 0, dy = 0;
    int scroll_dy = 0;
  };

  struct KeyState
  {
    bool pressed;
  };

  MouseData GetMouseData() const;
  KeyState GetKeyState(char c) const;

private:
  bool should_close_window_;
  HWND window_handle_;
  int width_, height_;
  Graphics::GfxDevice_DX11* render_;
  MouseData mouse_data_;

  CallbackHelper<OS_Win32, 0, WNDPROC> window_procedure_callback_helper_;

  std::array<KeyState, 256> key_states_{};

  class Timer
  {
  public:
    Timer();
    double GetTime() const;
  private:
    LONGLONG start_perf_count_ = 0;
    LONGLONG perf_counter_frequency_ = 0;
  } timer_;
};

};
