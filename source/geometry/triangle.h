#pragma once

#include "vec.h"
#include "ray.h"

namespace CS350
{

template<int Dim>
struct Triangle
{
  Vec<Dim> v0, v1, v2;

  bool intersect(Ray<Dim> const& ray) const
  {
    Vec<Dim> e1 = v1 - v0;
    Vec<Dim> e2 = v2 - v0;
    Vec<Dim> p = ray.dir.cross(e2);
    float d = p.dot(e1);

    // Ray is parallel to triangle
    if (d == 0.0f) return false;

    Vec<Dim> s = ray.begin - v0;
    float u = (p.dot(s)) / d;

    // Ray intersects plane, but outside E2 edge
    if (u < 0 || u > 1) return false;

    Vec<Dim> q = s.cross(e1);
    float v = (ray.dir.dot(q)) / d;

    // Ray intersects plane, but outside other edges
    if (v < 0 || (u + v) > 1) return false;

    float t = e2.dot(q) / d;

    // Ray's negative half intersects triangle
    if (t < 0) return false;

    //Intersection intersection;
    //intersection.t = t;
    //intersection.position = r.eval(t);
    //intersection.normal = -e2.cross(e1).normalized();
    //intersection.object = this;

    return true;
  }

  float area() const
  {
    return fabsf((v0.x * (v1.y - v2.y) + v1.x * (v2.y - v0.y) + v2.x * (v0.y - v1.y)) / 2.0f);
  }

  bool contains(Point<Dim> const& p) const
  {
    /* Calculate area of triangle ABC */
    float A = area();

    /* Calculate area of triangle PBC */
    float A1 = Triangle(p, v1, v2).area();

    /* Calculate area of triangle PAC */
    float A2 = Triangle(p, v0, v2).area();

    /* Calculate area of triangle PAB */
    float A3 = Triangle(p, v0, v1).area();

    /* Check if sum of A1, A2 and A3 is same as A */
    return (A1 + A2 + A3 - A > FLT_EPSILON * 4);
  }
};

}