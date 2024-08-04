#ifndef MATHS_RAYS_HPP_
#define MATHS_RAYS_HPP_

#include "aabb.hpp"
#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <array>

#include "glm/gtx/hash.hpp"

using rayvec3 = glm::highp_dvec3;
using rayvec2 = glm::highp_dvec2;
using scalar_t = double;

enum class RayRelation { Embed = 2, Intersect = 1, Parallel = 0, None = 0 };

class AABBFaces {
    static const unsigned char AABBFACES_COUNT = 6;
public:
    std::array<std::pair<rayvec3, rayvec2>, AABBFACES_COUNT>
        faces;  // every face is min-point and opposite corner point

    AABBFaces() = default;
    AABBFaces(const rayvec3& parentBoxPos, const AABB& parentBox);
};

class Ray {
public:
    rayvec3 origin;
    rayvec3 dir;

    Ray(const rayvec3& rayOrigin, const rayvec3& rayDir);

    RayRelation isIntersectsYZFace(
        const rayvec3& faceMin, const rayvec2& faceOppositeCorner
    );
    RayRelation isIntersectsXZFace(
        const rayvec3& faceMin, const rayvec2& faceOppositeCorner
    );
    RayRelation isIntersectsXYFace(
        const rayvec3& faceMin, const rayvec2& faceOppositeCorner
    );

    /// returns normal and distance
    RayRelation intersectYZFace(
        const rayvec3& faceMin,
        const rayvec2& faceOppositeCorner,
        glm::ivec3& normal_ret,
        scalar_t& distance_ret
    );
    RayRelation intersectXZFace(
        const rayvec3& faceMin,
        const rayvec2& faceOppositeCorner,
        glm::ivec3& normal_ret,
        scalar_t& distance_ret
    );
    RayRelation intersectXYFace(
        const rayvec3& faceMin,
        const rayvec2& faceOppositeCorner,
        glm::ivec3& normal_ret,
        scalar_t& distance_ret
    );

    RayRelation intersectAABB(
        const rayvec3& boxPos,
        const AABB& box,
        float maxDist,
        glm::ivec3& normal_ret,
        scalar_t& distance_ret
    );

    RayRelation intersectAABBFaces(  // calculates only normal and distance
        const AABBFaces& boxFaces,
        float maxDist,
        glm::ivec3& normal_ret,
        scalar_t& distance_ret
    );
};

#endif  // SRC_VOXNATHS_HPP_
