#pragma once

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
extern dx::XMVECTOR cam_up;
extern float3 model_position;
extern float3 model_color;
extern dx::XMMATRIX world_matrix;
extern float3 vertex_normals_color;
extern float3 face_normals_color;
extern float Reflectivity;
