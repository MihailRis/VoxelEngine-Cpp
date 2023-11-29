#ifndef MATHS_AABB_H_
#define MATHS_AABB_H_

#include <glm/glm.hpp>

// Axis Aligned Bounding Box
struct AABB {
	glm::vec3 a;
    glm::vec3 b;

    inline glm::vec3 min() const {
        return glm::min(a, b);
    }

    inline glm::vec3 size() const {
        return glm::vec3(
            fabs(b.x - a.x),
            fabs(b.y - a.y),
            fabs(b.z - a.z)
        );
    }

    inline bool inside(const glm::vec3 pos) const {
        const glm::vec3 p = min();
        const glm::vec3 s = size();
        return !(pos.x < p.x || pos.y < p.y || pos.z < p.z ||
                 pos.x >= p.x+s.x || pos.y >= p.y+s.y || pos.z >= p.z+s.z);
    }
};

#endif // MATHS_AABB_H_