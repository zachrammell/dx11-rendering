/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: render_dx11.cpp
Purpose: DX11 layer
Language: C++
Platform: Windows 8.1+, MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS350_1
Author: Zach Rammell, zach.rammell
Creation date: 10/2/20
End Header --------------------------------------------------------*/

#include "render_dx11.h"

#include <iostream>
#include <winrt/base.h>

#include "math_helper.h"
#include "os_win32.h"

namespace dx = DirectX;

namespace CS350
{

Render_DX11::Render_DX11(OS_Win32& os)
  : swap_chain_{ nullptr },
  device_{ nullptr },
  device_context_{ nullptr },
  render_target_view_{ nullptr },
  clear_color_{ 0, 0, 0, 1.0f }
{
  os.AttachRender(this);

  HRESULT hr;
  DXGI_MODE_DESC buffer_description
  {
    UINT(os.GetWidth()),
    UINT(os.GetHeight()),
    // TODO: get monitor refresh rate
    {},
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
    DXGI_SWAP_EFFECT_SEQUENTIAL,
    0
  };

  hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION,
                                     &swap_chain_descriptor, swap_chain_.put(), device_.put(), nullptr,
                                     device_context_.put());
  assert(SUCCEEDED(hr));

  ResizeFramebuffer(os);

  D3D11_RASTERIZER_DESC rasterizer_descriptor
  {
    D3D11_FILL_SOLID,
    D3D11_CULL_BACK,
    true,
  };

  hr = device_->CreateRasterizerState(&rasterizer_descriptor, rasterizer_state_.put());
  assert(SUCCEEDED(hr));

  D3D11_RASTERIZER_DESC rasterizer_descriptor_wireframe
  {
    D3D11_FILL_WIREFRAME,
    D3D11_CULL_NONE,
    true,
  };

  hr = device_->CreateRasterizerState(&rasterizer_descriptor_wireframe, rasterizer_state_wireframe_.put());
  assert(SUCCEEDED(hr));

  // set up the default viewport to match the window
  {
    RECT window_rect;
    GetClientRect(os.GetWindowHandle(), &window_rect);
    viewport_ =
    {
      0.0f,
      0.0f,
      (FLOAT)(window_rect.right - window_rect.left),
      (FLOAT)(window_rect.bottom - window_rect.top),
      0.0f,
      1.0f
    };
    device_context_->RSSetViewports(1, &viewport_);
  }

  D3D11_SAMPLER_DESC sampler_desc{};
  sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler_desc.MipLODBias = 0.0f;
  sampler_desc.MaxAnisotropy = 1;
  sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampler_desc.MinLOD = -FLT_MAX;
  sampler_desc.MaxLOD = FLT_MAX;

  hr = GetD3D11Device()->CreateSamplerState(&sampler_desc, sampler_state_.put());
  assert(SUCCEEDED(hr));
}

Render_DX11::~Render_DX11()
{
  swap_chain_->Release();
  device_->Release();
  device_context_->Release();
}

void Render_DX11::ClearDefaultFramebuffer()
{
  GetD3D11Context()->RSSetViewports(1, &viewport_);
  device_context_->ClearRenderTargetView(render_target_view_.get(), &(clear_color_.x));
  device_context_->ClearDepthStencilView(
    depth_stencil_view_.get(),
    D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
    1.0f,
    0
  );
}

void Render_DX11::SetClearColor(dx::XMFLOAT3 c)
{
  clear_color_ = {c.x, c.y, c.z, 1.0f};
}

void Render_DX11::SetClearColor(DirectX::XMFLOAT4 c)
{
  clear_color_ = c;
}

void Render_DX11::Present()
{
  swap_chain_->Present(1, 0);
  temp_texture_storage_.clear();
}

void Render_DX11::ResizeFramebuffer(OS_Win32 const& os)
{
  CleanupRenderTarget();
  DeleteDepthBuffer();
  CreateDepthBuffer(os);
  CreateRenderTarget();
  SetupViewport(os);

  // the windows api is not const-correct. it won't modify this,
  // but we still need a non-const pointer as our 1-element array
  auto* p_render_target_view = render_target_view_.get();
  device_context_->OMSetRenderTargets(1, &p_render_target_view, depth_stencil_view_.get());
  assert(p_render_target_view == render_target_view_.get()); // the pointer should be unchanged
}

