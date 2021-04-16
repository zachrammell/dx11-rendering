#pragma once

#include <array>
#include <vector>

#include "../mesh.h"
#include "../geometry/box.h"
#include "../render_dx11.h"

namespace CS350
{

class OctreeNode
{
public:
  OctreeNode(Model const& model)
  {
    Vec<3>
      mn{ float3::one() * std::numeric_limits<float>::max() },
      mx{ float3::one() * std::numeric_limits<float>::min() };
    for (size_t i = 0; i < model.vertex_buffer.size(); ++i)
    {
      float3 pos = model.vertex_buffer[i].position;
      mn = min(mn, pos);
      mx = max(mx, pos);
    }
    bounds = Box<3>{ mn, mx };
  }
private:
  Box<3> bounds;
  Model model;
  std::array<OctreeNode*, 8> children {};

  void DrawBounds(Render_DX11& render)
  {
    for (OctreeNode* child : children)
    {
      if (child)
      {
        child->DrawBounds(render);
      }
    }


  }

  friend class Octree;
};

class Octree
{
public:
  Octree(Model const& model)
    : head{ new OctreeNode{model} }
  {

  }
  void DrawBounds(Render_DX11& render)
  {
    render.GetD3D11Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    head->DrawBounds(render);
  }
private:
  OctreeNode* head;
};

}
