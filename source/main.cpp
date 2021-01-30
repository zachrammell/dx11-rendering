/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: main.cpp
Purpose: most of the functionality
Language: C++
Platform: MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS300_1
Author: Zach Rammell, zach.rammell
Creation date: 10/2/20
End Header --------------------------------------------------------*/
#include "os_win32.h"
#include "render_dx11.h"
#include "globals.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <iostream>
#include <system_error>
#include <filesystem>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "structures/per_object.h"

#include "math_helper.h"
namespace fs = std::filesystem;
namespace dx = DirectX;

namespace
{
int screen_width = 1280, screen_height = 720;
}

int main()
{
  CS350::OS_Win32 os{ TEXT("CS300 Project - Zach Rammell"), screen_width, screen_height };
  CS350::Render_DX11 render{ os };
  os.Show();

  dx::XMFLOAT3 clear_color{ 0.15f, 0.15f, 0.15f };

  dx::XMMATRIX cam_view_matrix;
  dx::XMMATRIX cam_projection_matrix;

  dx::XMFLOAT3 cam_position = { -5.0f, 3.0f, -10.0f };
  dx::XMFLOAT3 cam_target = { 0.0f, 0.0f, 0.0f };

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
  bool draw_face_normals = false;

  float cam_distance = 5.0f;
  float cam_fov = 90.0f;
  float cam_yaw = 0.0f, cam_pitch = 0.0f;
  float mouse_sensitivity = 0.005f;

  per_object per_object_data;

  CS350::Render_DX11::FramebufferID framebuffer = render.CreateFramebuffer(screen_width, screen_height, 3);

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

    CS350::OS_Win32::MouseData mouse_data = os.GetMouseData();

    if (mouse_data.right)
    {
      cam_pitch = std::clamp(cam_pitch - mouse_data.dy * mouse_sensitivity, -dx::XM_PIDIV2 + 0.05f, dx::XM_PIDIV2 - 0.05f);
      cam_yaw -= mouse_data.dx * mouse_sensitivity;
    }
    cam_distance = std::clamp(cam_distance - 0.15f * mouse_data.scroll_dy, 0.05f, 20.0f);

    dx::XMStoreFloat3(&cam_position, dx::XMVector3Transform(dx::XMVectorSet(0, 0, cam_distance, 0.0f), dx::XMMatrixRotationRollPitchYaw(cam_pitch, cam_yaw, 0.0f)));

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
      ImGui::MenuItem("File");
      ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Scene Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Checkbox("Show Vertex Normals", &draw_vertex_normals);
    if (draw_vertex_normals)
    {
      ImGui::ColorEdit3("Vertex Normals Color", &(vertex_normals_color.x));
    }
    ImGui::Checkbox("Show Face Normals", &draw_face_normals);
    if (draw_face_normals)
    {
      ImGui::ColorEdit3("Face Normals Color", &(face_normals_color.x));
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
      ImGui::DragFloat("Camera Distance", &cam_distance, 0.1f, 0.05f, 20.0f);
      ImGui::DragFloat3("Camera Position", &(cam_position.x));
      ImGui::DragFloat("Camera FOV", &cam_fov, 0.1f, 20.0f, 130.0f);
    }
    ImGui::End();

    world_matrix = dx::XMMatrixIdentity();
    world_matrix *= dx::XMMatrixScaling(model_scale, model_scale, model_scale);
    world_matrix *= dx::XMMatrixRotationAxis(cam_up, dx::XMConvertToRadians(model_rotation));
    world_matrix *= dx::XMMatrixTranslation(model_position.x, model_position.y, model_position.z);

    cam_projection_matrix = dx::XMMatrixPerspectiveFovRH(dx::XMConvertToRadians(cam_fov), (float)screen_width / (float)screen_height, 0.05f, 1000.0f);
    cam_view_matrix = dx::XMMatrixLookAtRH(dx::XMLoadFloat3(&cam_position), dx::XMLoadFloat3(&cam_target), cam_up);
    //dx::XMStoreFloat4x4(&(render.per_object.World), world_matrix);
    //dx::XMStoreFloat4x4(&(render.per_object.NormalWorld), CS350::InverseTranspose(world_matrix));
    //render.per_object.Color = model_color;

    //dx::XMStoreFloat4x4(&(render.per_frame_phong.ViewProjection), cam_view_matrix * cam_projection_matrix);
    //render.per_frame_phong.CameraPosition = cam_position;

    //render.per_frame_line.ViewProjection = render.per_frame_phong.ViewProjection;

    // render scene
    render.SetClearColor(clear_color);
    render.RenderToFramebuffer(framebuffer);
    render.ClearFramebuffer(framebuffer);

    render.SetClearColor({0.15f, 0.15f, 0.15f});
    render.RenderToFramebuffer(CS350::Render_DX11::Default);
    render.ClearDefaultFramebuffer();

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    {
      ImGui::Image(render.GetFramebufferTexture(framebuffer, 0), { (float)os.GetWidth(), (float)os.GetHeight() });
    }
    ImGui::End();

    ImGui::Begin("Framebuffer");
    for (int i = 0; i < render.GetFramebufferTargetCount(framebuffer); ++i)
    {
      ImGui::Text("Texture %i", i);
      ImGui::Image(render.GetFramebufferTexture(framebuffer, i), {os.GetWidth() * 0.15f, os.GetHeight() * 0.15f});
    }
    ImGui::End();

    if (show_imgui_demo)
    {
      ImGui::ShowDemoWindow(&show_imgui_demo);
    }
    ImGui::GetOverlayDrawList()->AddCircleFilled(io.MousePos, 20.0f, ImGui::GetColorU32({ 1, 0, 1, 0.5f }));

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
