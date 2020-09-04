#pragma once

#include "color.h"
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#define UNICODE
#include <Windows.h>
#include <d3d11.h>

namespace CS300
{
class OS_Win32;

class Render_DX11
{
public:
  Render_DX11(OS_Win32& os);
  ~Render_DX11();

  void DrawScene();
  void SetClearColor(color_RGBA c);

  void Present();
  void ResizeFramebuffer(OS_Win32 const& os);

  ID3D11Device* GetD3D11Device() const;
  ID3D11DeviceContext* GetD3D11Context() const;
private:
  IDXGISwapChain* d3d_swap_chain_;
  ID3D11Device* d3d11_device_;
  ID3D11DeviceContext* d3d11_device_context_;
  ID3D11RenderTargetView* d3d11_render_target_view_;
  ID3D11DepthStencilView* d3d11_depth_stencil_view_;

  ID3D11DepthStencilState* d3d11_depth_stencil_state_;
  ID3D11RasterizerState* d3d11_rasterizer_state_;

  color_RGBA clear_color_;
};

}
