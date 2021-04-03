/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: shader.h
Language: C++
Platform: Windows 8.1+, MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS300_1
Author: Zach Rammell, zach.rammell
Creation date: 10/2/20
End Header --------------------------------------------------------*/
#pragma once
#include <d3d11.h>
#include <string>

namespace CS350
{

class Render_DX11;

class Shader
{
public:
  // TODO: replace InputLayout system with parsing the shader file and handling it automatically
  enum InputLayout
  {
    InputLayout_POS = 1 << 0,
    InputLayout_COL = 1 << 1,
    InputLayout_NOR = 1 << 2,
    InputLayout_TEX = 1 << 3,
    // bookkeeping
    InputLayout_COUNT = 4,
  };

  ID3D11InputLayout* GetInputLayout() const
  {
    return vertex_shader_input_layout;
  }

private:
  friend class Render_DX11;
  Shader(Render_DX11& render, LPCWSTR shader_path, int input_layout, bool GS = false);

  std::wstring shader_path;
  ID3D11VertexShader* vertex_shader;
  ID3D11PixelShader* pixel_shader;
  ID3D11GeometryShader* geometry_shader = nullptr;
  ID3DBlob* vertex_shader_buffer;
  ID3DBlob* pixel_shader_buffer;
  ID3DBlob* geometry_shader_buffer;
  ID3D11InputLayout* vertex_shader_input_layout = nullptr;
};

}
