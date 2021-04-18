#include "math_helper.h"

namespace CS350
{

DirectX::XMMATRIX InverseTranspose(DirectX::XMMATRIX const mtx)
{
  return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, mtx));
}

float inverseLerp(float a, float b, float v)
{
  return (v - a) / (b - a);
}

}
