#pragma once

#include <DirectXMath.h>

namespace dx = DirectX;

extern float light_brightness;
extern bool light_rainbow_mode;
extern int light_count;
extern dx::XMFLOAT3 wireframe_color;
extern dx::XMFLOAT3 light_color;
extern float sphere_rotation;
extern dx::XMVECTOR cam_up;
extern dx::XMFLOAT3 model_position;
extern dx::XMFLOAT3 model_color;
extern dx::XMMATRIX world_matrix;
extern dx::XMFLOAT3 vertex_normals_color;
extern dx::XMFLOAT3 face_normals_color;
extern float Reflectivity;
