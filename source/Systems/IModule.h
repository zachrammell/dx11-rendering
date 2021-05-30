#pragma once

#include "NonCopyable.h"

namespace IE::Systems
{

class IModule : NonCopyable
{
public:
  virtual ~IModule() = default;
  // Initialized once
  virtual void Initialize() = 0;

  virtual bool IsSystem()
  {
    return false;
  }
};

}