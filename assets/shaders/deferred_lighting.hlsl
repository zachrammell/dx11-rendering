






























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









struct per_light {
  float4 Radius ;
} ;

struct lights {
  per_light LightData [16] ;
  int LightCount ;
} ;





static const int LT_POINT = 0;
static const int LT_DIRECTIONAL = 1;
static const int LT_SPOTLIGHT = 2;

struct point_light
{
  float3 color;
  float3 position;
};

point_light make_point_light(float3 color, float3 position)
{
  point_light p;
  p.color = color;
  p.position = position;
  return p;
}

struct directional_light
{
  float3 color;
  float3 direction;
};

directional_light make_directional_light(float3 color, float3 direction)
{
  directional_light d;
  d.color = color;
  d.direction = direction;
  return d;
}

struct spot_light
{
  float3 color;
  float3 position;
  float3 direction;
  float inner_angle;
  float outer_angle;
  float falloff;
};

spot_light make_spot_light(float3 color, float3 position, float3 direction, float inner_angle, float outer_angle, float falloff)
{
  spot_light s;
  s.color = color;
  s.position = position;
  s.direction = direction;
  s.inner_angle = inner_angle;
  s.outer_angle = outer_angle;
  s.falloff = falloff;
  return s;
}

struct light_contribution
{
  float3 diffuse;
  float3 specular;
};

light_contribution make_light_contribution(float3 diffuse, float3 specular)
{
  light_contribution lc;
  lc.diffuse = diffuse;
  lc.specular = specular;
  return lc;
}

float3 Fog(float3 original_color, float3 fog_color, float view_length, float near, float far)
{
  float s = (far - view_length) / (far - near);
  return s * original_color + (1 - s) * fog_color;
}

float3 Diffuse(float3 light, float3 normal, float3 light_color)
{
  const float normal_dot_light = max(dot(normal, light), 0.0f);
  const float3 color_diffuse = normal_dot_light * light_color;
  return color_diffuse;
}

float3 Phong(float3 light, float3 normal, float3 light_color, float3 view)
{
  const float normal_dot_light = max(dot(normal, light), 0.0f);
  const float3 reflection = normalize(2.0f * normal_dot_light * normal - light);
  const float reflection_dot_view = saturate(dot(reflection, view));
  
  const float3 color_specular = normal_dot_light > 0.0f ? max( 0.5f * pow(reflection_dot_view, 16 ) * light_color, float3(0.f, 0.f, 0.f)) : float3(0.f, 0.f, 0.f);
  return color_specular;
}

light_contribution PointLight(point_light in_light, float3 fragment_position, float3 view, float3 normal, float3 LightAttenuationConstants)
{
  float3 light = in_light.position - fragment_position;
  const float light_len = length(light);
  light /= light_len;
  float attenuation = min(1.0f / (LightAttenuationConstants.x + LightAttenuationConstants.y * light_len + LightAttenuationConstants.z * light_len * light_len), 1.0f);
  const float3 color_diffuse = attenuation * Diffuse(light, normal, in_light.color);
  const float3 color_specular = attenuation * Phong(light, normal, in_light.color, view);

  return make_light_contribution(color_diffuse, color_specular);
}

light_contribution DirectionalLight(directional_light in_light, float3 view, float3 normal)
{
  const float3 light = in_light.direction;
  const float3 color_diffuse = Diffuse(light, normal, in_light.color);
  const float3 color_specular = Phong(light, normal, in_light.color, view);

  return make_light_contribution(color_diffuse, color_specular);
}

light_contribution SpotLight(spot_light in_light, float3 fragment_position, float3 view, float3 normal)
{
  const float3 light = normalize(in_light.position - fragment_position);
  const float cos_alpha = dot(light, normalize(in_light.direction));

  if (cos_alpha < cos(in_light.outer_angle))
  {
    return make_light_contribution(float3(0, 0, 0), float3(0, 0, 0));
  }
  
  light_contribution point_light = PointLight(make_point_light(in_light.color, in_light.position), fragment_position, view, normal, float3(0,0,0) );

  if (cos_alpha > cos(in_light.inner_angle))
  {
    return point_light;
  }

  const float spotlight_effect = pow((cos_alpha - cos(in_light.outer_angle)) / (cos(in_light.inner_angle) - cos(in_light.outer_angle)), in_light.falloff);
  point_light.diffuse *= spotlight_effect;
  point_light.specular *= spotlight_effect;
  return point_light;
}


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

 
struct vs_in
{
  uint id : SV_VERTEXID;
};

 
struct vs_out
{
  float4 position_clip : SV_POSITION; 
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
  
  {
    light_contribution lc = PointLight(
      make_point_light(white, frame_data.CameraPosition),
      frag_pos, view, normal, float3(0.5f, 0.5f, 0.5f)
      );
    intensity += lc.diffuse + lc.specular;
  }

  output.color = frame_data.AmbientColor + float4(intensity * diffuse_color.xyz, 1.0f);
  

  return output;
}
