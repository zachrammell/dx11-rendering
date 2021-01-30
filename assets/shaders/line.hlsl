cbuffer cb_per_object
{
  float4x4 World;
  float4x4 WorldNormal;
  float4   ObjectColor;
};

cbuffer cb_per_frame
{
  float4x4 ViewProjection;
}

/* vertex attributes go here to input to the vertex shader */
struct vs_in
{
  float3 position_local : POS;
  float3 normal : NOR;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out
{
  float4 position_clip : SV_POSITION; // required output of VS
  float4 normal_clip : NORMAL0;
};

vs_out vs_main(vs_in input)
{
  vs_out output;
  float4 position_world = mul(World, float4(input.position_local, 1.0f));
  output.position_clip = mul(ViewProjection, position_world);
  float4 normal_world = mul(World, float4(input.normal, 1.0f));
  output.normal_clip = mul(ViewProjection, normal_world);
  return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
  return ObjectColor; // must return an RGBA color
}

[maxvertexcount(2)]
void gs_main(point vs_out input[1], inout LineStream<vs_out> stream_out)
{
  vs_out output = (vs_out)0;
  output.position_clip = input[0].position_clip + 0.05f * input[0].normal_clip;

  stream_out.Append(input[0]);
  stream_out.Append(output);

  stream_out.RestartStrip();
}
