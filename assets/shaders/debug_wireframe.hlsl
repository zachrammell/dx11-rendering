






























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

 
struct vs_in
{
  float3 position_local : POS;
  float3 unused0 : NOR;
  float2 unused1 : TEX;
};

 
struct vs_out
{
  float4 position_clip : SV_POSITION; 
};

vs_out vs_main(vs_in input)
{
  vs_out output;
  const float4 position_world = mul(obj_data.World, float4(input.position_local, 1.0f));
  const float4 position_view = mul(frame_data.View, position_world);
  output.position_clip = mul(frame_data.Projection, position_view);
  return output;
}

struct ps_out
{
  float4 color : SV_TARGET0;
};

ps_out ps_main(vs_out input) : SV_TARGET
{
  ps_out output;

  output.color.xyz = obj_data.Color.xyz;
  output.color.a = 1.0f;

  return output;
}
