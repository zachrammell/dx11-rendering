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
  Vec() = default;

  Vec(float3 const& r) : float3(r)
  {}

  operator float3() const
  {
    return *this;
  }

  float const& operator[](int i) const
  {
    return (&x)[i];
  }
  float& operator[](int i)
  {
    return (&x)[i];
  }
  Vec<3>& operator=(float3 const& f)
  {
    x = f.x;
    y = f.y;
    z = f.z;

    return *this;
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
