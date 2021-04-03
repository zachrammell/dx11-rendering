#ifndef PER_FRAME_H
#define PER_FRAME_H

#include "structure_system.h"

STRUCT(per_frame)
  FIELD(View, float4x4);
  FIELD(Projection, float4x4);
  FIELD(CameraPosition, float4);
  FIELD(AmbientColor, float4);
ENDSTRUCT;

#endif
