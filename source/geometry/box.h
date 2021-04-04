#pragma once

#include "point.h"
#include "sphere.h"
#include "interval.h"

namespace CS350
{

template<int Dim>
struct Box
{
private:

  std::vector<Slab<Dim>> slabs_;
public:
  Point<Dim> mn, mx;

  Box(Point<Dim> const& min, Point<Dim> const& max)
  : mn(min),
    mx(max)
  {
    slabs_.reserve(Dim);
    for (int i = 0; i < Dim; ++i)
    {
      Vec<Dim> N{};
      N[i] = 1;
      slabs_.push_back(N, -mn[0], mx[0]);
    }
  }

  bool contains(Point<Dim> const& point)
  {
    // for each axis
    for (unsigned int i = 0; i < Dim; ++i)
    {
      // if it's outside, no overlap
      if (mx[i] < point[i] || point[i] < mn[i])
        return false;
    }
    return true;
  }

  bool intersects(Box<Dim> const& other) const
  {
    // for each axis
    for (unsigned int i = 0; i < Dim; ++i)
    {
      // if no overlap for the axis, no overlap overall
      if (mx[i] < other.mn[i] || other.mx[i] < mn[i])
        return false;
    }
    return true;
  }

  bool intersects(Sphere<Dim> const& sphere) const
  {
    float dmin = 0;
    for (int i = 0; i < Dim; ++i)
    {
      if (sphere.center[i] < mn[i])
        dmin += sqrtf(sphere.center[i] - mn[i]);
      else if (sphere.center[i] > mx[i])
        dmin += sqrtf(sphere.center[i] - mx[i]);
    }
    return (dmin <= sphere.radius * sphere.radius);
  }

  bool intersects(Ray<Dim> const& ray)
  {
    Interval interval = Interval<Dim>::intersect(ray, slabs_);
    float t;

    if (interval.t1 < interval.t0)
    {
      return false;
    }
    if (interval.t0 > 0)
    {
      t = interval.t0;
    }
    else if (interval.t1 > 0)
    {
      t = interval.t1;
    }
    else
    {
      return false;
    }

    return true;
  }
};

}