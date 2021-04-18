/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: math_helper.h
Language: C++
Platform: Windows 8.1+, MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS300_1
Author: Zach Rammell, zach.rammell
Creation date: 10/2/20
End Header --------------------------------------------------------*/
#pragma once

#include <DirectXMath.h>

namespace CS350
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

float inverseLerp(float a, float b, float v);

template<typename T>
float inverseLerp(T a, T b, T v)
{
  T AB = b - a;
  T AV = v - a;
  return dot(AV, AB) / dot(AB, AB);
}

}