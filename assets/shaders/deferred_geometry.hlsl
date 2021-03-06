






























struct per_object { 
  float4x4 World ;
  float4x4 WorldNormal ;
  float4 Color ;
} ;









struct per_frame { 
  float4x4 View ;
  float4x4 Projection ;
  float4 CameraPosition ;
  float4 AmbientColor ;
} ;




cbuffer cb_per_object : register(b0)
{
  per_object obj_data;
}

cbuffer cb_per_frame : register(b1)
{
  per_frame frame_data;
}

Texture2D TextureDiffuse : register(t0);
Texture2D TextureSpecular : register(t1);
SamplerState Sampler : register(s0);

 
struct vs_in
{
  float3 position_local : POS;
  float3 normal : NOR;
  float2 tex_coord : TEX;
};

 
struct vs_out
{
  float4 position_clip : SV_POSITION; 
  float4 position_view : TEXCOORD0;   
  float4 position_world : TEXCOORD2;
  float2 uv : TEXCOORD1;
  float3 normal : COLOR0;
};

vs_out vs_main(vs_in input)
{
  vs_out output;
  output.position_world = mul(obj_data.World, float4(input.position_local, 1.0f));
  output.position_view = mul(frame_data.View, output.position_world);
  output.position_clip = mul(frame_data.Projection, output.position_view);
  output.normal = mul(obj_data.WorldNormal, float4(input.normal, 0.0f));
  output.uv = input.tex_coord;
  return output;
}

struct ps_out
{
  float4 color : SV_TARGET0;
  float4 normal : SV_TARGET1;
  float4 position : SV_TARGET2;
};

ps_out ps_main(vs_out input) : SV_TARGET
{
  ps_out output;
  const float3 normal = normalize(input.normal);

  float2 uv = input.uv;
  float4 diffuse_color = TextureDiffuse.Sample(Sampler, uv);
  if (diffuse_color.a < 0.01f)
    discard;

  diffuse_color.rgb *= diffuse_color.a * obj_data.Color.rgb;

  const float3 ambient_color = diffuse_color.rgb;
  float3 color = ambient_color;

  output.color.xyz = color;
  output.color.a = 1.0f;
  output.normal.xyz = normal;
  output.normal.a = 1.0f;
  output.position = input.position_world;

  return output;
}
