#include "Platform/OS_Win32.h"
#include "Graphics/Render_DX11.h"
#include "globals.h"

#include <fstream>
#include <filesystem>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "Structures/per_object.h"
#include "Structures/per_frame.h"
#include "MathHelper.h"

namespace fs = std::filesystem;
namespace dx = DirectX;

using namespace IE;

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

namespace
{
float4 headColor = float4(1, 1, 1, 1);
float4 bodyColor = float4(1, 1, 1, 1);
bool characterTextureDirty = true;
}

void CameraInput(Platform::OS_Win32& os, float dt)
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
  Graphics::GfxDevice_DX11::MeshID mesh;

  void Update(float dt)
  {
    position += velocity * dt;
  }

  void Draw(Graphics::GfxDevice_DX11& render, Graphics::GfxDevice_DX11::UniformID per_object_id)
  {
    render.UseMesh(mesh);
    DrawHelper(render, per_object_id);
  }

  void DrawBounds(Graphics::GfxDevice_DX11& render, Graphics::GfxDevice_DX11::UniformID per_object_id)
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

private:
  void DrawHelper(Graphics::GfxDevice_DX11& render, Graphics::GfxDevice_DX11::UniformID per_object_id)
  {
    dx::XMMATRIX world_matrix = dx::XMMatrixIdentity();
    world_matrix *= dx::XMMatrixScaling(scale.x, scale.y, scale.z);
    world_matrix *= dx::XMMatrixTranslation(position.x, position.y, position.z);

    per_object per_object_data;
    dx::XMStoreFloat4x4(&(per_object_data.World), world_matrix);
    dx::XMStoreFloat4x4(&(per_object_data.WorldNormal), IE::InverseTranspose(world_matrix));
    per_object_data.Color = color;

    render.UpdateUniform(per_object_id, per_object_data);
    render.Draw();
  }
};

