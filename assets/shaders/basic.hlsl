cbuffer cb_per_object
{
  float4x4 WVP;
};

/* vertex attributes go here to input to the vertex shader */
struct vs_in
{
  float3 position_local : POS;
  float3 color : COL;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out
{
  float4 position_clip : SV_POSITION; // required output of VS
  float3 color : COLOR0;
};

vs_out vs_main(vs_in input)
{
  vs_out output;
  output.position_clip = mul(float4(input.position_local, 1.0f), WVP);
  output.color = input.color;
  return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
  return float4(input.color, 1.0); // must return an RGBA color
}
