cbuffer cb_per_frame : register(b1)
{
  float4x4 ViewProjection;
  float4   CameraPosition;
  float4   LightPosition;
}

TextureCube TextureSkymap : register(t0);
SamplerState Sampler : register(s0);

struct vs_in
{
  float3 position_local : POS;
  float3 normal : NOR;
  float2 uv : TEX;
};

struct vs_out
{
  float4 position_clip : SV_POSITION; // required output of VS
  float3 texCoord : TEXCOORD0;
};

vs_out vs_main(vs_in input)
{
  vs_out output;
  output.position_clip = mul(ViewProjection, float4(input.position_local, 0.0f));
  output.position_clip.z = 1.0f;
  output.texCoord = input.position_local;
  return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
  float3 uv = input.texCoord;
  return TextureSkymap.Sample(Sampler, uv);
}
