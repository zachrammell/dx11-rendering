#pragma once

#include "mesh.h"
#include "render_dx11.h"

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
extern CS350::Model unit_line_cube_model;
extern CS350::Render_DX11::MeshID unit_line_cube_mesh;