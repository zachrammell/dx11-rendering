#include "Cubemap.h"

#include "render_dx11.h"

#include <string>
#include <vector>

#include "stb_image.h"

namespace CS350
{

  Cubemap::Cubemap(Render_DX11& render, std::string const& folder)
    : cube_texture(nullptr),
      cube_resource_view(nullptr)
  {
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 6;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.CPUAccessFlags = 0;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
    SMViewDesc.Format = texDesc.Format;
    SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    SMViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
    SMViewDesc.TextureCube.MostDetailedMip = 0;

    std::vector<std::string> faces
    {
      "right.jpg",
      "left.jpg",
      "top.jpg",
      "bottom.jpg",
      "front.jpg",
      "back.jpg",
    };

    uint8_t* img_data[6];

    for (int i = 0; i < 6; ++i)
    {
      int width, height, channel_count;
      img_data[i] = stbi_load((folder + faces[i]).c_str(), &width, &height, &channel_count, 4);
      if (img_data[i])
      {
        pData[i].pSysMem = img_data[i];
        pData[i].SysMemPitch = width * 4;
        pData[i].SysMemSlicePitch = 0;

        texDesc.Width = width;
        texDesc.Height = height;
      }
    }

    HRESULT hr = render.GetD3D11Device()->CreateTexture2D(&texDesc, pData, &cube_texture);
    assert(hr == S_OK);

    hr = render.GetD3D11Device()->CreateShaderResourceView(cube_texture, &SMViewDesc, &cube_resource_view);
    assert(hr == S_OK);

    for (uint8_t* img : img_data)
    {
      stbi_image_free(img);
    }
  }

  Cubemap::Cubemap(Render_DX11& render, RenderTexture render_textures[6])
    : cube_texture(nullptr),
      cube_resource_view(nullptr)
  {
    UpdateFromRenderTextures(render, render_textures);
  }

  void Cubemap::UpdateFromRenderTextures(Render_DX11& render, RenderTexture render_textures[6])
  {
    // Each element in the texture array has the same format/dimensions.
    D3D11_TEXTURE2D_DESC texElementDesc = render_textures[0].GetDesc();

    D3D11_TEXTURE2D_DESC texArrayDesc;
    texArrayDesc.Width = texElementDesc.Width;
    texArrayDesc.Height = texElementDesc.Height;
    texArrayDesc.MipLevels = texElementDesc.MipLevels;
    texArrayDesc.ArraySize = 6;
    texArrayDesc.Format = texElementDesc.Format;
    texArrayDesc.SampleDesc.Count = 1;
    texArrayDesc.SampleDesc.Quality = 0;
    texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
    texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texArrayDesc.CPUAccessFlags = 0;
    texArrayDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    if (cube_texture)
    {
      cube_texture->Release();
    }
    HRESULT hr = render.GetD3D11Device()->CreateTexture2D(&texArrayDesc, 0, &cube_texture);
    assert(SUCCEEDED(hr));

    // Copy individual texture elements into texture array.
    ID3D11DeviceContext* pd3dContext;
    render.GetD3D11Device()->GetImmediateContext(&pd3dContext);
    D3D11_BOX sourceRegion;

    // copy the mip map levels of the textures
    for (UINT x = 0; x < 6; x++)
    {
      for (UINT mipLevel = 0; mipLevel < texArrayDesc.MipLevels; mipLevel++)
      {
        sourceRegion.left = 0;
        sourceRegion.right = (texArrayDesc.Width >> mipLevel);
        sourceRegion.top = 0;
        sourceRegion.bottom = (texArrayDesc.Height >> mipLevel);
        sourceRegion.front = 0;
        sourceRegion.back = 1;

        //test for overflow
        if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
          break;

        pd3dContext->CopySubresourceRegion(cube_texture, D3D11CalcSubresource(mipLevel, x, texArrayDesc.MipLevels), 0, 0, 0, render_textures[x].GetTextureResource(), mipLevel, &sourceRegion);
      }
    }

    // Create a resource view to the texture array.
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    viewDesc.Format = texArrayDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    viewDesc.TextureCube.MostDetailedMip = 0;
    viewDesc.TextureCube.MipLevels = texArrayDesc.MipLevels;

    if (cube_resource_view)
    {
      cube_resource_view->Release();
    }
    hr = render.GetD3D11Device()->CreateShaderResourceView(cube_texture, &viewDesc, &cube_resource_view);
    assert(SUCCEEDED(hr));
  }

  void Cubemap::Use(Render_DX11& render, int slot)
  {
    render.GetD3D11Context()->VSSetShaderResources(slot, 1, &cube_resource_view);
    render.GetD3D11Context()->PSSetShaderResources(slot, 1, &cube_resource_view);
  }

}
