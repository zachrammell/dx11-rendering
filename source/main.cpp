/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: main.cpp
Purpose: most of the functionality
Language: C++
Platform: MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS350_1
Author: Zach Rammell, zach.rammell
Creation date: 10/2/20
End Header --------------------------------------------------------*/
#include "os_win32.h"
#include "render_dx11.h"
#include "globals.h"

#include <d3dcompiler.h>

#include <fstream>
#include <filesystem>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "structures/per_object.h"
#include "structures/per_frame.h"
#include "hierarchies/octree.h"
#include "hierarchies/kdtree.h"
#include "math_helper.h"

namespace fs = std::filesystem;
namespace dx = DirectX;

using Tree = void;

const char KEY_SHIFT = 16;

namespace
{

dx::XMVECTOR camPosition = dx::XMVectorSet(1.0f, 0.5f, -0.5f, 0.0f);
dx::XMVECTOR camTarget = dx::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
dx::XMVECTOR camUp = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
dx::XMVECTOR DefaultForward = dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
dx::XMVECTOR DefaultRight = dx::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
dx::XMVECTOR camForward = dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
dx::XMVECTOR camRight = dx::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

dx::XMMATRIX cam_view_matrix;
dx::XMMATRIX cam_projection_matrix;
dx::XMMATRIX camRotationMatrix;

float cam_fov = 90.0f;
float cam_yaw = -1.4f, cam_pitch = .25f;
float moveLeftRight = 0.0f;
float moveBackForward = 0.0f;
float moveUpDown = 0.0f;

float camera_movespeed = 2.0f;
float mouse_sensitivity = 0.005f;

}

void CameraInput(CS350::OS_Win32& os, float dt)
{
  auto const mouse_data = os.GetMouseData();

  if (mouse_data.right)
  {
    cam_pitch = std::clamp(cam_pitch + mouse_data.dy * mouse_sensitivity, -dx::XM_PIDIV2 + 0.05f, dx::XM_PIDIV2 - 0.05f);
    cam_yaw -= mouse_data.dx * mouse_sensitivity;
  }

  moveLeftRight = camera_movespeed * dt * (os.GetKeyState('A').pressed - os.GetKeyState('D').pressed);
  moveBackForward = camera_movespeed * dt * (os.GetKeyState('W').pressed - os.GetKeyState('S').pressed);
  moveUpDown = camera_movespeed * dt * (os.GetKeyState(' ').pressed - os.GetKeyState(KEY_SHIFT).pressed);
}

void UpdateCamera()
{
  using namespace DirectX;

  camRotationMatrix = dx::XMMatrixRotationRollPitchYaw(cam_pitch, cam_yaw, 0);
  camTarget = dx::XMVector3TransformCoord(DefaultForward, camRotationMatrix);
  camTarget = dx::XMVector3Normalize(camTarget);

  dx::XMMATRIX RotateYTempMatrix = dx::XMMatrixRotationY(cam_yaw);

  camRight = dx::XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
  camUp = dx::XMVector3TransformCoord(camUp, RotateYTempMatrix);
  camForward = dx::XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

  camPosition += camRight * moveLeftRight;
  camPosition += camForward * moveBackForward;
  camPosition += camUp * moveUpDown;

  moveLeftRight = 0.0f;
  moveBackForward = 0.0f;
  moveUpDown = 0.0f;

  camTarget = camPosition + camTarget;

  cam_view_matrix = dx::XMMatrixLookAtRH(camPosition, camTarget, camUp);
}

struct GameObject
{
  float3 position, velocity, scale;
  float4 color;
  CS350::Render_DX11::MeshID mesh;

  // for iterating octree
  void operator()(CS350::OctreeNode* node, int depth)
  {
    if (node->IsLeaf())
    {
      if (node->GetModel().GetBounds().intersects(GetBounds()))
      {
        Hit();
        node->Hit();
        node->color = color;
      }
    }
  }

  CS350::Box<3> GetBounds()
  {
    return CS350::Box<3>{ position - scale / 2, position + scale / 2 };
  }

