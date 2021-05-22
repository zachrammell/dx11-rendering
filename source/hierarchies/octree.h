#pragma once

#include <array>
#include <stack>
#include <vector>

#include "../math_helper.h"
#include "../mesh.h"
#include "../geometry/box.h"
#include "../geometry/triangle.h"
#include "../render_dx11.h"
#include "../structures/per_object.h"

namespace CS350
{
class Octree;

class OctreeNode
{
public:
  OctreeNode(Model const& mdl, int max_triangles, Box<3>* box = nullptr)
    : model{ mdl }
  {
    if (box)
    {
      bounds = *box;
    }
    else
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

    // if this node contains below the maximum amount of triangles, no split required
    if (model.index_buffer.size() / 3 < max_triangles)
      return;

    Box<3> new_bounds[8];
    Model new_models[8];
    for (int si = 0; si < 8; ++si)
    {
      Vec<3> offset = ((bounds.mx - bounds.mn) / 2.0f) *
        float3{ (float)(si & 1), (float)((si >> 1) & 1), (float)((si >> 2) & 1) };
      new_bounds[si] = { bounds.mn + offset, avg(bounds.mn, bounds.mx) + offset };

      for (int i = 0; i < model.index_buffer.size(); i += 3)
      {
        int idx[3];
        for (int j = 0; j < 3; ++j)
          idx[j] = model.index_buffer[i + j];

        if (new_bounds[si].contains(model.vertex_buffer[idx[0]].position)
            || new_bounds[si].contains(model.vertex_buffer[idx[1]].position)
            || new_bounds[si].contains(model.vertex_buffer[idx[2]].position))
        {
          for (int j = 0; j < 3; ++j)
            new_models[si].index_buffer.push_back(idx[j]);
        }
      }
    }

    for (Model& new_model : new_models)
    {
      std::map<unsigned, unsigned> idx_map;
      auto& idx_buf = new_model.index_buffer;
      for (auto i = idx_buf.begin(); i != idx_buf.end(); ++i)
      {
        new_model.vertex_buffer.push_back(model.vertex_buffer[*i]);
        auto mapped = idx_map.find(*i);
        if (mapped != idx_map.end())
        {
          *i = mapped->second;
        }
        else
        {
          unsigned new_idx = (new_model.vertex_buffer.size() - 1);
          idx_map.insert(std::make_pair(*i, new_idx));
          *i = new_idx;
        }
      }
    }

    model.vertex_buffer.clear();
    model.index_buffer.clear();

    for (int i = 0; i < 8; ++i)
    {
      children[i] = new OctreeNode(new_models[i], max_triangles, &new_bounds[i]);
    }
  }
  Box<3> const& GetBounds()
  {
    return bounds;
  }
  bool IsLeaf()
  {
    return !model.vertex_buffer.empty();
  }
  Model const& GetModel()
  {
    return model;
  }

  void Hit()
  {
    hit = true;
  }

  void Reset()
  {
    hit = false;
  }

  float4 color;

private:
  Box<3> bounds;
  Model model;
  std::array<OctreeNode*, 8> children{};
  bool hit;

  void DrawBounds(Render_DX11& render, Render_DX11::UniformID per_object_id)
  {
    for (OctreeNode* child : children)
    {
      if (child)
      {
        child->DrawBounds(render, per_object_id);
      }
    }

    if (model.index_buffer.empty()) return;

    dx::XMMATRIX world_matrix = dx::XMMatrixIdentity();
    float3 extents = (bounds.mx - bounds.mn) / 2.0f;
    world_matrix *= dx::XMMatrixScaling(extents.x, extents.y, extents.z);
    float3 center = avg(bounds.mn, bounds.mx);
    world_matrix *= dx::XMMatrixTranslation(center.x, center.y, center.z);

    per_object per_object_data;
    dx::XMStoreFloat4x4(&(per_object_data.World), world_matrix);
    per_object_data.Color = color;
    render.UpdateUniform(per_object_id, per_object_data);

    render.Draw();
  }

  friend Octree;
};

class Octree
{
public:
  Octree(Model const& model, int max_triangles = 4096)
    : head_{ new OctreeNode{model, max_triangles} }
  {
    Iterate([](OctreeNode* node, int depth)
    {
      dx::XMStoreFloat4(&node->color,
                        dx::XMColorHSVToRGB({
                          fmod(depth * 11.25f, 360.0f) / 360.0f,
                            1.0f, 1.0f }));
    });
  }

  ~Octree()
  {
    Iterate([](OctreeNode* node, int)
    {
      delete node;
    });
  }

  // non recursive DFS tree iteration
  template<typename F>
  void Iterate(F&& f)
  {
    int depth = 0;
    std::stack<std::pair<int, OctreeNode*>> stk;
    stk.push(std::make_pair(depth, head_));

    while (!stk.empty())
    {
      auto top = stk.top();
      stk.pop();
      bool any_children = false;
      for (OctreeNode* child : top.second->children)
      {
        if (child)
        {
          stk.push(std::make_pair(depth, child));
          any_children = true;
        }
      }

      f(top.second, top.first);

      if (any_children) ++depth;
    }
  }

  void Upload(Render_DX11& render)
  {
    Iterate([this, &render](OctreeNode* node, int)
    {
      if (node->model.index_buffer.empty())
        return;

      ColoredMesh mesh{ node->color, render.CreateMesh(node->model) };
      meshes_.emplace_back(mesh);
    });
  }

  void ReleaseMeshes(Render_DX11& render)
  {
    for (ColoredMesh const& mesh : meshes_)
    {
      render.DestroyMesh(mesh.mesh);
    }
  }

  void DrawMesh(Render_DX11& render, Render_DX11::UniformID per_object_id, bool color_chunks = false)
  {

    Iterate([](OctreeNode* node, int depth)
    {
      if (!node->hit)
      {
        dx::XMStoreFloat4(&node->color,
                          dx::XMColorHSVToRGB({
                            fmod(depth * 11.25f, 360.0f) / 360.0f,
                              1.0f, 1.0f }));
      }
    });

    for (ColoredMesh const& mesh : meshes_)
    {
      if (color_chunks)
      {
        per_object per_object_data;
        dx::XMStoreFloat4x4(&(per_object_data.World), world_matrix);
        dx::XMStoreFloat4x4(&(per_object_data.WorldNormal), InverseTranspose(world_matrix));
        per_object_data.Color = mesh.color;
        render.UpdateUniform(per_object_id, per_object_data);
      }
      render.UseMesh(mesh.mesh);
      render.Draw();
    }
  }

  void DrawBounds(Render_DX11& render, Render_DX11::UniformID per_object_id)
  {
    if (unit_line_cube_mesh == -1)
    {
      unit_line_cube_mesh = render.CreateMesh(unit_line_cube_model);
    }
    render.GetD3D11Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    render.UseMesh(unit_line_cube_mesh);
    head_->DrawBounds(render, per_object_id);
  }

private:
  OctreeNode* head_;

  struct ColoredMesh
  {
    float4& color;
    Render_DX11::MeshID mesh;
  };
  std::vector<ColoredMesh> meshes_;
};

}
