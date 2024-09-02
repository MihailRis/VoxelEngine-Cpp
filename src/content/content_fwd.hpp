#pragma once

#include "typedefs.hpp"

class Content;
class ContentPackRuntime;

enum class ContentType { NONE, BLOCK, ITEM, ENTITY };

enum class ResourceType : size_t { CAMERA, LAST = CAMERA };

inline constexpr auto RESOURCE_TYPES_COUNT =
    static_cast<size_t>(ResourceType::LAST) + 1;
