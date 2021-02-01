#ifndef PER_FRAME_H
#define PER_FRAME_H

#include "structure_system.h"

STRUCT(per_frame)
  FIELD(ViewProjection, float4x4);
  FIELD(CameraPosition, float4);
  FIELD(AmbientColor, float4);
ENDSTRUCT;

#endif
