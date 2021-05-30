#pragma once
#include <d3d11.h>
#include <string>

namespace IE::Graphics
{

class GfxDevice_DX11;

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
  friend class GfxDevice_DX11;
  Shader(GfxDevice_DX11& render, LPCWSTR shader_path, int input_layout, bool GS = false);

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
