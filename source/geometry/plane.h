#pragma once

#include "vec.h"
#include "box.h"

namespace CS350
{

template<int Dim>
struct Plane
{
  Vec<Dim> normal;
  float d;

  bool contains(Vec<Dim> const& point) const
  {
    Vec<Dim + 1> P = normal;
    P[Dim] = d;

    Vec<Dim + 1> Q = point;
    Q[Dim] = 1;

    return fabsf(dot(P, Q)) < FLT_EPSILON * 4;
  }

  bool intersects(Box<Dim> const& box) const
  {
    return false;
  }

  bool intersects(Sphere<Dim> const& sphere) const
  {
    return false;
  }

  bool intersects(Ray<Dim> const& ray) const
  {
    float const denom = dot(normal, ray.dir);
    if (fabsf(denom) > 0.0001f)
    {
      float t = dot(normal, (normal / d) - ray.origin) / denom;
      if (t >= 0.0001f) return true;
    }
    return false;
  }
};

}