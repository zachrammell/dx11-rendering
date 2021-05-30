#pragma once
#include <string>
#include <d3d11.h>

namespace IE::Graphics
{
  class RenderTexture;
  class GfxDevice_DX11;

  class Cubemap
  {
  public:
    Cubemap(GfxDevice_DX11& render, std::string const& folder);
    Cubemap(GfxDevice_DX11& render, RenderTexture render_textures[6]);
    void UpdateFromRenderTextures(GfxDevice_DX11& render, RenderTexture render_textures[6]);
    void Use(GfxDevice_DX11& render, int slot);
  private:
    ID3D11Texture2D* cube_texture;
    ID3D11ShaderResourceView* cube_resource_view;
    D3D11_SUBRESOURCE_DATA pData[6];
  };
}
