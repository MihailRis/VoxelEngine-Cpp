#pragma once

#include "typedefs.hpp"

class Content;
class ContentPackRuntime;

enum class contenttype { none, block, item, entity };

enum class ResourceType : size_t { CAMERA, LAST = CAMERA };

inline constexpr auto RESOURCE_TYPES_COUNT =
    static_cast<size_t>(ResourceType::LAST) + 1;