Render_DX11::ShaderID Render_DX11::CreateShader(LPCWSTR shader_path, int input_layout, bool GS)
{
  try
  {
    shaders_.emplace_back(Shader{ *this, shader_path, input_layout, GS });
  }
  catch (std::exception& e)
  {
    std::cout << e.what() << std::endl;
    return None;
  }
  return ShaderID(shaders_.size() - 1);
}

void Render_DX11::UseShader(ShaderID id)
{
  if (id == None)
  {
    GetD3D11Context()->VSSetShader(nullptr, nullptr, 0);
    GetD3D11Context()->PSSetShader(nullptr, nullptr, 0);
    GetD3D11Context()->GSSetShader(nullptr, nullptr, 0);
    GetD3D11Context()->IASetInputLayout(nullptr);
    return;
  }
  Shader& shader = shaders_[id];
  GetD3D11Context()->VSSetShader(shader.vertex_shader, nullptr, 0);
  GetD3D11Context()->PSSetShader(shader.pixel_shader, nullptr, 0);
  GetD3D11Context()->GSSetShader(shader.geometry_shader, nullptr, 0);
  GetD3D11Context()->IASetInputLayout(shader.GetInputLayout());
}

void Render_DX11::UseUniform(UniformID uniform, int slot)
{
  ID3D11Buffer* buffer = constant_buffers_[uniform];
  GetD3D11Context()->VSSetConstantBuffers(slot, 1, &buffer);
  GetD3D11Context()->PSSetConstantBuffers(slot, 1, &buffer);
  GetD3D11Context()->GSSetConstantBuffers(slot, 1, &buffer);
}

Render_DX11::TextureID Render_DX11::CreateTexture(Image const& image)
{
  image_textures_.emplace_back(Texture{ *this, image });
  textures_.emplace_back(InternalTexture{ uint32_t(image_textures_.size() - 1), InternalTexture::Type::ImageTexture });
  return TextureID(textures_.size() - 1);
}

void Render_DX11::UseTexture(TextureID texture, int slot)
{
  Texture& tex = *GetTexture(textures_[texture]);
  auto* resource_view = tex.resource_view_.get();
  GetD3D11Context()->VSSetShaderResources(slot, 1, &resource_view);
  GetD3D11Context()->PSSetShaderResources(slot, 1, &resource_view);
}

Render_DX11::TextureID Render_DX11::CreateRenderTexture(int width, int height)
{
  render_textures_.emplace_back(RenderTexture{ *this, width, height });
  textures_.emplace_back(InternalTexture{ uint32_t(image_textures_.size() - 1), InternalTexture::Type::RenderTexture });
  return TextureID(textures_.size() - 1);
}

void Render_DX11::RenderTo(TextureID render_texture)
{
  assert(textures_[render_texture].type == InternalTexture::Type::RenderTexture);
  RenderTexture& render_tex = *(RenderTexture*)GetTexture(textures_[render_texture]);
  auto* render_target_view = render_tex.render_target_view_.get();
  GetD3D11Context()->OMSetRenderTargets(1, &render_target_view, render_tex.depth_stencil_view_.get());
  GetD3D11Context()->RSSetViewports(1, &render_tex.viewport_);
}

void Render_DX11::ClearRenderTexture(TextureID render_texture)
{
  assert(textures_[render_texture].type == InternalTexture::Type::RenderTexture);
  RenderTexture const& render_tex = *(RenderTexture*)GetTexture(textures_[render_texture]);
  GetD3D11Context()->ClearRenderTargetView(render_tex.render_target_view_.get(), &(clear_color_.x));
}

Render_DX11::MeshID Render_DX11::CreateMesh(Mesh const& m)
{
  meshes_.emplace_back(Mesh_D3D{ *this, m });
  return MeshID(meshes_.size() - 1);
}

Render_DX11::MeshID Render_DX11::CreateMesh(std::vector<Mesh::Vertex> const& vertex_buffer)
{
  meshes_.emplace_back(Mesh_D3D{ *this, vertex_buffer });
  return MeshID(meshes_.size() - 1);
}

