#include "../structures/per_object.h"
#include "../structures/per_frame.h"

cbuffer cb_per_object : register(b0)
{
  per_object obj_data;
}

cbuffer cb_per_frame : register(b1)
{
  per_frame frame_data;
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
  const float4 position_world = mul(obj_data.World, float4(input.position_local, 1.0f));
  const float4 position_view = mul(frame_data.View, position_world);
  output.position_clip = mul(frame_data.Projection, position_view);
  const float4 normal_world = mul(obj_data.World, float4(input.normal, 1.0f));
  const float4 normal_view = mul(frame_data.View, normal_world);
  output.normal_clip = mul(frame_data.Projection, normal_view);
  return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
  return obj_data.Color; // must return an RGBA color
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
