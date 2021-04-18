/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: render_dx11.h
Language: C++
Platform: Windows 8.1+, MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS350_1
Author: Zach Rammell, zach.rammell
Creation date: 10/2/20
End Header --------------------------------------------------------*/
#pragma once

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#define UNICODE
#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "framebuffer.h"

namespace CS350
{
class OS_Win32;

class Render_DX11
{
public:
  using ShaderID = uint32_t;
  using TextureID = uint32_t;
  using MeshID = uint32_t;
  using FramebufferID = uint32_t;
  using SamplerID = uint32_t;
  using UniformID = uint32_t;

  enum ID_Constants : uint32_t
  {
    None = uint32_t(-1),
    Default = uint32_t(-2)
  };

  enum class RenderMode
  {
    FILL,
    WIREFRAME
  };
public:
  Render_DX11(OS_Win32& os);
  ~Render_DX11();

  void SetClearColor(float3 c);
  void SetClearColor(float4 c);

  void Present();
  void ResizeFramebuffer(OS_Win32 const& os);

  void EnableDepthTest(bool enabled);

  SamplerID CreateSampler(/* params??? */);
  void UseSampler(SamplerID sampler, int slot);

  ShaderID CreateShader(LPCWSTR shader_path, int input_layout, bool GS = false);
  void UseShader(ShaderID shader);

  template<typename T, typename = std::enable_if_t<std::is_pod_v<T>>>
  UniformID CreateUniform(T const& data)
  {
    auto size = uniform_byte_storage_.size();
    uniform_byte_storage_.resize(size + sizeof(T));
    auto* data_placed = uniform_byte_storage_.data() + size;
    std::memcpy(data_placed, &data, sizeof(T));

    D3D11_BUFFER_DESC buffer_desc {};
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.ByteWidth = sizeof(T);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA subresource_data {};
    subresource_data.pSysMem = data_placed;
     
    constant_buffers_.push_back(nullptr);
    ID3D11Buffer** back = constant_buffers_.data() + (constant_buffers_.size() - 1);
    HRESULT hr = GetD3D11Device()->CreateBuffer(&buffer_desc, &subresource_data, back);
    assert(SUCCEEDED(hr));

    return UniformID(constant_buffers_.size() - 1);
  }

  template<typename T, typename = std::enable_if_t<std::is_pod_v<T>>>
  void UpdateUniform(UniformID uniform, T const& data)
  {
    T* data_placed = reinterpret_cast<T*>(uniform_byte_storage_.data() + uniform);
    *data_placed = data;

    GetD3D11Context()->UpdateSubresource(constant_buffers_[uniform], 0, nullptr, data_placed, 0, 0);
  }

  void UseUniform(UniformID uniform, int slot);

  TextureID CreateTexture(Image const& image);
  void UseTexture(TextureID texture, int slot);
  void UnuseTexture(int slot);

  TextureID CreateRenderTexture(int width, int height);
  void RenderTo(TextureID render_texture);
  void ClearRenderTexture(TextureID render_texture);

  MeshID CreateMesh(Model const& m);
  MeshID CreateMesh(std::vector<Model::Vertex> const& vertex_buffer);
  void UseMesh(MeshID mesh);
  void DestroyMesh(MeshID mesh);

  FramebufferID CreateFramebuffer(int width, int height, int target_count);
  int GetFramebufferTargetCount(FramebufferID framebuffer);
  void RenderToFramebuffer(FramebufferID framebuffer);
  void ClearFramebuffer(FramebufferID framebuffer);
  void UseFramebufferTexture(FramebufferID framebuffer, int target, int slot);
  void UseFramebufferDepth(FramebufferID framebuffer);

  void SetRenderMode(RenderMode render_mode);

  void Draw();

  /* stupid temporary hack section of the API */

  ID3D11Device* GetD3D11Device() const;
  ID3D11DeviceContext* GetD3D11Context() const;

  ID3D11ShaderResourceView* DebugGetTexture(TextureID texture);
  ID3D11ShaderResourceView* DebugGetFramebufferTexture(FramebufferID framebuffer, int target);

private: /* ==== Raw DirectX resources ==== */
  winrt::com_ptr<IDXGISwapChain> swap_chain_;
  winrt::com_ptr<ID3D11Device> device_;
  winrt::com_ptr<ID3D11DeviceContext> device_context_;
  winrt::com_ptr<ID3D11RenderTargetView> render_target_view_;
  winrt::com_ptr<ID3D11DepthStencilView> depth_stencil_view_;

  winrt::com_ptr<ID3D11Texture2D> depth_stencil_buffer_;

  winrt::com_ptr<ID3D11DepthStencilState> depth_stencil_state_;
  winrt::com_ptr<ID3D11DepthStencilState> depth_stencil_state_no_test_;
  winrt::com_ptr<ID3D11RasterizerState> rasterizer_state_;
  winrt::com_ptr<ID3D11RasterizerState> rasterizer_state_wireframe_;
  winrt::com_ptr<ID3D11SamplerState> sampler_state_;
  std::vector<ID3D11Buffer*> constant_buffers_;

  D3D11_VIEWPORT viewport_;
private: /* ==== Encapsulated DirectX resources ==== */
  std::vector<Shader>   shaders_;
  std::vector<Mesh> meshes_;
  struct InternalTexture
  {
    union
    {
      uint32_t index;
      struct
      {
        uint16_t encapsulating_resource_index;
        uint16_t resource_index;
      };
    };
    enum class Type
    {
      ImageTexture,
      RenderTexture,
      FramebufferTexture,
    } type;
  };
  Texture* GetTexture(InternalTexture const& internal_texture);
  // stores indexes into typed texture-owning containers
  std::vector<InternalTexture> textures_;
  // stores the Texture‍s that came from Image‍s
  std::vector<Texture> image_textures_;
  // stores the RenderTexture‍s
  std::vector<RenderTexture> render_textures_;
  std::vector<Framebuffer> framebuffers_;
  std::vector<Texture> temp_texture_storage_;

  std::vector<uint8_t> uniform_byte_storage_;

  DirectX::XMFLOAT4 clear_color_;

  MeshID bound_mesh = None;

  RenderMode render_mode_ = RenderMode::FILL;

  /* internal helpers */

  void SetupViewport(OS_Win32 const& os);
  void CleanupRenderTarget();
  void DeleteDepthBuffer();
  void CreateDepthBuffer(OS_Win32 const& os);
  void CreateRenderTarget(OS_Win32 const& os);

  Texture* TextureFromFramebuffer(FramebufferID framebuffer, int target);
  void ClearDefaultFramebuffer();
};

}
