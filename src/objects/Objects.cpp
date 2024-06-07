#include "Objects.hpp"

void Objects::update(float delta) {
    for (const auto& entry : objects) {
        const auto& obj = entry.second;
        if (obj && obj->shouldUpdate) {
            obj->update(delta);
        }
    }
}
