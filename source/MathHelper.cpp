#include "MathHelper.h"

namespace dx = DirectX;

namespace IE
{

DirectX::XMMATRIX InverseTranspose(dx::XMMATRIX const mtx)
{
  return dx::XMMatrixTranspose(dx::XMMatrixInverse(nullptr, mtx));
}

float InverseLerp(float a, float b, float v)
{
  return (v - a) / (b - a);
}

}