void Render_DX11::UseMesh(MeshID mesh)
{
  if (bound_mesh != mesh)
  {
    bound_mesh = mesh;
    if (mesh == None)
    {
      GetD3D11Context()->IASetInputLayout(nullptr);
      return;
    }
    Mesh_D3D& mesh_d3d = meshes_[mesh];
    UINT vertex_stride = sizeof(Mesh::Vertex);
    UINT vertex_offset = 0;

    ID3D11Buffer* buffer = mesh_d3d.vertex_buffer_.get();
    GetD3D11Context()->IASetVertexBuffers(
      0,
      1,
      &buffer,
      &vertex_stride,
      &vertex_offset
    );
    GetD3D11Context()->IASetIndexBuffer(mesh_d3d.index_buffer_.get(), DXGI_FORMAT_R32_UINT, 0);
  }
}

void Render_DX11::Draw()
{
  auto& mesh = meshes_[bound_mesh];
  if (mesh.index_count_)
    GetD3D11Context()->DrawIndexed(mesh.index_count_, 0, 0);
  else
    GetD3D11Context()->Draw(mesh.vertex_count_, 0);
}

Render_DX11::FramebufferID Render_DX11::CreateFramebuffer(int width, int height, int target_count)
{
  framebuffers_.emplace_back(Framebuffer{ *this, width, height, target_count });
  return framebuffers_.size() - 1;
}

int Render_DX11::GetFramebufferTargetCount(FramebufferID framebuffer)
{
  if (framebuffer == Default)
  {
    return 1;
  }
  return framebuffers_[framebuffer].GetTargetCount();
}

void Render_DX11::RenderToFramebuffer(FramebufferID framebuffer)
{
  if (framebuffer == Default)
  {
    auto* rtv = render_target_view_.get();
    GetD3D11Context()->OMSetRenderTargets(1, &rtv, depth_stencil_view_.get());
    GetD3D11Context()->RSSetViewports(1, &(viewport_));
    return;
  }
  Framebuffer& fb = framebuffers_[framebuffer];
  GetD3D11Context()->OMSetRenderTargets(fb.target_count_, fb.render_target_views_.data(), fb.depth_stencil_view_.get());
  GetD3D11Context()->RSSetViewports(1, &(fb.viewport_));
}

void Render_DX11::ClearFramebuffer(FramebufferID framebuffer)
{
  if (framebuffer == Default)
  {
    return ClearDefaultFramebuffer();
  }
  Framebuffer& fb = framebuffers_[framebuffer];
  GetD3D11Context()->RSSetViewports(1, &(fb.viewport_));
  for (int i = 0; i < fb.GetTargetCount() - 1; ++i)
  {
    GetD3D11Context()->ClearRenderTargetView(fb.render_target_views_[i], &(clear_color_.x));
  }
  GetD3D11Context()->ClearDepthStencilView(
    fb.depth_stencil_view_.get(),
    D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
    1.0f,
    0
  );
}

void Render_DX11::UseFramebufferTexture(FramebufferID framebuffer, int target, int slot)
{
  Framebuffer& fb = framebuffers_[framebuffer];
  auto* resource_view = fb.resource_views_[target];
  GetD3D11Context()->VSSetShaderResources(slot, 1, &resource_view);
  GetD3D11Context()->PSSetShaderResources(slot, 1, &resource_view);
}

void Render_DX11::SetRenderMode(RenderMode render_mode)
{
  if (render_mode_ != render_mode)
  {
    render_mode_ = render_mode;
    switch (render_mode_)
    {
    case RenderMode::FILL:
      device_context_->RSSetState(rasterizer_state_.get());
      break;
    case RenderMode::WIREFRAME:
      device_context_->RSSetState(rasterizer_state_wireframe_.get());
      break;
    }
  }
}

ID3D11Device* Render_DX11::GetD3D11Device() const
{
  return device_.get();
}

ID3D11DeviceContext* Render_DX11::GetD3D11Context() const
{
  return device_context_.get();
}

ID3D11ShaderResourceView* Render_DX11::DebugGetTexture(TextureID texture)
{
  return GetTexture(textures_[texture])->resource_view_.get();
}

