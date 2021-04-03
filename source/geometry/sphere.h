#pragma once

#include "point.h"
#include "ray.h"
#include "box.h"

namespace CS350
{

template<int Dim>
struct Sphere
{
  Point<Dim> center;
  float radius;

  bool contains(Point<Dim> const& p) const
  {
    float const d = p - center;
    return (d * d) <= (radius * radius);
  }

  bool intersects(Sphere<Dim> other) const
  {
    return Sphere<Dim>(center, radius + other.radius).contains(other.center);
  }

  bool intersects(Ray<Dim> ray) const
  {
    Vec<Dim> const Qdir = ray.begin - center;
    float QdD = Qdir.dot(ray.dir);
    float QdQ = Qdir.dot(Qdir);
    float discriminant = QdD * QdD - QdQ + radius * radius;

    if (discriminant < 0) return false;

    discriminant = sqrt(discriminant);
    float t0 = -QdD + discriminant;
    float t1 = -QdD - discriminant;

    if (t0 < 0 && t1 < 0) return false;

    //Intersection intersection;
    //intersection.t = std::min(t0, t1);
    //intersection.position = r.eval(intersection.t);
    //intersection.normal = (intersection.position - center_).normalized();
    //intersection.object = this;

    return true;
  }

  bool intersects(Box<Dim> const& box) const
  {
    return box.intersects(*this);
  }
};

}
