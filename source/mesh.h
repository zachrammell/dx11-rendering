/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: mesh.h
Language: C++
Platform: Windows 8.1+, MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS300_1
Author: Zach Rammell, zach.rammell
Creation date: 10/2/20
End Header --------------------------------------------------------*/
#pragma once

#define NOMINMAX
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <winrt/base.h>

namespace CS350
{

class Render_DX11;

struct Mesh
{
  struct Vertex
  {
    DirectX::XMFLOAT3 position = {};
    DirectX::XMFLOAT3 normal = {};
    DirectX::XMFLOAT2 tex_coord = {};
  };
  using Index = uint32_t;

  std::vector<Vertex> vertex_buffer;
  std::vector<Index>  index_buffer;
  std::vector<Vertex> face_center_vertex_buffer;
};

class Mesh_D3D
{
public:
  void Reload(Mesh const& m, Render_DX11& render);
private:
  friend class Render_DX11;
  Mesh_D3D(Render_DX11& render, Mesh const& m);
  Mesh_D3D(Render_DX11& render, std::vector<Mesh::Vertex> const& vertex_buffer);
  winrt::com_ptr<ID3D11Buffer> vertex_buffer_ = nullptr;
  winrt::com_ptr<ID3D11Buffer> index_buffer_ = nullptr;
  UINT vertex_count_, index_count_;
};

Mesh GenerateSphereMesh(int sectorCount, int stackCount);

Mesh GenerateCircleMesh(int segments);

}
