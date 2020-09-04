#include "render_dx11.h"

#include <winrt/base.h>

#include "os_win32.h"

namespace CS300
{

Render_DX11::Render_DX11(OS_Win32& os)
  : d3d_swap_chain_{ nullptr },
    d3d11_device_{ nullptr },
    d3d11_device_context_{ nullptr },
    d3d11_render_target_view_{ nullptr },
    clear_color_{ colors::black }
{
  os.AttachRender(this);

  HRESULT hr;
  DXGI_MODE_DESC buffer_description
  {
    static_cast<UINT>(os.GetWidth()),
    static_cast<UINT>(os.GetHeight()), 
    // TODO: get monitor refresh rate
    {60, 1},
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
    DXGI_MODE_SCALING_UNSPECIFIED
  };

  DXGI_SWAP_CHAIN_DESC swap_chain_descriptor
  {
    buffer_description,
    DXGI_SAMPLE_DESC{1, 0},
    DXGI_USAGE_RENDER_TARGET_OUTPUT,
    1,
    os.GetWindowHandle(),
    true,
    DXGI_SWAP_EFFECT_DISCARD
  };

  hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
    &swap_chain_descriptor, &d3d_swap_chain_, &d3d11_device_, nullptr, &d3d11_device_context_);
  assert(SUCCEEDED(hr));

  winrt::com_ptr<ID3D11Texture2D> back_buffer;
  hr = d3d_swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), back_buffer.put_void());
  assert(SUCCEEDED(hr));
  hr = d3d11_device_->CreateRenderTargetView(back_buffer.get(), nullptr, &d3d11_render_target_view_);
  assert(SUCCEEDED(hr));
  D3D11_TEXTURE2D_DESC back_buffer_descriptor = { 0 };
  back_buffer->GetDesc(&back_buffer_descriptor);


  D3D11_DEPTH_STENCIL_DESC depth_stencil_descriptor
  {
    true,
    D3D11_DEPTH_WRITE_MASK_ALL,
    D3D11_COMPARISON_LESS
  };

  hr = d3d11_device_->CreateDepthStencilState(&depth_stencil_descriptor, &d3d11_depth_stencil_state_);
  assert(SUCCEEDED(hr));
  d3d11_device_context_->OMSetDepthStencilState(d3d11_depth_stencil_state_, 1);

  D3D11_TEXTURE2D_DESC depth_stencil_buffer_descriptor
  {
    back_buffer_descriptor.Width,
    back_buffer_descriptor.Height,
    1,
    1,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    back_buffer_descriptor.SampleDesc,
    D3D11_USAGE_DEFAULT,
    D3D11_BIND_DEPTH_STENCIL,
    0,
    0
  };

  winrt::com_ptr<ID3D11Texture2D> depth_stencil_buffer;
  hr = d3d11_device_->CreateTexture2D(
    &depth_stencil_buffer_descriptor,
    nullptr,
    depth_stencil_buffer.put()
  );

  D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_descriptor
  {
    depth_stencil_buffer_descriptor.Format,
    D3D11_DSV_DIMENSION_TEXTURE2D,
    0,
  };
  depth_stencil_view_descriptor.Texture2D.MipSlice = 0;

  d3d11_device_->CreateDepthStencilView(
    depth_stencil_buffer.get(),
    &depth_stencil_view_descriptor,
    &d3d11_depth_stencil_view_
  );

  d3d11_device_context_->OMSetRenderTargets(1, &d3d11_render_target_view_, d3d11_depth_stencil_view_);


  D3D11_RASTERIZER_DESC rasterizer_descriptor
  {
    D3D11_FILL_SOLID,
    D3D11_CULL_BACK,
    true,

  };

  hr = d3d11_device_->CreateRasterizerState(&rasterizer_descriptor, &d3d11_rasterizer_state_);
  assert(SUCCEEDED(hr));
  d3d11_device_context_->RSSetState(d3d11_rasterizer_state_);

  // set up the default viewport to match the window
  {
    RECT window_rect;
    GetClientRect(os.GetWindowHandle(), &window_rect);
    D3D11_VIEWPORT viewport
    {
      0.0f,
      0.0f,
      (FLOAT)(window_rect.right - window_rect.left),
      (FLOAT)(window_rect.bottom - window_rect.top),
      0.0f,
      1.0f
    };
    d3d11_device_context_->RSSetViewports(1, &viewport);
  }
}

Render_DX11::~Render_DX11()
{
  d3d_swap_chain_->Release();
  d3d11_device_->Release();
  d3d11_device_context_->Release();
}

void Render_DX11::DrawScene()
{
  d3d11_device_context_->OMSetRenderTargets(1, &d3d11_render_target_view_, d3d11_depth_stencil_view_);
  d3d11_device_context_->ClearRenderTargetView(d3d11_render_target_view_, clear_color_.data);
  d3d11_device_context_->ClearDepthStencilView(
    d3d11_depth_stencil_view_,
    D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
    1.0f, 
    0
  );
}

void Render_DX11::SetClearColor(color_RGBA c)
{
  clear_color_ = c;
}

void Render_DX11::Present()
{
  d3d_swap_chain_->Present(1, 0);
}

void Render_DX11::ResizeFramebuffer(OS_Win32 const& os)
{
  d3d11_device_context_->OMSetRenderTargets(0, 0, 0);
  d3d11_render_target_view_->Release();

  HRESULT hr = d3d_swap_chain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
  assert(SUCCEEDED(hr));

  ID3D11Texture2D* d3d11_frame_buffer;
  hr = d3d_swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11_frame_buffer);
  assert(SUCCEEDED(hr));

  hr = d3d11_device_->CreateRenderTargetView(d3d11_frame_buffer, nullptr, &d3d11_render_target_view_);
  assert(SUCCEEDED(hr));
  d3d11_frame_buffer->Release();

  // set up the default viewport to match the window
  {
    RECT window_rect;
    GetClientRect(os.GetWindowHandle(), &window_rect);
    D3D11_VIEWPORT viewport
    {
      0.0f,
      0.0f,
      (FLOAT)(window_rect.right - window_rect.left),
      (FLOAT)(window_rect.bottom - window_rect.top),
      0.0f,
      1.0f
    };
    d3d11_device_context_->RSSetViewports(1, &viewport);
  }
}

ID3D11Device* Render_DX11::GetD3D11Device() const
{
  return d3d11_device_;
}

ID3D11DeviceContext* Render_DX11::GetD3D11Context() const
{
  return d3d11_device_context_;
}

}
