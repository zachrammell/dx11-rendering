/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: mesh.h
Language: C++
Platform: Windows 8.1+, MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS350_1
Author: Zach Rammell, zach.rammell
Creation date: 10/2/20
End Header --------------------------------------------------------*/
#pragma once

#define NOMINMAX
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <winrt/base.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>

#include "structures/types/types_cpp.h"

namespace CS350
{

class Render_DX11;

struct Model
{
  struct Vertex
  {
    float3 position = {};
    float3 normal = {};
    float2 tex_coord = {};

    Vertex interp(Vertex const& v, float t) const
    {
      Vertex nv;
      nv.position = lerp(position, v.position, t);
      nv.normal = lerp(normal, v.normal, t);
      nv.tex_coord = lerp(tex_coord, v.tex_coord, t);
      return nv;
    }
  };
  using Index = uint32_t;

  std::vector<Vertex> vertex_buffer;
  std::vector<Index>  index_buffer;
  std::vector<Vertex> face_center_vertex_buffer;

  /* Mesh Creation Functions */

  static Model Load(char const* filepath);
  static Model GenerateSphere(int sectorCount, int stackCount);
  static Model GenerateCircle(int segments);
  static Model GenerateUnitLineCube();

private:
  static inline struct PropertyStore
  {
    PropertyStore();
    ~PropertyStore();
    aiPropertyStore* store_;
  } property_store_;
  static PropertyStore const& GetPropertyStore();

  void ProcessNode(aiScene const* scene, aiNode const* node);
  void ProcessMesh(aiScene const* scene, aiMesh const* mesh);
};

class Mesh
{
public:
  void Reload(Model const& m, Render_DX11& render);
private:
  friend class Render_DX11;
  Mesh(Render_DX11& render, Model const& m);
  Mesh(Render_DX11& render, std::vector<Model::Vertex> const& vertex_buffer);
  winrt::com_ptr<ID3D11Buffer> vertex_buffer_ = nullptr;
  winrt::com_ptr<ID3D11Buffer> index_buffer_ = nullptr;
  UINT vertex_count_, index_count_;
};

}
