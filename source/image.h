#pragma once
#include <cstdint>

namespace IE
{

struct pixel_rgb
{
  uint8_t r, g, b, a;
};

class Image
{
public:
  explicit Image(char const* image_path);
  ~Image();
  pixel_rgb const* GetData() const;
  int GetWidth() const;
  int GetHeight() const;

  int GetPitch() const;
private:
  int width, height, channel_count = 4;
  pixel_rgb* image_data;
};

}
