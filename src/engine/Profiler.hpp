#pragma once

#if defined(VOXELENGINE_PROFILER)
#if defined(VOXELENGINE_TRACY_PROFILER)
// #include <GL/glew.h>  // Need for TracyOpenGL.hpp

#include <Tracy/Tracy.hpp>

#define VOXELENGINE_PROFILE ZoneScoped
#define VOXELENGINE_PROFILE_FRAME FrameMark
#else
#error "No active profiler"
#endif
// You can implement any your profiler with adding VOXELENGINE_{NAME}_PROFILER
// macros
#else
#define VOXELENGINE_PROFILE
#define VOXELENGINE_PROFILE_FRAME
#endif