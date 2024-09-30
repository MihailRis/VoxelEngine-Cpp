#pragma once

#include "typedefs.hpp"

enum RegionLayerIndex : uint {
    REGION_LAYER_VOXELS = 0,
    REGION_LAYER_LIGHTS,
    REGION_LAYER_INVENTORIES,
    REGION_LAYER_ENTITIES,
    REGION_LAYER_BLOCKS_DATA,
    
    REGION_LAYERS_COUNT
};
