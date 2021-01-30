#include "../structures/per_object.h"
#include "phong_common.hlsl"

// PEEPO WEIRD CLAP

cbuffer cb_per_object : register(b0)
{
  per_object data;
}

cbuffer cb_per_frame : register(b1)
{
  float4x4 ViewProjection;
  float4   CameraPosition;
  float4 AmbientColor;
  float4 FogColor;
  float FogNear;
  float FogFar;
}

Texture2D TextureDiffuse : register(t0);
Texture2D TextureSpecular : register(t1);
SamplerState Sampler : register(s0);

/* vertex attributes go here to input to the vertex shader */
struct vs_in
{
  float3 position_local : POS;
  float3 normal : NOR;
  float2 tex_coord : TEX;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out
{
  float4 position_clip : SV_POSITION; // required output of VS
  float4 position_world : TEXCOORD0;
  float2 uv : TEXCOORD1;
  float3 normal : COLOR0;
};

vs_out vs_main(vs_in input)
{
  vs_out output;
  output.position_world = mul(data.World, float4(input.position_local, 1.0f));
  output.position_clip = mul(ViewProjection, output.position_world);
  output.normal = mul(data.WorldNormal, input.normal);
  output.uv = input.tex_coord;
  return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
  const float3 view = normalize(CameraPosition.xyz - input.position_world.xyz);
  const float3 normal = normalize(input.normal);

  float3 diffuse_color = data.Color.rgb;
  float2 uv = input.uv;

  diffuse_color *= TextureDiffuse.Sample(Sampler, uv);

  const float3 ambient_color = diffuse_color * AmbientColor;
  float3 color = ambient_color;

  light_contribution all_of_the_lights;

  for (int i = 0; i < LightCount; ++i)
  {
    switch (LightType[i].x)
    {
    case LT_POINT:
    {
      light_contribution point_light = PointLight(
        make_point_light(LightColors[i].rgb, LightPositions[i].xyz),
        input.position_world.xyz, view, normal
      );
      all_of_the_lights.diffuse += point_light.diffuse;
      all_of_the_lights.specular += point_light.specular;
    }
    break;
    case LT_DIRECTIONAL:
    {
      light_contribution directional_light = DirectionalLight(
        make_directional_light(LightColors[i].rgb, LightDirections[i].xyz),
        view, normal
      );
      all_of_the_lights.diffuse += directional_light.diffuse;
      all_of_the_lights.specular += directional_light.specular;
    }
    break;
    case LT_SPOTLIGHT:
    {
      const float inner_angle = LightSpotlightData[i].x;
      const float outer_angle = LightSpotlightData[i].y;
      const float falloff = LightSpotlightData[i].z;
      light_contribution spot_light = SpotLight(
        make_spot_light(LightColors[i].rgb, LightPositions[i].xyz, LightDirections[i].xyz, inner_angle, outer_angle, falloff),
        input.position_world.xyz, view, normal
      );
      all_of_the_lights.diffuse += spot_light.diffuse;
      all_of_the_lights.specular += spot_light.specular;
    }
    break;
    default:
      color += float3(1, 0, 1);
      break;
    }
  }

  color += diffuse_color * all_of_the_lights.diffuse;
  color += TextureSpecular.Sample(Sampler, uv).r * all_of_the_lights.specular;
  color = Fog(color, FogColor.rgb, length(CameraPosition.xyz - input.position_world.xyz), FogNear, FogFar);

  return float4(color, 1.0f); // must return an RGBA color
}
