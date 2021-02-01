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
  // TODO: fix coefficients
  const float3 color_specular = normal_dot_light > 0.0f ? max(/*SpecularCoefficient*/0.5f * pow(reflection_dot_view, 16/*SpecularExponent*/) * light_color, float3(0.f, 0.f, 0.f)) : float3(0.f, 0.f, 0.f);
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
  // TODO: fix attenuation constants
  light_contribution point_light = PointLight(make_point_light(in_light.color, in_light.position), fragment_position, view, normal, float3(0,0,0)/*AttenuationConstants*/);

  if (cos_alpha > cos(in_light.inner_angle))
  {
    return point_light;
  }

  const float spotlight_effect = pow((cos_alpha - cos(in_light.outer_angle)) / (cos(in_light.inner_angle) - cos(in_light.outer_angle)), in_light.falloff);
  point_light.diffuse *= spotlight_effect;
  point_light.specular *= spotlight_effect;
  return point_light;
}