#pragma once

#include "graphics/mesh.h"
#include "graphics/render_dx11.h"

#include <DirectXMath.h>
namespace dx = DirectX;
#include <WindowsNumerics.h>
using namespace Windows::Foundation::Numerics;

extern float light_brightness;
extern bool light_rainbow_mode;
extern int light_count;
extern float3 wireframe_color;
extern float3 light_color;
extern float sphere_rotation;
extern float3 model_position;
extern float3 model_color;
extern dx::XMMATRIX world_matrix;
extern float3 vertex_normals_color;
extern float3 face_normals_color;
extern float Reflectivity;

extern IE::Graphics::Model unit_line_cube_model;
extern IE::Graphics::GfxDevice_DX11::MeshID unit_line_cube_mesh;
extern IE::Graphics::Model unit_quad_model;
extern IE::Graphics::GfxDevice_DX11::MeshID unit_quad_mesh;
