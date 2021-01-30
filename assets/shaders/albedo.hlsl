cbuffer cb_per_object : register(b0)
{
  float4x4 World;
  float4x4 WorldNormal;
  float4   ObjectColor;
};

cbuffer cb_per_frame : register(b1)
{
  float4x4 ViewProjection;
  float4   CameraPosition;
  float4   LightPosition;
}

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
};

vs_out vs_main(vs_in input)
{
  vs_out output;
  float4 position_world = mul(World, float4(input.position_local, 1.0f));
  output.position_clip = mul(ViewProjection, position_world);
  return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
  return ObjectColor; // must return an RGBA color
}
