#include "framebuffer.h"

#include "render_dx11.h"

namespace CS350
{

int Framebuffer::GetTargetCount() const
{
  return target_count_;
}

Framebuffer::Framebuffer(Render_DX11& render, int width, int height, int target_count)
  : target_count_{ target_count },
  textures_{ decltype(textures_)::size_type(GetTargetCount()) },
  resource_views_{ decltype(resource_views_)::size_type(GetTargetCount()) },
  render_target_views_{ decltype(render_target_views_)::size_type(GetTargetCount()) }
{

  D3D11_TEXTURE2D_DESC texture_2d_desc{};
  texture_2d_desc.Width = width;
  texture_2d_desc.Height = height;
  texture_2d_desc.MipLevels = texture_2d_desc.ArraySize = 1;
  texture_2d_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  texture_2d_desc.SampleDesc.Count = 1;
  texture_2d_desc.Usage = D3D11_USAGE_DEFAULT;
  texture_2d_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

  D3D11_SHADER_RESOURCE_VIEW_DESC resource_view_desc{};
  resource_view_desc.Format = texture_2d_desc.Format;
  resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  resource_view_desc.Texture2D.MostDetailedMip = 0;
  resource_view_desc.Texture2D.MipLevels = 1;

  D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc{};
  render_target_view_desc.Format = texture_2d_desc.Format;
  render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  render_target_view_desc.Texture2D.MipSlice = 0;

  HRESULT hr;

  for (int i = 0; i < target_count - 1; ++i)
  {
    auto** tex = textures_.data() + i;
    hr = render.GetD3D11Device()->CreateTexture2D(&texture_2d_desc, nullptr, tex);
    assert(SUCCEEDED(hr));
    hr = render.GetD3D11Device()->CreateShaderResourceView(textures_[i], &resource_view_desc, resource_views_.data() + i);
    assert(SUCCEEDED(hr));
    hr = render.GetD3D11Device()->CreateRenderTargetView(textures_[i], &render_target_view_desc, render_target_views_.data() + i);
    assert(SUCCEEDED(hr));
  }
  texture_2d_desc.Format = DXGI_FORMAT_R32_TYPELESS;
  resource_view_desc.Format = DXGI_FORMAT_R32_FLOAT;
  texture_2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
  {
    int const i = target_count - 1;
    auto** tex = textures_.data() + i;
    hr = render.GetD3D11Device()->CreateTexture2D(&texture_2d_desc, nullptr, tex);
    assert(SUCCEEDED(hr));
    hr = render.GetD3D11Device()->CreateShaderResourceView(textures_[i], &resource_view_desc, resource_views_.data() + i);
    assert(SUCCEEDED(hr));
    D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
    depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
    depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    hr = render.GetD3D11Device()->CreateDepthStencilView(textures_[i], &depth_stencil_view_desc, depth_stencil_view_.put());
    assert(SUCCEEDED(hr));
  }


  ZeroMemory(&viewport_, sizeof(viewport_));
  viewport_.Height = height;
  viewport_.Width = width;
  viewport_.MinDepth = 0.0f;
  viewport_.MaxDepth = 1.0f;
  viewport_.TopLeftX = 0;
  viewport_.TopLeftY = 0;
}

}
