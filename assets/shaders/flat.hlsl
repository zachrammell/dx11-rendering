






























struct per_object { 
  float4x4 World ;
  float4x4 WorldNormal ;
  float4 Color ;
} ;




cbuffer cb_per_object : register(b0)
{
  per_object obj_data;
}

Texture2D TextureDiffuse : register(t0);

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
  float2 uv : TEXCOORD0;
  float3 normal : COLOR0;
};

vs_out vs_main(vs_in input)
{
  vs_out output;
  output.position_clip = mul(obj_data.World, float4(input.position_local, 1.0f));
  output.normal = mul(obj_data.WorldNormal, float4(input.normal, 0.0f));
  output.uv = input.tex_coord;
  return output;
}

struct ps_out
{
  float4 color : SV_TARGET0;
};

ps_out ps_main(vs_out input) : SV_TARGET
{
  ps_out output;

  float2 uv = input.uv;
  float4 diffuse_color = TextureDiffuse.Sample(Sampler, uv);
  if (diffuse_color.a < 0.001f)
    discard;

  diffuse_color.rgb *= obj_data.Color.rgb;

  const float3 ambient_color = diffuse_color.rgb;
  float3 color = ambient_color;

  output.color.xyz = color;
  output.color.a = 1.0f;

  return output;
}
