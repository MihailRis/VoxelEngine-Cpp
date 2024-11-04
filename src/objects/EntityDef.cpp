#include "EntityDef.hpp"


void EntityDef::cloneTo(EntityDef& dst) {
    dst.components = components;
    dst.bodyType = bodyType;
    dst.hitbox = hitbox;
    dst.boxSensors = boxSensors;
    dst.radialSensors = radialSensors;
    dst.skeletonName = skeletonName;
    dst.blocking = blocking;
    dst.save = save;
}
