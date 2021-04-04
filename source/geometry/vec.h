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
  float const& operator[](int) const
  {
    return x;
  }
  float& operator[](int)
  {
    return x;
  }
};

template<>
struct Vec<2> : float2
{
  float const& operator[](int i) const
  {
    return (&x)[i];
  }
  float& operator[](int i)
  {
    return (&x)[i];
  }
};

template<>
struct Vec<3> : float3
{
  float const& operator[](int i) const
  {
    return (&x)[i];
  }
  float& operator[](int i)
  {
    return (&x)[i];
  }
};

template<>
struct Vec<4> : float4
{
  float const& operator[](int i) const
  {
    return (&x)[i];
  }
  float& operator[](int i)
  {
    return (&x)[i];
  }
};

}