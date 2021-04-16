#pragma once

#include "vec.h"

namespace CS350
{

template<int Dim>
struct Ray
{
  Vec<Dim> begin, dir;

  Vec<Dim> eval(float t) const
  {
    return begin + (dir * t);
  }
};

}
