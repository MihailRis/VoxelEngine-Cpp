#ifndef CONTENT_CONTENT_FWD_HPP_
#define CONTENT_CONTENT_FWD_HPP_

#include "typedefs.hpp"

class Content;
class ContentPackRuntime;

enum class contenttype { none, block, item, entity };

enum class ResourceType : size_t { CAMERA, LAST = CAMERA };

inline constexpr auto RESOURCE_TYPES_COUNT =
    static_cast<size_t>(ResourceType::LAST) + 1;

#endif  // CONTENT_CONTENT_FWD_HPP_
