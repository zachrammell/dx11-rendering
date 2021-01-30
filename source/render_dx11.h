/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: render_dx11.h
Language: C++
Platform: Windows 8.1+, MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS300_1
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
  using CubemapID = uint32_t;
  using FramebufferID = uint32_t;
  using SamplerID = uint32_t;
  enum ID_Constants : uint32_t
  {
    None = uint32_t(-1),
    Default = uint32_t(-2)
  };
public:
  Render_DX11(OS_Win32& os);
  ~Render_DX11();

  void ClearDefaultFramebuffer();
  void SetClearColor(DirectX::XMFLOAT3 c);

  void Present();
  void ResizeFramebuffer(OS_Win32 const& os);

  SamplerID CreateSampler(/* params??? */);
  void UseSampler(SamplerID sampler, int slot);

  ShaderID CreateShader(LPCWSTR shader_path, int input_layout, bool GS = false);
  void UseShader(ShaderID shader);

  TextureID CreateTexture(Image const& image);
  void UseTexture(TextureID texture, int slot);

  TextureID CreateRenderTexture(int width, int height);
  void RenderTo(TextureID render_texture);

  MeshID CreateMesh(Mesh const& m);
  MeshID CreateMesh(std::vector<Mesh::Vertex> const& vertex_buffer);
  void UseMesh(MeshID mesh);

  FramebufferID CreateFramebuffer(int width, int height, int target_count);
  int GetFramebufferTargetCount(FramebufferID framebuffer);
  void RenderToFramebuffer(FramebufferID framebuffer);
  void ClearFramebuffer(FramebufferID framebuffer);
  void UseFramebufferTexture(FramebufferID framebuffer, int target, int slot);

  void Draw();

  ID3D11Device* GetD3D11Device() const;
  ID3D11DeviceContext* GetD3D11Context() const;

  /* stupid temporary hack section of the API */

  ID3D11ShaderResourceView* GetFramebufferTexture(FramebufferID framebuffer, int target);

private: /* ==== Raw DirectX resources ==== */
  winrt::com_ptr<IDXGISwapChain> swap_chain_;
  winrt::com_ptr<ID3D11Device> device_;
  winrt::com_ptr<ID3D11DeviceContext> device_context_;
  winrt::com_ptr<ID3D11RenderTargetView> render_target_view_;
  winrt::com_ptr<ID3D11DepthStencilView> depth_stencil_view_;

  winrt::com_ptr<ID3D11Texture2D> depth_stencil_buffer_;

  winrt::com_ptr<ID3D11DepthStencilState> depth_stencil_state_;
  winrt::com_ptr<ID3D11RasterizerState> rasterizer_state_;
  winrt::com_ptr<ID3D11SamplerState> sampler_state_;

  D3D11_VIEWPORT viewport_;
private: /* ==== Encapsulated DirectX resources ==== */
  std::vector<Shader>   shaders_;
  std::vector<Mesh_D3D> meshes_;
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

  DirectX::XMFLOAT4 clear_color_;

  MeshID bound_mesh = None;

  /* internal helpers */

  void SetupViewport(OS_Win32 const& os);
  void CleanupRenderTarget();
  void DeleteDepthBuffer();
  void CreateDepthBuffer(OS_Win32 const& os);
  void CreateRenderTarget();

  Texture* TextureFromFramebuffer(FramebufferID framebuffer, int target);
};

}
