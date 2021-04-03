#pragma once

#include "vec.h"
#include "point.h"

namespace CS350
{

template<int Dim>
struct Ray
{
  Vec<Dim> begin, dir;

  Point<Dim> eval(float t) const
  {
    return begin + (dir * t);
  }
};

}
