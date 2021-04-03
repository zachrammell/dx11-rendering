#include "../structures/per_object.h"
#include "../structures/per_frame.h"
#include "../structures/per_light.h"

#include "phong_common.hlsl"

cbuffer cb_per_object : register(b0)
{
  per_object obj_data;
}

cbuffer cb_per_frame : register(b1)
{
  per_frame frame_data;
}

cbuffer cb_per_light : register(b2)
{
  lights light_data;
}

Texture2D TextureDiffuse : register(t0);
Texture2D TextureNormal : register(t1);
Texture2D TexturePosition : register(t2);

SamplerState Sampler : register(s0);

/* vertex attributes go here to input to the vertex shader */
struct vs_in
{
  uint id : SV_VERTEXID;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out
{
  float4 position_clip : SV_POSITION; // required output of VS
  float2 texture_coord : TEXCOORD0;
};

vs_out vs_main(vs_in input)
{
  vs_out output;
  output.texture_coord = float2(input.id & 1, input.id >> 1);
  output.position_clip = float4((output.texture_coord.x - 0.5f) * 2, -(output.texture_coord.y - 0.5f) * 2, 0, 1);
  return output;
}

struct ps_out
{
  float4 color : SV_TARGET0;
};

ps_out ps_main(vs_out input) : SV_TARGET
{
  ps_out output;

  float3 view = frame_data.CameraPosition - TexturePosition.Sample(Sampler, input.texture_coord);
  float3 normal = TextureNormal.Sample(Sampler, input.texture_coord).xyz;
  float3 frag_pos = TexturePosition.Sample(Sampler, input.texture_coord);
  float3 white = float3(1, 1, 1);

  float4 diffuse_color = TextureDiffuse.Sample(Sampler, input.texture_coord);
  if (diffuse_color.a < 0.01f)
  {
    discard;
  }

  float3 intensity = 0.0f;
  //for (int i = 0; i < 20; ++i)
  {
    light_contribution lc = PointLight(
      make_point_light(white, float3(0, 0, 0)),
      frag_pos, view, normal, float3(0.5f, 0.5f, 0.5f)
      );
    intensity += lc.diffuse + lc.specular;
  }

  output.color = frame_data.AmbientColor + float4(intensity * diffuse_color.xyz, 1.0f);
  //output.color *= 

  return output;
}