ID3D11ShaderResourceView* Render_DX11::DebugGetFramebufferTexture(FramebufferID framebuffer, int target)
{
  return framebuffers_[framebuffer].resource_views_[target];
}

Texture* Render_DX11::GetTexture(InternalTexture const& internal_texture)
{
  switch (internal_texture.type)
  {
  case InternalTexture::Type::ImageTexture:
    return image_textures_.data() + internal_texture.index;
  case InternalTexture::Type::RenderTexture:
    return render_textures_.data() + internal_texture.index;
  case InternalTexture::Type::FramebufferTexture:
    return TextureFromFramebuffer(internal_texture.encapsulating_resource_index, internal_texture.resource_index);
  }
  return nullptr;
}

void Render_DX11::SetupViewport(OS_Win32 const& os)
{
  //set up viewport
  viewport_.Width = os.GetWidth();
  viewport_.Height = os.GetHeight();
  viewport_.TopLeftX = 0;
  viewport_.TopLeftY = 0;
  viewport_.MinDepth = 0;
  viewport_.MaxDepth = 1;
  device_context_->RSSetViewports(1, &viewport_);
}

void Render_DX11::CleanupRenderTarget()
{
  if (render_target_view_)
  {
    device_context_->OMSetRenderTargets(0, 0, 0);
    render_target_view_.attach(nullptr);
  }
}

void Render_DX11::DeleteDepthBuffer()
{
  depth_stencil_view_.attach(nullptr);
  depth_stencil_buffer_.attach(nullptr);
  depth_stencil_state_.attach(nullptr);
}

void Render_DX11::CreateDepthBuffer(OS_Win32 const& os)
{
  //set up Depth/Stencil buffer
  D3D11_TEXTURE2D_DESC depth_stencil_buffer_desc;
  depth_stencil_buffer_desc.Width = os.GetWidth();
  depth_stencil_buffer_desc.Height = os.GetHeight();
  depth_stencil_buffer_desc.MipLevels = 1;
  depth_stencil_buffer_desc.ArraySize = 1;
  depth_stencil_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_stencil_buffer_desc.SampleDesc.Count = 1;
  depth_stencil_buffer_desc.SampleDesc.Quality = 0;
  depth_stencil_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  depth_stencil_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depth_stencil_buffer_desc.CPUAccessFlags = 0;
  depth_stencil_buffer_desc.MiscFlags = 0;

  D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;

  // Depth test parameters
  depth_stencil_desc.DepthEnable = true;
  depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;

  // Stencil test parameters
  depth_stencil_desc.StencilEnable = true;
  depth_stencil_desc.StencilReadMask = 255;
  depth_stencil_desc.StencilWriteMask = 255;

  // Stencil operations if pixel is front-facing
  depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  // Stencil operations if pixel is back-facing
  depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  HRESULT hr;

  // Create depth stencil state
  hr = device_->CreateDepthStencilState(&depth_stencil_desc, depth_stencil_state_.put());
  assert(SUCCEEDED(hr));

  hr = device_->CreateTexture2D(&depth_stencil_buffer_desc, nullptr, depth_stencil_buffer_.put());
  assert(SUCCEEDED(hr));

  hr = device_->CreateDepthStencilView(depth_stencil_buffer_.get(), nullptr, depth_stencil_view_.put());
  assert(SUCCEEDED(hr));

  device_context_->OMSetDepthStencilState(depth_stencil_state_.get(), 0);
}

void Render_DX11::CreateRenderTarget()
{
  HRESULT hr;
  hr = swap_chain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, 0);
  assert(SUCCEEDED(hr));
  //get backbuffer from swap chain
  winrt::com_ptr<ID3D11Texture2D> back_buffer;
  hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), back_buffer.put_void());
  assert(SUCCEEDED(hr));

  hr = device_->CreateRenderTargetView(back_buffer.get(), nullptr, render_target_view_.put());
  assert(SUCCEEDED(hr));
}

Texture* Render_DX11::TextureFromFramebuffer(FramebufferID framebuffer, int target)
{
  temp_texture_storage_.emplace_back(Texture{ framebuffers_[framebuffer].textures_[target], framebuffers_[framebuffer].resource_views_[target] });
  return temp_texture_storage_.data() + temp_texture_storage_.size() - 1;
}

}
