#ifndef VOX_TYPEDEFS_H
#define VOX_TYPEDEFS_H

#include <stdlib.h>
#include <stdint.h>

using uint = unsigned int;

// use for bytes arrays
using ubyte = uint8_t;

// content indices
using itemid_t = uint32_t;
using blockid_t = uint16_t;

using itemcount_t = uint32_t;
using blockstate_t = uint16_t;
using light_t = uint16_t;

#endif
