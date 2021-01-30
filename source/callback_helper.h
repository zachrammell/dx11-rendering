/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: callback_helper.h
Purpose: callback adapter
Language: C++
Platform: MSVC v142, DirectX 11 compatible graphics hardware
Project: zach.rammell_CS300_1
Author: Zach Rammell, zach.rammell
Creation date: 10/2/20
End Header --------------------------------------------------------*/
#pragma once

#include <cassert>

namespace CS350
{

// Base template that can accept one argument (function pointer type)

template<class BoundClass, int UID, typename FunctionSignature>
class CallbackHelper;

// Specialized template that will extract the information out of the function pointer type

template<class BoundClass, int UID, typename ReturnType, typename... ArgTypes>
class CallbackHelper<BoundClass, UID, ReturnType(*)(ArgTypes...)>
{
public:
  using callback_type = ReturnType(*)(ArgTypes...);
  // Type of the corresponding method in BoundClass which has the same signature
  using method_type = ReturnType(BoundClass::*)(ArgTypes...);

  // Take a `this` pointer and a method pointer to bind
  CallbackHelper(BoundClass* instance, method_type method)
  {
    if (instance_ || method_)
    {
      assert(!"Overwriting callback helper state! Use a different UID.");
    }
    instance_ = instance;
    method_ = method;
  }

  // A callback which can be called like a non-member function
  static ReturnType callback(ArgTypes... args)
  {
    return (instance_->*method_)(args...);
  }

private:
  static inline BoundClass* instance_ = nullptr;
  static inline method_type method_ = nullptr;
};

}
