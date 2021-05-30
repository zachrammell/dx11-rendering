#pragma once

#include "IModule.h"

namespace IE::Systems
{

class ISystem : public IModule
{
public:
  // Updated once per frame
  virtual void Update() = 0;

  bool IsSystem() final override
  {
    return true;
  }
};

}
