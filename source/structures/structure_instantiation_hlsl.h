#ifndef STRUCTURE_INSTANTIATION_HLSL_H
#define STRUCTURE_INSTANTIATION_HLSL_H

#define EXPAND(x) x

#define STRUCT(x) struct x {
#define ENDSTRUCT }

#define FIELD(name, type) EXPAND(type) name

#endif
