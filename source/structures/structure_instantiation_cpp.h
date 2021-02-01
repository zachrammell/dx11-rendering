#ifndef STRUCTURE_INSTANTIATION_CPP_H
#define STRUCTURE_INSTANTIATION_CPP_H

#include "types/types_cpp.h"

#define EXPAND(x) x

#define STRUCT(x) \
__pragma(pack(push, 1)) \
struct x {
#define ENDSTRUCT \
__pragma(pack(pop)) \
}

#define FIELD(name, type) EXPAND(type) name

#endif
