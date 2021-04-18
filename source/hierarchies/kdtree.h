#pragma once

#include <array>
#include <stack>
#include <vector>

#include "../globals.h"
#include "../math_helper.h"
#include "../mesh.h"
#include "../geometry/box.h"
#include "../render_dx11.h"
#include "../geometry/plane.h"
#include "../structures/per_object.h"

namespace CS350
{
class KdTree;

class KdNode
{
public:
  KdNode(Model const& mdl, int max_triangles, Vec<3> split_dir, Box<3>* box = nullptr)
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

    Box<3> new_bounds[2];
    Model new_models[2];
    for (int si = 0; si < 2; ++si)
    {
      // TODO: heuristic for factor
      float factor = 0.5f;// (rand() / (float)RAND_MAX) * 0.5f + 0.33f;
      Vec<3> offset = factor * split_dir * (bounds.mx - bounds.mn);
      new_bounds[si] = { bounds.mn + !(si & 1) * offset, bounds.mx - (si & 1) * offset };

      size_t const idx_size = model.index_buffer.size();
      for (int i = 0; i < idx_size; i += 3)
      {
        int idx[3];
        bool contained[3];
        Triangle<3> tri;
        for (int j = 0; j < 3; ++j)
        {
          idx[j] = model.index_buffer[i + j];
          tri[j] = model.vertex_buffer[idx[j]].position;
          contained[j] = new_bounds[si].contains(tri[j]);
        }

        int num_contained = contained[0] + contained[1] + contained[2];
        // at least one point belongs in this new bound
        if (num_contained > 0)
        {
          // the triangle is split across bounds
          if (num_contained < 3)
          {
            Vec<3> point_on_plane = bounds.mx - offset;
            Plane<3> split_plane{ split_dir, dot(split_dir, -point_on_plane) };

            // add the vertices that are not contained to the main vertex buffer
            // add the whole triangle including the indices of any vertices not contained

            for (int j = 0; j < 3; ++j)
            {
              if (contained[j])
              {
                auto j0 = (j + 1) % 3;
                auto j1 = (j + 2) % 3;

                auto split_pt0 = split_plane.intersection(tri[j], tri[j0]);
                auto split_pt1 = split_plane.intersection(tri[j], tri[j1]);

                if (!(split_pt0 || split_pt1))
                {
                  continue;
                }

                auto& vbuf = model.vertex_buffer;
                Model::Vertex const v = vbuf[idx[j]];
                Model::Vertex const v0 = vbuf[idx[j0]];
                Model::Vertex const v1 = vbuf[idx[j1]];

                new_models[si].index_buffer.push_back(idx[j]);
                if (split_pt0)
                {
                  vbuf.push_back(v.interp(v0, split_pt0.value().first));
                  new_models[si].index_buffer.push_back(vbuf.size() - 1);
                }
                else
                {
                  vbuf.push_back(v0);
                  new_models[si].index_buffer.push_back(idx[j0]);
                }

                if (split_pt1)
                {
                  vbuf.push_back(v.interp(v1, split_pt1.value().first));
                  new_models[si].index_buffer.push_back(vbuf.size() - 1);
                }
                else
                {
                  vbuf.push_back(v1);
                  new_models[si].index_buffer.push_back(idx[j1]);
                }
              }
            }

            continue;
          }
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

    // cycle elements of the split dir
    std::swap(split_dir.y, split_dir.z);
    std::swap(split_dir.x, split_dir.y);

    for (int i = 0; i < 2; ++i)
    {
      children[i] = new KdNode(new_models[i], max_triangles, split_dir, &new_bounds[i]);
    }
  }
private:
  Box<3> bounds;
  Model model;
  float4 color;
  std::array<KdNode*, 2> children{};

  void DrawBounds(Render_DX11& render, Render_DX11::UniformID per_object_id)
  {
    for (KdNode* child : children)
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

  friend KdTree;
};

class KdTree
{
public:
  KdTree(Model const& model, int max_triangles = 4096)
    : head_{ new KdNode{model, max_triangles, float3{1.0f, 0.0f, 0.0f}} }
  {
    Iterate([](KdNode* node, int depth)
    {
      node->color = float4{ (float)(depth & 1), (float)((depth >> 1) & 1), (float)((depth >> 2) & 1), 1.0f };
    });
  }

  ~KdTree()
  {
    Iterate([](KdNode* node, int)
    {
      delete node;
    });
  }

  // non recursive DFS tree iteration
  template<typename F>
  void Iterate(F const& f)
  {
    int depth = 0;
    std::stack<std::pair<int, KdNode*>> stk;
    stk.push(std::make_pair(depth, head_));

    while (!stk.empty())
    {
      auto top = stk.top();
      stk.pop();
      bool any_children = false;
      for (KdNode* child : top.second->children)
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
    Iterate([this, &render](KdNode* node, int)
    {
      if (node->model.index_buffer.empty())
        return;

      ColoredMesh mesh;
      mesh.color = node->color;
      mesh.mesh = render.CreateMesh(node->model);

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
  KdNode* head_;
  struct ColoredMesh
  {
    float4 color;
    Render_DX11::MeshID mesh;
  };
  std::vector<ColoredMesh> meshes_;
};

}