  void Update(float dt)
  {
    position += velocity * dt;
  }

  void Draw(CS350::Render_DX11& render, CS350::Render_DX11::UniformID per_object_id)
  {
    render.UseMesh(mesh);
    DrawHelper(render, per_object_id);
  }

  void DrawBounds(CS350::Render_DX11& render, CS350::Render_DX11::UniformID per_object_id)
  {
    render.UseMesh(unit_line_cube_mesh);
    DrawHelper(render, per_object_id);
  }

  void Reset(float3 position, float3 velocity)
  {
    color = { 1, 1, 1, 1 };
    this->position = position;
    this->velocity = velocity;
  }

  void Hit()
  {
    color = { 1, 0, 0, 1 };
    velocity = { 0, 0, 0 };
  }

private:
  void DrawHelper(CS350::Render_DX11& render, CS350::Render_DX11::UniformID per_object_id)
  {
    dx::XMMATRIX world_matrix = dx::XMMatrixIdentity();
    world_matrix *= dx::XMMatrixScaling(scale.x, scale.y, scale.z);
    world_matrix *= dx::XMMatrixTranslation(position.x, position.y, position.z);

    per_object per_object_data;
    dx::XMStoreFloat4x4(&(per_object_data.World), world_matrix);
    dx::XMStoreFloat4x4(&(per_object_data.WorldNormal), CS350::InverseTranspose(world_matrix));
    per_object_data.Color = color;

    render.UpdateUniform(per_object_id, per_object_data);
    render.Draw();
  }
};