int main()
{
  Platform::OS_Win32 os{ TEXT("Project Impostor"), 1600, 900 };
  Graphics::GfxDevice_DX11 render{ os };
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

  int selected_view = -1;

  per_object per_object_data;
  per_frame per_frame_data;
  per_frame_data.AmbientColor = float4(0.031, 0.027, 0.031, 1.0f);

  Graphics::Model sphere_model = Graphics::Model::GenerateSphere(16, 16);
  Graphics::GfxDevice_DX11::MeshID sphere_mesh = render.CreateMesh(sphere_model);
  GameObject sphere_gameobject{ {1e38, 1e38, 1e38}, {0, 0.15f, 0}, {0.1f, 0.1f, 0.1f}, {1, 1, 1, 1}, sphere_mesh };

  Image base_lines_image = Image("assets/textures/character/base_lines.png");
  Graphics::GfxDevice_DX11::TextureID base_lines_tex = render.CreateTexture(base_lines_image);
  Image base_body_image = Image("assets/textures/character/base_body.png");
  Graphics::GfxDevice_DX11::TextureID base_body_tex = render.CreateTexture(base_body_image);
  Image base_head_image = Image("assets/textures/character/base_head.png");
  Graphics::GfxDevice_DX11::TextureID base_head_tex = render.CreateTexture(base_head_image);

  Graphics::Model bunny_model = Graphics::Model::Load("assets/models/bunny_high_poly.obj");
  Graphics::GfxDevice_DX11::MeshID bunny_mesh = render.CreateMesh(bunny_model);
  GameObject bunny_gameobject{ {0, 0, 0}, {0, 0, 0}, {1, 1, 1}, {1, 0, 0, 1}, bunny_mesh };

  unit_line_cube_mesh = render.CreateMesh(unit_line_cube_model);
  unit_quad_mesh = render.CreateMesh(unit_quad_model);

  GameObject character_gameobject{ {0, 0, 0}, {0, 0, 0}, {1, 1, 1}, {1, 1, 1, 1}, unit_quad_mesh };

  Graphics::GfxDevice_DX11::FramebufferID framebuffer = render.CreateFramebuffer(os.GetWidth(), os.GetHeight(), 4);
  Graphics::GfxDevice_DX11::ShaderID deferred_geometry =
    render.CreateShader(TEXT("assets/shaders/deferred_geometry.hlsl"),
                        (Graphics::Shader::InputLayout_POS | Graphics::Shader::InputLayout_NOR | Graphics::Shader::InputLayout_TEX)
    );

  Graphics::GfxDevice_DX11::UniformID per_object_id = render.CreateUniform(per_object_data);
  Graphics::GfxDevice_DX11::UniformID per_frame_id = render.CreateUniform(per_frame_data);

  Graphics::GfxDevice_DX11::TextureID character_texture = render.CreateRenderTexture(base_lines_image.GetWidth(), base_lines_image.GetHeight());

  Graphics::GfxDevice_DX11::TextureID composited_texture = render.CreateRenderTexture(os.GetWidth(), os.GetHeight());

  Graphics::GfxDevice_DX11::ShaderID deferred_lighting =
    render.CreateShader(TEXT("assets/shaders/deferred_lighting.hlsl"),
                        0
    );

  Graphics::GfxDevice_DX11::ShaderID fsq =
    render.CreateShader(TEXT("assets/shaders/fullscreen_quad_render.hlsl"),
                        0
    );

  Graphics::GfxDevice_DX11::ShaderID debug_line =
    render.CreateShader(TEXT("assets/shaders/debug_line.hlsl"),
                        (Graphics::Shader::InputLayout_POS | Graphics::Shader::InputLayout_NOR),
                        true
    );

  Graphics::GfxDevice_DX11::ShaderID debug_wireframe =
    render.CreateShader(TEXT("assets/shaders/debug_wireframe.hlsl"),
                        (Graphics::Shader::InputLayout_POS | Graphics::Shader::InputLayout_NOR | Graphics::Shader::InputLayout_TEX)
    );

  Graphics::GfxDevice_DX11::ShaderID flat =
    render.CreateShader(TEXT("assets/shaders/flat.hlsl"),
                        (Graphics::Shader::InputLayout_POS | Graphics::Shader::InputLayout_NOR | Graphics::Shader::InputLayout_TEX)
    );

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
    ImGui::End();

    UpdateCamera();

    sphere_gameobject.Update(dt);

    // render the character layers into a rendertexture
    if (characterTextureDirty)
    {
      characterTextureDirty = false;

      per_object flat_per_object;
      flat_per_object.World = float4x4::identity();
      flat_per_object.WorldNormal = float4x4::identity();

      render.GetD3D11Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      render.SetRenderMode(Graphics::GfxDevice_DX11::RenderMode::FILL);
      render.RenderTo(character_texture);
      render.SetClearColor({ 0, 0, 0, 0 });
      render.ClearRenderTexture(character_texture);
      render.UseShader(flat);
      render.UseUniform(per_object_id, 0);
      render.UseMesh(unit_quad_mesh);

      flat_per_object.Color = bodyColor;
      render.UpdateUniform(per_object_id, flat_per_object);
      render.UseTexture(base_body_tex, 0);
      render.Draw();
      flat_per_object.Color = headColor;
      render.UpdateUniform(per_object_id, flat_per_object);
      render.UseTexture(base_head_tex, 0);
      render.Draw();
      flat_per_object.Color = float4(1, 1, 1, 1);
      render.UpdateUniform(per_object_id, flat_per_object);
      render.UseTexture(base_lines_tex, 0);
      render.Draw();
    }

    world_matrix = dx::XMMatrixIdentity();
    world_matrix *= dx::XMMatrixScaling(model_scale, model_scale, model_scale);
    world_matrix *= dx::XMMatrixRotationAxis(camUp, dx::XMConvertToRadians(model_rotation));
    world_matrix *= dx::XMMatrixTranslation(model_position.x, model_position.y, model_position.z);
    dx::XMStoreFloat4x4(&(per_object_data.World), world_matrix);
    dx::XMStoreFloat4x4(&(per_object_data.WorldNormal), IE::InverseTranspose(world_matrix));

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
    render.SetRenderMode(Graphics::GfxDevice_DX11::RenderMode::FILL);
    render.UseShader(deferred_geometry);
    render.UseUniform(per_object_id, 0);
    render.UseUniform(per_frame_id, 1);

    // draw meshes
    render.UseTexture(Graphics::GfxDevice_DX11::None, 0);
    bunny_gameobject.Draw(render, per_object_id);
    render.UseTexture(character_texture, 0);
    character_gameobject.Draw(render, per_object_id);

    // light meshes
    render.SetRenderMode(Graphics::GfxDevice_DX11::RenderMode::FILL);
    render.EnableDepthTest(false);
    render.SetClearColor(clear_color);
    render.RenderTo(composited_texture);
    render.ClearRenderTexture(composited_texture);

    render.UseShader(deferred_lighting);
    for (int i = 0; i < 3; ++i)
    {
      render.UseFramebufferTexture(framebuffer, i, i);
    }
    render.UseMesh(Graphics::GfxDevice_DX11::None);
    render.GetD3D11Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    render.GetD3D11Context()->Draw(4, 0);
    for (int i = 0; i < 3; ++i)
    {
      render.UnuseTexture(i);
    }

    // draw debug objects
    render.EnableDepthTest(true);
    render.UseFramebufferDepth(framebuffer);
    render.SetRenderMode(Graphics::GfxDevice_DX11::RenderMode::WIREFRAME);
    render.GetD3D11Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    render.UseShader(debug_wireframe);

    per_object_data.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    render.UpdateUniform(per_object_id, per_object_data);
    sphere_gameobject.Draw(render, per_object_id);
    sphere_gameobject.DrawBounds(render, per_object_id);

    if (draw_vertex_normals)
    {
      render.UseShader(debug_line);
      per_object_data.Color = { vertex_normals_color, 1.0f };
      render.UpdateUniform(per_object_id, per_object_data);
    }

    render.SetClearColor({ 0.15f, 0.15f, 0.15f });
    render.RenderToFramebuffer(Graphics::GfxDevice_DX11::Default);
    render.ClearFramebuffer(Graphics::GfxDevice_DX11::Default);

    if (ImGui::Begin("Framebuffer"))
    {
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
    }
    ImGui::End();

    if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar))
    {
      if (selected_view == -1)
        ImGui::Image(render.DebugGetTexture(composited_texture), ImGui::GetContentRegionAvail());
      else
        ImGui::Image(render.DebugGetFramebufferTexture(framebuffer, selected_view), ImGui::GetContentRegionAvail());
    }
    ImGui::End();

    if (ImGui::Begin("Character Customizer"))
    {
      if (ImGui::ColorEdit3("Head Color", &headColor.x))
      {
        characterTextureDirty = true;
      }
      if (ImGui::ColorEdit3("Body Color", &bodyColor.x))
      {
        characterTextureDirty = true;
      }
      ImGui::Text("Preview:");
      ImGui::Image(render.DebugGetTexture(character_texture), { 256, 256 });
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
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
    }

    render.Present();
  }

  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();

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
