#include "Image.h"

#include <cassert>
#include "stb_image.h"

namespace IE
{

Image::Image(char const* image_path)
{
  image_data = reinterpret_cast<pixel_rgb*>(stbi_load(image_path, &width, &height, &channel_count, channel_count));
  assert(channel_count == 4);
}

Image::~Image()
{
  stbi_image_free(image_data);
}

pixel_rgb const* Image::GetData() const
{
  return image_data;
}

int Image::GetWidth() const
{
  return width;
}

int Image::GetHeight() const
{
  return height;
}

int Image::GetPitch() const
{
  return GetWidth() * (int)sizeof(pixel_rgb);
}

}
