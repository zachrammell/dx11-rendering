





























struct per_object {
  float4x4 World ;
  float4x4 WorldNormal ;
  float4 Color ;
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
  const float3 color_specular = normal_dot_light > 0.0f ? max(SpecularCoefficient * pow(reflection_dot_view, SpecularExponent) * light_color, float3(0.f, 0.f, 0.f)) : float3(0.f, 0.f, 0.f);
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

  light_contribution point_light = PointLight(make_point_light(in_light.color, in_light.position), fragment_position, view, normal, AttenuationConstants);

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

 
struct vs_in
{
  float3 position_local : POS;
  float3 normal : NOR;
  float2 tex_coord : TEX;
};

 
struct vs_out
{
  float4 position_clip : SV_POSITION; 
  float4 position_world : TEXCOORD0;
  float2 uv : TEXCOORD1;
  float3 normal : COLOR0;
};

vs_out vs_main(vs_in input)
{
  vs_out output;
  output.position_world = mul(World, float4(input.position_local, 1.0f));
  output.position_clip = mul(ViewProjection, output.position_world);
  output.normal = mul(WorldNormal, input.normal);
  output.uv = input.tex_coord;
  return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
  const float3 view = normalize(CameraPosition.xyz - input.position_world.xyz);
  const float3 normal = normalize(input.normal);

  float3 diffuse_color = ObjectColor.rgb;
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

  return float4(color, 1.0f); 
}
