// cs300-project.cpp : Defines the entry point for the application.
//

#include "cs300-project.h"

#include "os_win32.h"
#include "render_dx11.h"
#include "color.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>

namespace
{

ID3D11Buffer* cube_vertex_buffer;
ID3D11Buffer* cube_index_buffer;
ID3D11Buffer* cb_per_object_buffer;

ID3D11VertexShader* vertex_shader;
ID3D11PixelShader* pixel_shader;
ID3DBlob* vertex_shader_buffer;
ID3DBlob* pixel_shader_buffer;
ID3D11InputLayout* vertex_shader_input_layout;

int screen_width = 960, screen_height = 720;

}

int main()
{
  CS300::OS_Win32 os { TEXT("CS300 Project - Zach Rammell"), screen_width, screen_height };
  CS300::Render_DX11 render { os };

  CS300::color_RGBA clear_color { 0.25f, 0.25f, 0.25f, 1.0f };

  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
  flags |= D3DCOMPILE_DEBUG;
#endif

  ID3DBlob* error_buffer;
  HRESULT hr;
  LPCWSTR basic_shader_path = L"assets/shaders/basic.hlsl";

  // Compile vertex shader
  hr = D3DCompileFromFile(
    basic_shader_path,
    nullptr,
    D3D_COMPILE_STANDARD_FILE_INCLUDE,
    "vs_main",
    "vs_5_0",
    flags,
    0,
    &vertex_shader_buffer,
    &error_buffer
  );

  if (FAILED(hr)) {
    if (error_buffer) {
      OutputDebugStringA((char*)error_buffer->GetBufferPointer());
      error_buffer->Release();
    }
    if (vertex_shader_buffer) { vertex_shader_buffer->Release(); }
    assert(!"Couldn't compile vertex shader");
  }

  // Compile pixel shader
  hr = D3DCompileFromFile(
    basic_shader_path,
    nullptr,
    D3D_COMPILE_STANDARD_FILE_INCLUDE,
    "ps_main",
    "ps_5_0",
    flags,
    0,
    &pixel_shader_buffer,
    &error_buffer
  );

  if (FAILED(hr)) {
    if (error_buffer) {
      OutputDebugStringA((char*)error_buffer->GetBufferPointer());
      error_buffer->Release();
    }
    if (pixel_shader_buffer) { pixel_shader_buffer->Release(); }
    assert(!"Couldn't compile pixel shader");
  }

  hr = render.GetD3D11Device()->CreateVertexShader(
    vertex_shader_buffer->GetBufferPointer(),
    vertex_shader_buffer->GetBufferSize(),
    nullptr,
    &vertex_shader
  );
  assert(SUCCEEDED(hr));

  hr = render.GetD3D11Device()->CreatePixelShader(
    pixel_shader_buffer->GetBufferPointer(),
    pixel_shader_buffer->GetBufferSize(),
    nullptr,
    &pixel_shader
  );
  assert(SUCCEEDED(hr));

  D3D11_INPUT_ELEMENT_DESC input_element_descriptor[]
  {
    {"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    /*
    { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    */
  };

  hr = render.GetD3D11Device()->CreateInputLayout(
    input_element_descriptor,
    ARRAYSIZE(input_element_descriptor),
    vertex_shader_buffer->GetBufferPointer(),
    vertex_shader_buffer->GetBufferSize(),
    &vertex_shader_input_layout
  );
  assert(SUCCEEDED(hr));


  // create vertex buffer

  struct SimpleVertex
  {
    DirectX::XMFLOAT3 pos;   // Position
    DirectX::XMFLOAT3 color; // Color
  };

  SimpleVertex vertex_data_array[] = { // x, y, z, r, g, b
  {{0.5f,  0.5f,   0.5f}, // front top-right
    {0.0f, 1.0f, 1.0f}},   // cyan

  {{-0.5f,  0.5f,  0.5f}, // front top-left
    {1.0f, 1.0f, 1.0f}},   // white

  {{0.5f, -0.5f,   0.5f}, // front bottom-right
    {1.0f, 0.0f, 1.0f}},   // magenta

  {{-0.5f, -0.5f,  0.5f}, // front bottom-left
    {1.0f, 1.0f, 0.0f}},   // yellow

  {{0.5f,  0.5f,  -0.5f}, // back top-right
    {0.0f, 1.0f, 0.0f}},   // green

  {{-0.5f,  0.5f, -0.5f}, // back top-left
    {0.0f, 0.0f, 0.0f}},   // black

  {{-0.5f, -0.5f, -0.5f}, // back bottom-left
    {1.0f, 0.0f, 0.0f}},   // red

  {{0.5f, -0.5f,  -0.5f}, // back bottom-right
    {0.0f, 0.0f, 1.0f}},   // blue
  };
  UINT vertex_stride = sizeof(SimpleVertex);
  UINT vertex_offset = 0;
  

  {
    D3D11_BUFFER_DESC vertex_buffer_descriptor
    {
      sizeof(vertex_data_array),
      D3D11_USAGE_IMMUTABLE,
      D3D11_BIND_VERTEX_BUFFER,
      0, 0, 0
    };
    D3D11_SUBRESOURCE_DATA subresource_data{ vertex_data_array, 0, 0 };

    hr = render.GetD3D11Device()->CreateBuffer(&vertex_buffer_descriptor, &subresource_data, &cube_vertex_buffer);
    assert(SUCCEEDED(hr));
  }

  UINT index_data_array[] =
  {
    3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0
    //0, 1, 2, 3
  };

  UINT vertex_count = ARRAYSIZE(index_data_array);

  {
    D3D11_BUFFER_DESC index_buffer_descriptor
    {
      sizeof(index_data_array),
      D3D11_USAGE_IMMUTABLE,
      D3D11_BIND_INDEX_BUFFER,
      0, 0, 0
    };
    D3D11_SUBRESOURCE_DATA subresource_data{ index_data_array, 0, 0 };

    hr = render.GetD3D11Device()->CreateBuffer(&index_buffer_descriptor, &subresource_data, &cube_index_buffer);
    assert(SUCCEEDED(hr));
  }

  DirectX::XMMATRIX WVP;
  DirectX::XMMATRIX world_matrix;
  DirectX::XMMATRIX cam_view_matrix;
  DirectX::XMMATRIX cam_projection_matrix;

  DirectX::XMVECTOR cam_position;
  DirectX::XMVECTOR cam_target;
  DirectX::XMVECTOR cam_up;

  struct cb_per_object
  {
    DirectX::XMMATRIX WVP;
  } cb_per_obj;

  {
    D3D11_BUFFER_DESC cb_buffer_descriptor
    {
      sizeof(cb_per_object),
      D3D11_USAGE_DEFAULT,
      D3D11_BIND_CONSTANT_BUFFER,
      0, 0, 0
    };
    hr = render.GetD3D11Device()->CreateBuffer(&cb_buffer_descriptor, nullptr, &cb_per_object_buffer);
    assert(SUCCEEDED(hr));
  }

  cam_position = DirectX::XMVectorSet(0.0f, 2.0f, -2.0f, 0.0f);
  cam_target = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
  cam_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

  cam_view_matrix = DirectX::XMMatrixLookAtLH(cam_position, cam_target, cam_up);
  cam_projection_matrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(70.0f), (float)screen_width / (float)screen_height, 1000.0f, 0.05f);

  world_matrix = DirectX::XMMatrixIdentity();
  world_matrix *= DirectX::XMMatrixRotationAxis(cam_up, DirectX::XMConvertToRadians(35.0f + 90.0f));
  WVP = world_matrix * cam_view_matrix * cam_projection_matrix;

  cb_per_obj.WVP = DirectX::XMMatrixTranspose(WVP);

  render.GetD3D11Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  render.GetD3D11Context()->IASetInputLayout(vertex_shader_input_layout);
  render.GetD3D11Context()->IASetVertexBuffers(
    0,
    1,
    &cube_vertex_buffer,
    &vertex_stride,
    &vertex_offset
  );
  render.GetD3D11Context()->IASetIndexBuffer(cube_index_buffer, DXGI_FORMAT_R32_UINT, 0);

  render.GetD3D11Context()->VSSetShader(vertex_shader, nullptr, 0);
  render.GetD3D11Context()->PSSetShader(pixel_shader, nullptr, 0);

  render.GetD3D11Context()->UpdateSubresource(cb_per_object_buffer, 0, nullptr, &cb_per_obj, 0, 0);
  render.GetD3D11Context()->VSSetConstantBuffers(0, 1, &cb_per_object_buffer);

  double current_time = os.GetTime();

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

    // program loop

    if (0)
    {
      float col = (sin(current_time) + 1.0f) / 2.0f;
      clear_color = { col, col, col, clear_color.a };
    }

    world_matrix *= DirectX::XMMatrixRotationAxis(cam_up, DirectX::XMConvertToRadians(0.5f));
    WVP = world_matrix * cam_view_matrix * cam_projection_matrix;
    cb_per_obj.WVP = DirectX::XMMatrixTranspose(WVP);

    render.SetClearColor(clear_color);
    render.DrawScene();

    render.GetD3D11Context()->UpdateSubresource(cb_per_object_buffer, 0, nullptr, &cb_per_obj, 0, 0);
    render.GetD3D11Context()->VSSetConstantBuffers(0, 1, &cb_per_object_buffer);
    render.GetD3D11Context()->DrawIndexed(vertex_count, 0, 0);

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
