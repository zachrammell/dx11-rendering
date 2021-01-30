#pragma once

#include <vector>
#include "texture.h"

namespace CS350
{

class Render_DX11;

class Framebuffer
{
public:
  int GetTargetCount() const;
private:
  friend Render_DX11;
  Framebuffer(Render_DX11& render, int width, int height, int target_count);
  int target_count_;
  std::vector<ID3D11Texture2D*> textures_;
  std::vector<ID3D11ShaderResourceView*> resource_views_;
  std::vector<ID3D11RenderTargetView*> render_target_views_;
  winrt::com_ptr<ID3D11DepthStencilView> depth_stencil_view_ = nullptr;
  D3D11_VIEWPORT viewport_;
};

}