int main()
{
  CS350::OS_Win32 os{ TEXT("CS350 Project 3 - Zach Rammell"), 1600, 900 };
  CS350::Render_DX11 render{ os };
  os.Show();

  float3 clear_color{ 0.15f, 0.15f, 0.15f };

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

  io.ConfigViewportsNoTaskBarIcon = true;

  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  ImGui_ImplWin32_Init(os.GetWindowHandle());
  ImGui_ImplDX11_Init(render.GetD3D11Device(), render.GetD3D11Context());

  double current_time = os.GetTime();
  bool show_imgui_demo = false;

  float model_rotation = 0.0f;
  float model_scale = 1.0f;

  bool draw_vertex_normals = false;

  bool cursor_highlight = false;

  enum tree_type_t
  {
    tree_octree,
    tree_kdtree,

    tree_COUNT
  };
  char* tree_type_names[] =
  {
    "Octree",
    "Kd-Tree"
  };
  tree_type_t tree_type = tree_octree;

  struct tree_settings_t
  {
    bool bounds = true;
    bool mesh = true;
    bool mesh_colors = true;
    int node_size = 512;
  } tree_settings;

  int selected_view = -1;

  per_object per_object_data;
  per_frame per_frame_data;
  per_frame_data.AmbientColor = float4(0.031, 0.027, 0.031, 1.0f);

  CS350::Model sphere_model = CS350::Model::GenerateSphere(16, 16);
  CS350::Render_DX11::MeshID sphere_mesh = render.CreateMesh(sphere_model);
  GameObject sphere_gameobject{ {1e38, 1e38, 1e38}, {0, 0.15f, 0}, {0.1f, 0.1f, 0.1f}, {1, 1, 1, 1}, sphere_mesh };

  CS350::Image metal_image = CS350::Image("assets/textures/metal_roof_diff_512x512.png");

  CS350::Model bunny_model = CS350::Model::Load("assets/models/bunny_high_poly.obj");
  //CS350::Render_DX11::MeshID bunny_mesh = render.CreateMesh(bunny_model);

  Tree* bunny_tree = new CS350::Octree(bunny_model, 512);
  ((CS350::Octree*)bunny_tree)->Upload(render);

  CS350::Render_DX11::TextureID metal_texture = render.CreateTexture(metal_image);
  CS350::Render_DX11::FramebufferID framebuffer = render.CreateFramebuffer(os.GetWidth(), os.GetHeight(), 4);
  CS350::Render_DX11::ShaderID deferred_geometry =
    render.CreateShader(TEXT("assets/shaders/deferred_geometry.hlsl"),
                        (CS350::Shader::InputLayout_POS | CS350::Shader::InputLayout_NOR | CS350::Shader::InputLayout_TEX)
    );

  CS350::Render_DX11::UniformID per_object_id = render.CreateUniform(per_object_data);
  CS350::Render_DX11::UniformID per_frame_id = render.CreateUniform(per_frame_data);

  CS350::Render_DX11::TextureID composited_texture = render.CreateRenderTexture(os.GetWidth(), os.GetHeight());

  CS350::Render_DX11::ShaderID deferred_lighting =
    render.CreateShader(TEXT("assets/shaders/deferred_lighting.hlsl"),
                        0
    );

  CS350::Render_DX11::ShaderID fsq =
    render.CreateShader(TEXT("assets/shaders/fullscreen_quad_render.hlsl"),
                        0
    );

  CS350::Render_DX11::ShaderID debug_line =
    render.CreateShader(TEXT("assets/shaders/debug_line.hlsl"),
                        (CS350::Shader::InputLayout_POS | CS350::Shader::InputLayout_NOR),
                        true
    );

  CS350::Render_DX11::ShaderID debug_wireframe =
    render.CreateShader(TEXT("assets/shaders/debug_wireframe.hlsl"),
                        (CS350::Shader::InputLayout_POS | CS350::Shader::InputLayout_NOR | CS350::Shader::InputLayout_TEX)
    );

  auto regenerate_tree = [&](tree_type_t new_tree_type)
  {
    if (tree_type == tree_octree)
    {
      ((CS350::Octree*)bunny_tree)->ReleaseMeshes(render);
      delete ((CS350::Octree*)bunny_tree);
    }
    else if (tree_type == tree_kdtree)
    {
      ((CS350::KdTree*)bunny_tree)->ReleaseMeshes(render);
      delete ((CS350::KdTree*)bunny_tree);
    }

    if (new_tree_type == tree_octree)
    {
      bunny_tree = new CS350::Octree(bunny_model, tree_settings.node_size);
      ((CS350::Octree*)bunny_tree)->Upload(render);
    }
    else if (new_tree_type == tree_kdtree)
    {
      bunny_tree = new CS350::KdTree(bunny_model, tree_settings.node_size);
      ((CS350::KdTree*)bunny_tree)->Upload(render);
    }
  };

  while (!os.ShouldClose())
  {
    float dt;
    {
      double previous_time = current_time;
      current_time = os.GetTime();
      dt = static_cast<float>(current_time - previous_time);
      if (dt > .25f)
      {
        dt = (1.0f / 60.0f);
      }
    }

    // message loop
    os.HandleMessages();

    CameraInput(os, dt);

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // program loop

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);
    ImGui::SetNextWindowSize({ (float)os.GetWidth(), (float)os.GetHeight() });
    ImGui::Begin("root_window", nullptr, ImGuiWindowFlags_NoDecoration
                 | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking);
    ImGui::DockSpace(ImGui::GetID("root_window"), {}, ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    if (ImGui::BeginMainMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("View"))
      {
        ImGui::MenuItem("Cursor Highlight", 0, &cursor_highlight);
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Scene Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Checkbox("Show Vertex Normals", &draw_vertex_normals);
    if (draw_vertex_normals)
    {
      ImGui::ColorEdit3("Vertex Normals Color", &(vertex_normals_color.x));
    }
    ImGui::ColorEdit3("Clear Color", &clear_color.x);

    if (ImGui::CollapsingHeader("Object Properties"))
    {
      ImGui::DragFloat("Object Rotation", &model_rotation, 1, -360.0f, 360.0f);
      ImGui::DragFloat3("Object Position", (&model_position.x), 0.01f);
      ImGui::DragFloat("Object Size", (&model_scale), 0.01f, 0, D3D11_FLOAT32_MAX);
      ImGui::ColorEdit3("Object Color", &(model_color.x));
    }
    if (ImGui::CollapsingHeader("Camera Properties"))
    {
      ImGui::DragFloat("Camera FOV", &cam_fov, 0.1f, 20.0f, 130.0f);
    }
    if (ImGui::CollapsingHeader("Tree Properties"))
    {
      if (ImGui::BeginCombo("Tree Type", tree_type_names[tree_type]))
      {
        for (int i = 0; i < tree_COUNT; ++i)
        {
          if (ImGui::Selectable(tree_type_names[i], tree_type == i))
          {
            if (i != tree_type)
            {
              regenerate_tree((tree_type_t)i);
            }
            tree_type = (tree_type_t)i;
          }
        }
        ImGui::EndCombo();
      }

      ImGui::Checkbox("Draw Tree Bounds", &tree_settings.bounds);
      ImGui::Checkbox("Draw Tree Meshes", &tree_settings.mesh);
      if (tree_settings.mesh)
      {
        ImGui::Checkbox("Color Tree Meshes", &tree_settings.mesh_colors);
      }
      ImGui::InputInt("Tree Node Triangle Count", &tree_settings.node_size);
      if (ImGui::Button("Regenerate Tree"))
      {
        regenerate_tree(tree_type);
      }
    }
    ImGui::End();

    UpdateCamera();

    {
      using namespace DirectX;

      auto mousedata = os.GetMouseData();
      if (mousedata.left)
      {
        if (tree_type == tree_octree)
        {
          auto& tree = *((CS350::Octree*)bunny_tree);
          tree.Iterate([](CS350::OctreeNode* node, int)
          {
            node->Reset();
          });
        }

        sphere_gameobject.Reset({}, {});
        dx::XMStoreFloat3(&sphere_gameobject.position, camPosition);
        dx::XMStoreFloat3(&sphere_gameobject.velocity, camTarget - camPosition);
      }
    }

    if (tree_type == tree_octree)
    {
      auto& tree = *((CS350::Octree*)bunny_tree);
      tree.Iterate(sphere_gameobject);
    }

    sphere_gameobject.Update(dt);

    world_matrix = dx::XMMatrixIdentity();
    world_matrix *= dx::XMMatrixScaling(model_scale, model_scale, model_scale);
    world_matrix *= dx::XMMatrixRotationAxis(camUp, dx::XMConvertToRadians(model_rotation));
    world_matrix *= dx::XMMatrixTranslation(model_position.x, model_position.y, model_position.z);
    dx::XMStoreFloat4x4(&(per_object_data.World), world_matrix);
    dx::XMStoreFloat4x4(&(per_object_data.WorldNormal), CS350::InverseTranspose(world_matrix));

    dx::XMStoreFloat4x4(&(per_frame_data.View), cam_view_matrix);
    cam_projection_matrix = dx::XMMatrixPerspectiveFovRH(dx::XMConvertToRadians(cam_fov), (float)os.GetWidth() / (float)os.GetHeight(), 0.05f, 1000.0f);
    dx::XMStoreFloat4x4(&(per_frame_data.Projection), cam_projection_matrix);
    dx::XMStoreFloat4(&per_frame_data.CameraPosition, camPosition);
    per_object_data.Color = { model_color, 1.0f };

    render.UpdateUniform(per_frame_id, per_frame_data);
    render.UpdateUniform(per_object_id, per_object_data);

    // render scene
    render.SetClearColor({ 0, 0, 0, 0 });
    render.EnableDepthTest(true);
    render.RenderToFramebuffer(framebuffer);
    render.ClearFramebuffer(framebuffer);

    render.GetD3D11Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    render.SetRenderMode(CS350::Render_DX11::RenderMode::FILL);
    render.UseShader(deferred_geometry);
    render.UseTexture(metal_texture, 0);
    render.UseUniform(per_object_id, 0);
    render.UseUniform(per_frame_id, 1);

    // draw meshes
    if (tree_settings.mesh)
    {
      if (tree_type == tree_octree)
        ((CS350::Octree*)bunny_tree)->DrawMesh(render, per_object_id, tree_settings.mesh_colors);
      else if (tree_type == tree_kdtree)
        ((CS350::KdTree*)bunny_tree)->DrawMesh(render, per_object_id, tree_settings.mesh_colors);
    }

    // light meshes
    render.SetRenderMode(CS350::Render_DX11::RenderMode::FILL);
    render.EnableDepthTest(false);
    render.SetClearColor(clear_color);
    render.RenderTo(composited_texture);
    render.ClearRenderTexture(composited_texture);

    render.UseShader(deferred_lighting);
    for (int i = 0; i < 3; ++i)
    {
      render.UseFramebufferTexture(framebuffer, i, i);
    }
    render.UseMesh(CS350::Render_DX11::None);
    render.GetD3D11Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    render.GetD3D11Context()->Draw(4, 0);
    for (int i = 0; i < 3; ++i)
    {
      render.UnuseTexture(i);
    }

    // draw debug objects
    render.EnableDepthTest(true);
    render.UseFramebufferDepth(framebuffer);
    render.SetRenderMode(CS350::Render_DX11::RenderMode::WIREFRAME);
    render.GetD3D11Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    render.UseShader(debug_wireframe);
    if (tree_settings.bounds)
    {
      if (tree_type == tree_octree)
        ((CS350::Octree*)bunny_tree)->DrawBounds(render, per_object_id);
      else if (tree_type == tree_kdtree)
        ((CS350::KdTree*)bunny_tree)->DrawBounds(render, per_object_id);
    }
    per_object_data.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    render.UpdateUniform(per_object_id, per_object_data);
    sphere_gameobject.Draw(render, per_object_id);
    sphere_gameobject.DrawBounds(render, per_object_id);

    if (draw_vertex_normals)
    {
      render.UseShader(debug_line);
      per_object_data.Color = { vertex_normals_color, 1.0f };
      render.UpdateUniform(per_object_id, per_object_data);

      if (tree_type == tree_octree)
        ((CS350::Octree*)bunny_tree)->DrawMesh(render, per_object_id);
      else if (tree_type == tree_kdtree)
        ((CS350::KdTree*)bunny_tree)->DrawMesh(render, per_object_id);
    }

    render.SetClearColor({ 0.15f, 0.15f, 0.15f });
    render.RenderToFramebuffer(CS350::Render_DX11::Default);
    render.ClearFramebuffer(CS350::Render_DX11::Default);

    ImGui::Begin("Framebuffer");
    ImVec2 const target_size = { os.GetWidth() * 0.15f, os.GetHeight() * 0.15f };
    for (int i = 0; i < render.GetFramebufferTargetCount(framebuffer); ++i)
    {
      std::string itemid = "##Texture " + std::to_string(i);
      ImGui::Text("Texture %i", i);
      if (ImGui::Selectable(itemid.c_str(), i == selected_view, 0, target_size))
      {
        if (i == selected_view)
          selected_view = -1;
        else
          selected_view = i;
      }
      ImGui::SameLine();
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() - target_size.x - 8);
      ImGui::Image(render.DebugGetFramebufferTexture(framebuffer, i), target_size);
    }
    ImGui::End();

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);
    {
      if (selected_view == -1)
        ImGui::Image(render.DebugGetTexture(composited_texture), ImGui::GetContentRegionAvail());
      else
        ImGui::Image(render.DebugGetFramebufferTexture(framebuffer, selected_view), ImGui::GetContentRegionAvail());
    }
    ImGui::End();

    if (show_imgui_demo)
    {
      ImGui::ShowDemoWindow(&show_imgui_demo);
    }
    if (cursor_highlight)
    {
      ImGui::GetOverlayDrawList()->AddCircleFilled(io.MousePos, 10.0f, ImGui::GetColorU32({ 1, 0, 1, 0.5f }));
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();

    render.Present();
  }

  return 0;
}

int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
  return main();
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  return main();
}
