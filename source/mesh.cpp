/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: mesh.cpp
Language: C++
Platform: Windows 8.1+, MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS300_1
Author: Zach Rammell, zach.rammell
Creation date: 10/2/20
End Header --------------------------------------------------------*/
#include "mesh.h"

#include "render_dx11.h"

namespace CS350
{
Mesh_D3D::Mesh_D3D(Render_DX11& render, Mesh const& m)
  : vertex_count_(m.vertex_buffer.size()),
  index_count_(m.index_buffer.size())
{
  HRESULT hr;
  {
    D3D11_BUFFER_DESC vertex_buffer_descriptor
    {
      UINT(sizeof(Mesh::Vertex) * vertex_count_),
      D3D11_USAGE_DYNAMIC,
      D3D11_BIND_VERTEX_BUFFER,
      D3D11_CPU_ACCESS_WRITE, 0, 0
    };
    D3D11_SUBRESOURCE_DATA subresource_data{ m.vertex_buffer.data(), 0, 0 };

    hr = render.GetD3D11Device()->CreateBuffer(&vertex_buffer_descriptor, &subresource_data, vertex_buffer_.put());
    assert(SUCCEEDED(hr));
  }

  {
    D3D11_BUFFER_DESC index_buffer_descriptor
    {
      UINT(sizeof(Mesh::Index) * index_count_),
      D3D11_USAGE_DEFAULT,
      D3D11_BIND_INDEX_BUFFER,
      0, 0, 0
    };
    D3D11_SUBRESOURCE_DATA subresource_data{ m.index_buffer.data(), 0, 0 };

    hr = render.GetD3D11Device()->CreateBuffer(&index_buffer_descriptor, &subresource_data, index_buffer_.put());
    assert(SUCCEEDED(hr));
  }
}

Mesh_D3D::Mesh_D3D(Render_DX11& render, std::vector<Mesh::Vertex> const& vertex_buffer)
  : vertex_count_(vertex_buffer.size()),
  index_count_(0)
{
  D3D11_BUFFER_DESC vertex_buffer_descriptor
  {
    UINT(sizeof(Mesh::Vertex) * vertex_count_),
    D3D11_USAGE_DEFAULT,
    D3D11_BIND_VERTEX_BUFFER,
    0, 0, 0
  };
  D3D11_SUBRESOURCE_DATA subresource_data{ vertex_buffer.data(), 0, 0 };

  HRESULT hr = render.GetD3D11Device()->CreateBuffer(&vertex_buffer_descriptor, &subresource_data, vertex_buffer_.put());
  assert(SUCCEEDED(hr));
}

void Mesh_D3D::Reload(Mesh const& m, Render_DX11& render)
{
  D3D11_MAPPED_SUBRESOURCE resource;
  HRESULT hr;
  hr = render.GetD3D11Context()->Map(vertex_buffer_.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
  if (FAILED(hr)) __debugbreak();
  memcpy(resource.pData, m.vertex_buffer.data(), m.vertex_buffer.size() * sizeof(Mesh::Vertex));
  render.GetD3D11Context()->Unmap(vertex_buffer_.get(), 0);
}

Mesh GenerateSphereMesh(int sectorCount, int stackCount)
{
  using namespace DirectX;
  Mesh sphere_mesh;

  float x, y, z, xy;                              // vertex position
  float nx, ny, nz;                               // vertex normal

  float radius = 1.0f;

  float sectorStep = XM_2PI / static_cast<float>(sectorCount);
  float stackStep = XM_PI / static_cast<float>(stackCount);
  float sectorAngle, stackAngle;

  sphere_mesh.vertex_buffer.reserve(stackCount * (sectorCount + 1));

  for (int i = 0; i <= stackCount; ++i)
  {
    stackAngle = XM_PIDIV2 - i * stackStep;     // starting from pi/2 to -pi/2
    xy = radius * cosf(stackAngle);             // r * cos(u)
    z = radius * sinf(stackAngle);              // r * sin(u)

    // add (sectorCount+1) vertices per stack
    for (int j = 0; j <= sectorCount; ++j)
    {
      Mesh::Vertex v;
      sectorAngle = j * sectorStep;           // starting from 0 to 2pi

      // vertex position (x, y, z)
      x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
      y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
      v.position = { x, y, z };

      // normalized vertex normal (nx, ny, nz)
      nx = -x;
      ny = -y;
      nz = -z;
      v.normal = { nx, ny, nz };

      sphere_mesh.vertex_buffer.push_back(v);
    }
  }

  int face_count = 2 * (sectorCount - 1);
  sphere_mesh.index_buffer.reserve(3 * face_count);
  sphere_mesh.face_center_vertex_buffer.reserve(face_count);

  int k1, k2;
  for (int i = 0; i < stackCount; ++i)
  {
    k1 = i * (sectorCount + 1);     // beginning of current stack
    k2 = k1 + sectorCount + 1;      // beginning of next stack

    for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
    {
      // 2 triangles per sector excluding first and last stacks
      // k1 => k2 => k1+1
      if (i != 0)
      {
        XMVECTOR face_center{ 0, 0, 0 };
        XMVECTOR face_normal{ 0, 0, 0 };
        int indices[3] = { k1, k2, k1 + 1 };
        for (int k = 0; k < 3; ++k)
        {
          sphere_mesh.index_buffer.push_back(indices[k]);
          face_center += XMLoadFloat3(&(sphere_mesh.vertex_buffer[indices[k]].position));
          face_normal += XMLoadFloat3(&(sphere_mesh.vertex_buffer[indices[k]].normal));
        }
        face_center /= 3;
        XMVector3Normalize(face_normal);

        Mesh::Vertex face_vertex;
        XMStoreFloat3(&(face_vertex.position), face_center);
        XMStoreFloat3(&(face_vertex.normal), face_normal);

        sphere_mesh.face_center_vertex_buffer.push_back(face_vertex);
      }

      // k1+1 => k2 => k2+1
      if (i != (stackCount - 1))
      {
        sphere_mesh.index_buffer.push_back(k1 + 1);
        sphere_mesh.index_buffer.push_back(k2);
        sphere_mesh.index_buffer.push_back(k2 + 1);
      }
    }
  }

  return sphere_mesh;
}

Mesh GenerateCircleMesh(int segments)
{
  using namespace DirectX;
  Mesh circle_mesh;

  circle_mesh.vertex_buffer.reserve(segments + 1);

  for (int i = 0; i < segments + 1; ++i)
  {
    Mesh::Vertex v{};
    v.position = { sin(static_cast<float>(i) / segments * XM_2PI), 0,  cos(static_cast<float>(i) / segments * XM_2PI) };
    circle_mesh.vertex_buffer.push_back(v);
  }

  return circle_mesh;
}

}
