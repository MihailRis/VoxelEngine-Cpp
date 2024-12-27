#pragma once

#if defined(VOXELENGINE_PROFILER)
#if defined(VOXELENGINE_TRACY_PROFILER)
#include <Tracy/Tracy.hpp>

#define VOXELENGINE_PROFILE ZoneScoped
#define VOXELENGINE_PROFILE_FRAME FrameMark
#define VOXELENGINE_PROFILE_SECTION(name) ZoneScopedN(name)
#define VOXELENGINE_PROFILE_TAG(tag) ZoneText(tag, strlen(tag))
#define VOXELENGINE_PROFILE_LOG(text) TracyMessage(text, strlen(text))
#define VOXELENGINE_PROFILE_VALUE(text, value) TracyPlot(text, value)
/// @warning Tracy can capture only one context
#define VOXELENGINE_GPU_CONTEXT TracyGpuContext
#define VOXELENGINE_GPU_ZONE(name) TracyGpuZone(name)
#else
#error "No active profiler"
#endif
// You can implement any your profiler with adding VOXELENGINE_{NAME}_PROFILER
// macros
#else
#define VOXELENGINE_PROFILE
#define VOXELENGINE_PROFILE_FRAME
#define VOXELENGINE_PROFILE_SECTION(name)
#define VOXELENGINE_PROFILE_TAG(tag)
#define VOXELENGINE_PROFILE_LOG(text)
#define VOXELENGINE_PROFILE_VALUE(text, value)
#define VOXELENGINE_GPU_CONTEXT
#define VOXELENGINE_GPU_ZONE(name)
#endif