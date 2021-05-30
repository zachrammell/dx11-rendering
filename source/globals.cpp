#include "globals.h"

float light_brightness = 1.0f;
bool light_rainbow_mode = true;
int light_count = 2;
float3 wireframe_color = { 0.75f, 0.75f, 0.75f };
float3 light_color = { 1, 0.980392157, 0.788235294 };
float sphere_rotation = 0;

float3 model_position = {};

float3 model_color = { 0.75f, 0.75f, 0.75f };
dx::XMMATRIX world_matrix;

float3 vertex_normals_color = { 0, 1, 1 };
float3 face_normals_color = { 1, 0, 1 };
float Reflectivity = 0.5f;

IE::Graphics::Model unit_line_cube_model = IE::Graphics::Model::GenerateUnitLineCube();
IE::Graphics::GfxDevice_DX11::MeshID unit_line_cube_mesh = -1;

IE::Graphics::Model unit_quad_model = IE::Graphics::Model::GenerateUnitQuad();
IE::Graphics::GfxDevice_DX11::MeshID unit_quad_mesh = -1;
