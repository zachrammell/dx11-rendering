#pragma once

#include <cstdint>
#define NOMINMAX
#include <d3d11.h>
#include <DirectXMath.h>
#include <winrt/base.h>

#include "image.h"

namespace IE::Platform
{
  class OS_Win32;
}

namespace IE::Graphics
{
class GfxDevice_DX11;

class Texture
{
public:
  D3D11_TEXTURE2D_DESC GetDesc() const;
  ID3D11Texture2D* GetTextureResource() const;
protected:
  explicit Texture() = default;
  winrt::com_ptr<ID3D11Texture2D> texture_ = nullptr;
  winrt::com_ptr<ID3D11ShaderResourceView> resource_view_ = nullptr;
private:
  friend GfxDevice_DX11;
  Texture(GfxDevice_DX11& render, Image const& image);
  Texture(ID3D11Texture2D* texture, ID3D11ShaderResourceView* resource_view);
};

class RenderTexture : public Texture
{
public:
  void Clear(GfxDevice_DX11& render, DirectX::XMFLOAT4 color);
private:
  friend GfxDevice_DX11;
  RenderTexture(GfxDevice_DX11& render, int width, int height);
  winrt::com_ptr<ID3D11RenderTargetView> render_target_view_ = nullptr;
  winrt::com_ptr<ID3D11DepthStencilView> depth_stencil_view_ = nullptr;
  D3D11_VIEWPORT viewport_;
};
}
