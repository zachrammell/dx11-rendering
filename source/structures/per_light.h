#ifndef PER_LIGHT_H
#define PER_LIGHT_H

#include "structure_system.h"

STRUCT(per_light)
  FIELD(Radius, float4);
ENDSTRUCT;

STRUCT(lights)
  FIELD_ARRAY(LightData, per_light, 16);
  FIELD(LightCount, int);
ENDSTRUCT;

#endif
