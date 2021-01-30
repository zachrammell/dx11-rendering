#include "globals.h"

float light_brightness = 1.0f;
bool light_rainbow_mode = true;
int light_count = 2;
dx::XMFLOAT3 wireframe_color = { 0.75f, 0.75f, 0.75f };
dx::XMFLOAT3 light_color = { 1, 0.980392157, 0.788235294 };
float sphere_rotation = 0;

dx::XMVECTOR cam_up = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
dx::XMFLOAT3 model_position = {};

dx::XMFLOAT3 model_color = { 0.75f, 0.75f, 0.75f };
dx::XMMATRIX world_matrix;

dx::XMFLOAT3 vertex_normals_color = { 0, 1, 1 };
dx::XMFLOAT3 face_normals_color = { 1, 0, 1 };
float Reflectivity = 0.5f;
