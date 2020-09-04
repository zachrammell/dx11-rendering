#pragma once

namespace CS300
{

struct color_RGBA
{
  using raw_data_t = float[4];
  union
  {
    struct
    {
      float r, g, b, a;
    };
    raw_data_t data;
  };
};

namespace colors
{

constexpr color_RGBA black = { 0.0f, 0.0f, 0.0f, 1.0f };
constexpr color_RGBA white = { 1.0f, 1.0f, 1.0f, 1.0f };

}

}
