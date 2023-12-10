#ifndef MATHS_RAYS_H_
#define MATHS_RAYS_H_

// #include "../typedefs.h"
#include "aabb.h"
#include "glm/glm.hpp"

#include <array>
#include <unordered_map>

typedef glm::highp_dvec3 rayvec3;
typedef glm::highp_dvec2 rayvec2;
typedef double scalar_t;

enum class RayRelation{
    Embed=2, Intersect=1, Parallel=0, None=0
};
enum class AAFaceKind : unsigned char{
	Xperp=0, Yperp=1, Zperp=2 //perpendicular faces to corresponding axis
};

const unsigned char AABBFACES_COUNT = 6;

class AABBFaces{
public:
	std::array<std::pair<rayvec3, rayvec2>, AABBFACES_COUNT> faces; // every face is min-point and opposite corner point

	static constexpr std::array<AAFaceKind,AABBFACES_COUNT> KINDS_ORDER = {
									AAFaceKind::Xperp, AAFaceKind::Xperp,
									AAFaceKind::Yperp, AAFaceKind::Yperp,
									AAFaceKind::Zperp, AAFaceKind::Zperp};

	AABBFaces(){};
	AABBFaces(const rayvec3& parentBoxPos, const AABB& parentBox);

};

template<>
struct std::hash<rayvec3>{
	std::size_t operator()(const rayvec3& r) const noexcept{
		return std::hash<scalar_t>{}(r.x) ^ (std::hash<scalar_t>{}(r.y) << 1) ^ (std::hash<scalar_t>{}(r.z) << 2);
	}
};

class Rays{
protected:
	static const bool IS_RAYS_BOX_CACHE_ON = true;
	static std::unordered_map<rayvec3, AABBFaces> raysBoxCache_; //[boxPos]: faces array 

public:

//optimized, NOT returns intersectPoint coordinates and normal vector
template <AAFaceKind faceKind>
static RayRelation isRayIntersectsAAFace(
				   const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner
				);

//returns only normal
template <AAFaceKind faceKind>
static RayRelation rayIntersectAAFace( 
	               const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner,
				   glm::ivec3& normal_ret
);

//returns normal and distance
template <AAFaceKind faceKind>
static RayRelation rayIntersectAAFace( 
	               const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner,
				   glm::ivec3& normal_ret,
				   scalar_t& distance_ret
				   );

// returns normal, distance and intersection point
template <AAFaceKind faceKind>
static RayRelation rayIntersectAAFace( 
				const rayvec3& rayOrigin,
				const rayvec3& rayDir,
				const rayvec3& faceMin,
				const rayvec2& faceOppositeCorner,
				glm::ivec3& normal_ret,
				scalar_t& distance_ret,
				rayvec3& intersectPoint_ret
);

static RayRelation rayIntersectAABB(
	               const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
                   const rayvec3& boxPos,
				   const AABB& box,
				   float maxDist,
                   glm::ivec3& normal_ret,
				   scalar_t& distance_ret);

static RayRelation rayIntersectAABBFaces( // calculates only normal and distance
                   const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
                   const AABBFaces& boxFaces,
				   float maxDist,
                   glm::ivec3& normal_ret,
				   scalar_t& distance_ret);
};

#endif // SRC_VOXNATHS_H_
