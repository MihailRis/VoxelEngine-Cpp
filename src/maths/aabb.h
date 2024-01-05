#ifndef MATHS_AABB_H_
#define MATHS_AABB_H_

#include <glm/glm.hpp>

// Axis Aligned Bounding Box
struct AABB {
	glm::vec3 a {0.0f};
    glm::vec3 b {1.0f};

    /* Get AABB point with minimal x,y,z */
    inline glm::vec3 min() const {
        return glm::min(a, b);
    }

    /* Get AABB point with minimal x,y,z */
    inline glm::vec3 max() const {
        return glm::max(a, b);
    }

    /* Get AABB dimensions: width, height, depth */
    inline glm::vec3 size() const {
        return glm::vec3(
            fabs(b.x - a.x),
            fabs(b.y - a.y),
            fabs(b.z - a.z)
        );
    }

    inline glm::vec3 center() const {
        return (a + b) * 0.5f;
    }

    /* Multiply AABB size from center */
    inline void scale(const glm::vec3 mul) {
        glm::vec3 center = (a + b) * 0.5f;
        a = (a - center) * mul + center;
        b = (b - center) * mul + center;
    }

    /* Multiply AABB size from given origin */
    inline void scale(const glm::vec3 mul, const glm::vec3 orig) {
        glm::vec3 beg = min();
        glm::vec3 end = max();
        glm::vec3 center = glm::mix(beg, end, orig);
        a = (a - center) * mul + center;
        b = (b - center) * mul + center;
    }

    /* Check if given point is inside */
    inline bool contains(const glm::vec3 pos) const {
        const glm::vec3 p = min();
        const glm::vec3 s = size();
        return !(pos.x < p.x || pos.y < p.y || pos.z < p.z ||
                 pos.x >= p.x+s.x || pos.y >= p.y+s.y || pos.z >= p.z+s.z);
    }
};

#endif // MATHS_AABB_H_