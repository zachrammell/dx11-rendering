#include "texture.h"

#include "render_dx11.h"

namespace CS350
{
Texture::Texture(Render_DX11& render, Image const& image)
{
  HRESULT hr;

  //upload texture to GPU
  D3D11_TEXTURE2D_DESC texture_2d_desc{};
  texture_2d_desc.Width = image.GetWidth();
  texture_2d_desc.Height = image.GetHeight();
  texture_2d_desc.MipLevels = texture_2d_desc.ArraySize = 1;
  texture_2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  texture_2d_desc.SampleDesc.Count = 1;
  texture_2d_desc.Usage = D3D11_USAGE_DEFAULT;
  texture_2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  D3D11_SUBRESOURCE_DATA texture_data{ image.GetData(), UINT(image.GetPitch()) };

  hr = render.GetD3D11Device()->CreateTexture2D(&texture_2d_desc, &texture_data, texture_.put());
  if (FAILED(hr)) __debugbreak();

  D3D11_SHADER_RESOURCE_VIEW_DESC resource_view_desc{};
  resource_view_desc.Format = texture_2d_desc.Format;
  resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  resource_view_desc.Texture2D = { 0, UINT(-1) };

  hr = render.GetD3D11Device()->CreateShaderResourceView(texture_.get(), &resource_view_desc, resource_view_.put());
  if (FAILED(hr)) __debugbreak();
}

Texture::Texture(ID3D11Texture2D* texture, ID3D11ShaderResourceView* resource_view)
{
  texture_.attach(texture);
  texture_->AddRef();
  resource_view_.attach(resource_view);
  resource_view_->AddRef();
}

D3D11_TEXTURE2D_DESC Texture::GetDesc() const
{
  D3D11_TEXTURE2D_DESC desc;
  texture_->GetDesc(&desc);
  return desc;
}

ID3D11Texture2D* Texture::GetTextureResource() const
{
  return texture_.get();
}

RenderTexture::RenderTexture(Render_DX11& render, int width, int height)
{
  HRESULT hr;

  D3D11_TEXTURE2D_DESC texture_2d_desc{};
  texture_2d_desc.Width = width;
  texture_2d_desc.Height = height;
  texture_2d_desc.MipLevels = texture_2d_desc.ArraySize = 1;
  texture_2d_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  texture_2d_desc.SampleDesc.Count = 1;
  texture_2d_desc.Usage = D3D11_USAGE_DEFAULT;
  texture_2d_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

  hr = render.GetD3D11Device()->CreateTexture2D(&texture_2d_desc, nullptr, texture_.put());
  if (FAILED(hr)) __debugbreak();

  D3D11_SHADER_RESOURCE_VIEW_DESC resource_view_desc{};
  resource_view_desc.Format = texture_2d_desc.Format;
  resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  resource_view_desc.Texture2D.MostDetailedMip = 0;
  resource_view_desc.Texture2D.MipLevels = 1;

  hr = render.GetD3D11Device()->CreateShaderResourceView(texture_.get(), &resource_view_desc, resource_view_.put());
  if (FAILED(hr)) __debugbreak();

  D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc{};
  render_target_view_desc.Format = texture_2d_desc.Format;
  render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  render_target_view_desc.Texture2D.MipSlice = 0;

  hr = render.GetD3D11Device()->CreateRenderTargetView(texture_.get(), &render_target_view_desc, render_target_view_.put());

  ZeroMemory(&viewport_, sizeof(viewport_));
  viewport_.Height = height;
  viewport_.Width = width;
  viewport_.MinDepth = 0.0f;
  viewport_.MaxDepth = 1.0f;
  viewport_.TopLeftX = 0;
  viewport_.TopLeftY = 0;

}

void RenderTexture::Clear(Render_DX11& render, DirectX::XMFLOAT4 color)
{
  render.GetD3D11Context()->ClearRenderTargetView(render_target_view_.get(), &color.x);
  render.GetD3D11Context()->ClearDepthStencilView(depth_stencil_view_.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

}
