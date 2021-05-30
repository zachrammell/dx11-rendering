#pragma once

#include <DirectXMath.h>

namespace IE
{

DirectX::XMMATRIX InverseTranspose(DirectX::XMMATRIX const mtx);

template<typename... T>
auto sum(T... args)
{
  return (args + ...);
}

template<typename... T>
auto avg(T... args)
{
  return sum(args...) / sizeof...(args);
}

float InverseLerp(float a, float b, float v);

template<typename T>
float InverseLerp(T a, T b, T v)
{
  T AB = b - a;
  T AV = v - a;
  return dot(AV, AB) / dot(AB, AB);
}

}