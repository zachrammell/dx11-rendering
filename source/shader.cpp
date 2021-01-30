/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: shader.cpp
Language: C++
Platform: Windows 8.1+, MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS300_1
Author: Zach Rammell, zach.rammell
Creation date: 10/2/20
End Header --------------------------------------------------------*/
#include "shader.h"

#include "render_dx11.h"

#include <cassert>
#include <d3dcompiler.h>
#include <iostream>

namespace CS350
{

Shader::Shader(Render_DX11& render, LPCWSTR shader_path, int input_layout, bool GS)
  : shader_path(shader_path)
{
  HRESULT hr;
  ID3DBlob* error_buffer;

  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
  flags |= D3DCOMPILE_DEBUG;
#endif

  // Compile vertex shader
  hr = D3DCompileFromFile(
    shader_path,
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
    std::cerr << std::system_category().message(hr) << std::endl;
    if (error_buffer) {
      std::cerr << (char*)error_buffer->GetBufferPointer() << std::endl;
      error_buffer->Release();
    }
    if (vertex_shader_buffer) { vertex_shader_buffer->Release(); }
    assert(!"Couldn't compile vertex shader");
  }

  // Compile pixel shader
  hr = D3DCompileFromFile(
    shader_path,
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
    std::cerr << std::system_category().message(hr) << std::endl;
    if (error_buffer) {
      std::cerr << (char*)error_buffer->GetBufferPointer() << std::endl;
      error_buffer->Release();
    }
    if (pixel_shader_buffer) { pixel_shader_buffer->Release(); }
    assert(!"Couldn't compile pixel shader");
  }

  if (GS)
  {
    // Compile geometry shader
    hr = D3DCompileFromFile(
      shader_path,
      nullptr,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "gs_main",
      "gs_5_0",
      flags,
      0,
      &geometry_shader_buffer,
      &error_buffer
    );

    if (FAILED(hr)) {
      std::cerr << std::system_category().message(hr) << std::endl;
      if (error_buffer) {
        std::cerr << (char*)error_buffer->GetBufferPointer() << std::endl;
        error_buffer->Release();
      }
      if (geometry_shader_buffer) { geometry_shader_buffer->Release(); }
    }
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

  if (GS && geometry_shader_buffer)
  {
    hr = render.GetD3D11Device()->CreateGeometryShader(
      geometry_shader_buffer->GetBufferPointer(),
      geometry_shader_buffer->GetBufferSize(),
      nullptr,
      &geometry_shader
    );
    assert(SUCCEEDED(hr));
  }

  D3D11_INPUT_ELEMENT_DESC input_element_descriptor[InputLayout_COUNT]{};
  int input_element_count = 0;
  for (int i = 0; i < InputLayout_COUNT; ++i)
  {
    if (input_layout & (1 << i))
    {
      ++input_element_count;
    }
  }
  assert(input_element_count > 0);

  int elements_filled = 0;
  if (input_layout & InputLayout_POS)
  {
    input_element_descriptor[elements_filled] =
      { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    ++elements_filled;
  }
  if (input_layout & InputLayout_COL)
  {
    input_element_descriptor[elements_filled] =
      { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    ++elements_filled;
  }
  if (input_layout & InputLayout_NOR)
  {
    input_element_descriptor[elements_filled] =
      { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    ++elements_filled;
  }
  if (input_layout & InputLayout_TEX)
  {
    input_element_descriptor[elements_filled] =
      { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    ++elements_filled;
  }

  hr = render.GetD3D11Device()->CreateInputLayout(
    input_element_descriptor,
    input_element_count,
    vertex_shader_buffer->GetBufferPointer(),
    vertex_shader_buffer->GetBufferSize(),
    &vertex_shader_input_layout
  );
  assert(SUCCEEDED(hr));
}

}
