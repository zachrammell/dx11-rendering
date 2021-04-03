#pragma once

#include "../structures/types/types_cpp.h"

namespace CS350
{

template<int Dim>
struct Vec;

template<>
struct Vec<1>
{
  float x;
  float operator[](int) const
  {
    return x;
  }
};

template<>
struct Vec<2> : float2
{
  float operator[](int i) const
  {
    return (&x)[i];
  }
};

template<>
struct Vec<3> : float3
{
  float operator[](int i) const
  {
    return (&x)[i];
  }
};

template<>
struct Vec<4> : float4
{
  float operator[](int i) const
  {
    return (&x)[i];
  }
};

}
