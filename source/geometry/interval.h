#pragma once

#include <vector>

#include "ray.h"
#include "vec.h"


namespace CS350
{

template<int Dim>
struct Slab
{
  Slab(Vec<Dim> N, float d0, float d1)
    : N{ N }, d0{ d0 }, d1{ d1 }
  {}
  Slab() = default;
  Vec<Dim> N;
  float d0, d1;
};

template<int Dim>
class Interval
{
public:
  Interval() noexcept
    : t0(0),
    t1{ std::numeric_limits<float>::max() }
  {}

  Interval(float t0, float t1)
    : t0(t0),
    t1(t1)
  {}

  static Interval empty()
  {
    return Interval(0.0f, -1.0f);
  }

  static Interval intersect(Interval const& a, Interval const& b)
  {
    return Interval(std::max(a.t0, b.t0), std::min(a.t1, b.t1));
  }

  static Interval intersect(Ray<Dim> const& r, std::vector<Slab<Dim>> const& slabs)
  {
    Interval i;
    for (Slab const& s : slabs)
    {
      i = intersect(i, s.intersect(r));
    }
    return i;
  }

  float t0, t1;
};

template<int Dim>
Interval<Dim> intersect(Slab<Dim> const& slab, Ray<Dim> const& ray)
{
  // Ray intersects both slab planes
  if (slab.N.dot(ray.dir) != 0.0f)
  {
    float const t0 = -(slab.d0 + slab.N.dot(ray.begin)) / slab.N.dot(ray.dir);
    float const t1 = -(slab.d1 + slab.N.dot(ray.begin)) / slab.N.dot(ray.dir);
    if (t0 < t1)
      return Interval(t0, t1, -slab.N, slab.N);
    else
      return Interval(t1, t0, slab.N, -slab.N);
  }
  // ray is parallel to slab planes
  else
  {
    float const s0 = slab.N.dot(ray.begin) + slab.d0;
    float const s1 = slab.N.dot(ray.begin) + slab.d1;
    if (signbit(s0) != signbit(s1))
    {
      // ray is fully inside the slab
      return Interval(0, std::numeric_limits<float>::max(), {}, {});
    }
    else
    {
      // ray is fully outside the slab
      return Interval<Dim>::empty();
    }
  }
}

}
