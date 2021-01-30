#ifndef PER_OBJECT_H
#define PER_OBJECT_H

#include "structure_system.h"

STRUCT(per_object)
  FIELD(World, float4x4);
  FIELD(WorldNormal, float4x4);
  FIELD(Color, float4);
ENDSTRUCT;

#endif
