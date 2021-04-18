#pragma once

#include "vec.h"
#include "triangle.h"
#include "ray.h"

#include "../math_helper.h"

#include <algorithm>

namespace CS350
{

template<int Dim>
struct Plane
{
  Vec<Dim> normal;
  float d;

  static constexpr float plane_epsilon = 10e-4f;

  float distanceFrom(Vec<Dim> const& point) const
  {
    return dot(normal, point) + d;
  }

  void intersectSegment(Vec<Dim> const& v1, Vec<Dim> const& v2, std::vector<Vec<Dim>>& segments) const
  {
    float distv1 = distanceFrom(v1),
          distv2 = distanceFrom(v2);

    bool bP1OnPlane = (abs(distv1) < plane_epsilon),
         bP2OnPlane = (abs(distv2) < plane_epsilon);

    if (bP1OnPlane)
      segments.push_back(v1);

    if (bP2OnPlane)
      segments.push_back(v2);

    if (bP1OnPlane && bP2OnPlane)
      return;

    if (distv1 * distv2 > 0)  // points on the same side of plane
      return;

    auto x = normalize(v2 - v1);

    segments.push_back(v2 - x * (distv2 / dot(normal, x)));
  }

  void intersectTriangle(Triangle<Dim> tri, std::vector<Vec<Dim>>& intersection_points)
  {
    intersectSegment(tri.v0, tri.v1, intersection_points);
    intersectSegment(tri.v1, tri.v2, intersection_points);
    intersectSegment(tri.v2, tri.v0, intersection_points);

    // remove duplicates
    auto& vec = intersection_points;
    std::sort(vec.begin(), vec.end(), [](Vec<Dim> const& a, Vec<Dim> const& b)
    {
      return (a.x < b.x) && (a.y < b.y) && (a.z < b.z);
    });
    vec.erase(std::unique(vec.begin(), vec.end(), [](Vec<Dim> const& a, Vec<Dim> const& b)
    {
      auto d = (a - b);
      return (abs(d.x) < plane_epsilon) && (abs(d.y) < plane_epsilon) && (abs(d.z) < plane_epsilon);
    }), vec.end());

    //assert(intersection_points.size() <= 2);
  }

  bool contains(Vec<Dim> const& point) const
  {
    Vec<Dim + 1> P = normal;
    P[Dim] = d;

    Vec<Dim + 1> Q = point;
    Q[Dim] = 1;

    return fabsf(dot(P, Q)) < FLT_EPSILON * 4;
  }

  bool intersects(class Box<Dim> const& box) const
  {
    return false;
  }

  bool intersects(class Sphere<Dim> const& sphere) const
  {
    return false;
  }

  bool intersects(Ray<Dim> const& ray) const
  {
    float const denom = dot(normal, ray.dir);
    if (fabsf(denom) > plane_epsilon)
    {
      float t = dot(normal, (normal / d) - ray.origin) / denom;
      if (t >= plane_epsilon) return true;
    }
    return false;
  }

  std::optional<std::pair<float, Vec<Dim>>> intersection(Vec<Dim> const& v1, Vec<Dim> const& v2) const
  {
    std::vector<Vec<3>> intersects;

    intersectSegment(v1, v2, intersects);

    if (intersects.empty())
      return std::nullopt;

    float t = inverseLerp(v1, v2, intersects[0]);

    return std::make_pair(t, intersects[0]);
  }
};

}