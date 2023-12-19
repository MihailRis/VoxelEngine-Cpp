
#include "rays.h"
#include "aabb.h"

#include "glm/glm.hpp"

std::unordered_map<rayvec3, AABBFaces> Ray::raysBoxCache_ = {};
const rayvec3 X_AXIS = rayvec3(1,0,0), Y_AXIS = rayvec3(0,1,0), Z_AXIS = rayvec3(0,0,1);

Ray::Ray(const rayvec3& origin, const rayvec3& dir) : origin(origin), dir(dir) {}

//make faces from AABB
AABBFaces::AABBFaces(const rayvec3& parentBoxPos, const AABB& parentBox){
	rayvec3 pbMin = parentBox.min(), // every face is min-point and opposite corner point
			pbMax = parentBox.max(),
			pbRealPos = parentBoxPos + pbMin;
	rayvec2 yzMax = rayvec2(parentBoxPos.y + pbMax.y, parentBoxPos.z + pbMax.z ),
			xzMax = rayvec2(parentBoxPos.x + pbMax.x, parentBoxPos.z + pbMax.z ),
			xyMax = rayvec2(parentBoxPos.x + pbMax.x, parentBoxPos.y + pbMax.y );
	faces[0] = {pbRealPos, yzMax}; //in order of AABBFaces::KINDS_ORDER!

	faces[1] = {parentBoxPos + rayvec3(pbMax.x, pbMin.y, pbMin.z), yzMax};

	faces[2] = {pbRealPos, xzMax};

	faces[3] = {parentBoxPos + rayvec3(pbMin.x, pbMax.y, pbMin.z), xzMax};

	faces[4] = {pbRealPos, xyMax};

	faces[5] = {parentBoxPos + rayvec3(pbMin.x, pbMin.y, pbMax.z), xyMax};
}

template <>
RayRelation Ray::intersectAAFace<AAFaceKind::Xperp>( 
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner, //y and z global coords of opposite corner
				   glm::ivec3& normal_ret,
				   scalar_t& distance_ret //sinonym of rayCoef
				   ){
	if (fabs(glm::dot(dir, X_AXIS)) < 1.0E-8){ //precision
		return RayRelation::Parallel;
	}

	scalar_t rayCoef = (faceMin.x - origin.x) / (dir.x);// equivalent to distance if dir normalized
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = {faceMin.x,
						rayCoef*dir.y + origin.y,
						rayCoef*dir.z + origin.z};

	if (intersectPoint.y >= faceMin.y
	&& intersectPoint.y <= faceOppositeCorner[0]
	&& intersectPoint.z >= faceMin.z
	&& intersectPoint.z <= faceOppositeCorner[1]){
		distance_ret = rayCoef; // believe that dir normalized
		if (dir.x > 0) normal_ret = -X_AXIS;
		else normal_ret = X_AXIS;
		return RayRelation::Intersect; 
	} 
	return RayRelation::None;
}

template <>
RayRelation Ray::intersectAAFace<AAFaceKind::Yperp>(
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner, //x and z global coords of opposite corner
				   glm::ivec3& normal_ret,
				   scalar_t& distance_ret
				   ){
	if (fabs(glm::dot(dir, Y_AXIS)) < 1.0E-8){ //precision
		return RayRelation::Parallel;
	}

	scalar_t rayCoef = (faceMin.y - origin.y) / (dir.y);// equivalent to distance if dir normalized
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = { rayCoef *dir.x + origin.x,
						faceMin.y,
						rayCoef*dir.z + origin.z};

	if (intersectPoint.x >= faceMin.x  //Face-hit check
	&& intersectPoint.x <= faceOppositeCorner[0] 
	&& intersectPoint.z >= faceMin.z 
	&& intersectPoint.z <= faceOppositeCorner[1] ){
		distance_ret = rayCoef; // believe that dir normalized
		if (dir.y > 0) normal_ret = -Y_AXIS;
		else normal_ret = Y_AXIS;
		return RayRelation::Intersect;
	} 
	return RayRelation::None;
}

template <>
RayRelation Ray::intersectAAFace<AAFaceKind::Zperp>(
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner, //x and y global coords of opposite corner
				   glm::ivec3& normal_ret,
				   scalar_t& distance_ret
					){
	if (fabs(glm::dot(dir, Z_AXIS)) < 1.0E-8){ //precision
		return RayRelation::Parallel;
	}
	
	scalar_t rayCoef = (faceMin.z - origin.z) / (dir.z); // equivalent to distance if dir normalized
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = { rayCoef *dir.x + origin.x,
						rayCoef*dir.y + origin.y,
						faceMin.z};
		
	if (intersectPoint.x >= faceMin.x  //Face-hit check
	&& intersectPoint.x <= faceOppositeCorner[0] 
	&& intersectPoint.y >= faceMin.y  
	&& intersectPoint.y <= faceOppositeCorner[1]  ){
		distance_ret = rayCoef; // believe that dir normalized
		if (dir.z > 0) normal_ret = -Z_AXIS;
		else normal_ret = Z_AXIS;
		return RayRelation::Intersect;
	}
	return RayRelation::None;
}

template <>
RayRelation Ray::isIntersectsAAFace<AAFaceKind::Xperp>(
				const rayvec3& faceMin,
				const rayvec2& faceOppositeCorner
				){ 
	if (fabs(glm::dot(dir, X_AXIS)) < 1.0E-8) { //precision of "parallelity"
		return RayRelation::Parallel;
	}

	scalar_t rayCoef = (faceMin.x - origin.x) / (dir.x);
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = { faceMin.x,
						rayCoef * dir.y + origin.y,
						rayCoef * dir.z + origin.z };

	if (intersectPoint.y >= faceMin.y
		&& intersectPoint.y <= faceOppositeCorner[0]
		&& intersectPoint.z >= faceMin.z
		&& intersectPoint.z <= faceOppositeCorner[1]) {
		return RayRelation::Intersect;
	}
	return RayRelation::None;
}

template <>
RayRelation Ray::isIntersectsAAFace<AAFaceKind::Yperp>(
				const rayvec3& faceMin,
				const rayvec2& faceOppositeCorner
				) {
	if (fabs(glm::dot(dir, Y_AXIS)) < 1.0E-8) { //precision of "parallelity"
		return RayRelation::Parallel;
	}

	scalar_t rayCoef = (faceMin.y - origin.y) / (dir.y);
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = { rayCoef * dir.x + origin.x,
						faceMin.y,
						rayCoef * dir.z + origin.z };

	if (intersectPoint.x >= faceMin.x  //Face-hit check
		&& intersectPoint.x <= faceOppositeCorner[0]
		&& intersectPoint.z >= faceMin.z
		&& intersectPoint.z <= faceOppositeCorner[1]) {
		return RayRelation::Intersect;
	}
	return RayRelation::None;
}

template <>
RayRelation Ray::isIntersectsAAFace<AAFaceKind::Zperp>(
				const rayvec3& faceMin,
				const rayvec2& faceOppositeCorner
				) {
	if (fabs(glm::dot(dir, Z_AXIS)) < 1.0E-8) { //precision of "parallelity"
		return RayRelation::Parallel;
	}

	scalar_t rayCoef = (faceMin.z - origin.z) / (dir.z);
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = { rayCoef * dir.x + origin.x,
						rayCoef * dir.y + origin.y,
						faceMin.z };

	if (intersectPoint.x >= faceMin.x  //Face-hit check
		&& intersectPoint.x <= faceOppositeCorner[0]
		&& intersectPoint.y >= faceMin.y
		&& intersectPoint.y <= faceOppositeCorner[1]) {
		return RayRelation::Intersect;
	}
	return RayRelation::None;
}

RayRelation Ray::intersectAABB(
                   const rayvec3& boxPos,
				   const AABB& box,
				   float maxDist,
                   glm::ivec3& normal_ret,
				   scalar_t& distance_ret){
	if constexpr (IS_RAYS_BOX_CACHE_ON){

		if (raysBoxCache_.find(boxPos) != raysBoxCache_.end()){
			const AABBFaces& boxFaces = raysBoxCache_[boxPos];
			return intersectAABBFaces(boxFaces, maxDist, normal_ret, distance_ret);
		} else {
			const AABBFaces& boxFaces = AABBFaces(boxPos, box);
			raysBoxCache_[boxPos] = boxFaces;
			return intersectAABBFaces(boxFaces, maxDist, normal_ret, distance_ret);
		}

	} else {
		const AABBFaces& boxFaces = AABBFaces(boxPos, box);
		return intersectAABBFaces(boxFaces, maxDist, normal_ret, distance_ret);
	}

}

RayRelation Ray::intersectAABBFaces(
                   const AABBFaces& boxFaces,
				   float maxDist,
                   glm::ivec3& normal_ret,
				   scalar_t& distance_ret){//TODO: points returning

	scalar_t faceDist;
	distance_ret = maxDist;
	glm::ivec3 bufNormal;
	//unsigned char intersectedCount = 0; //this code is very uncomfortable, DONT LEARN IT!
	bool isIntersect = false;

	if (intersectAAFace<AABBFaces::KINDS_ORDER[0]>(
		boxFaces.faces[0].first, boxFaces.faces[0].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret){
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}

	if (intersectAAFace<AABBFaces::KINDS_ORDER[1]>(
		boxFaces.faces[1].first, boxFaces.faces[1].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret) {
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}

	if (intersectAAFace<AABBFaces::KINDS_ORDER[2]>(
		boxFaces.faces[2].first, boxFaces.faces[2].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret) {
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}

	if (intersectAAFace<AABBFaces::KINDS_ORDER[3]>(
		boxFaces.faces[3].first, boxFaces.faces[3].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret) {
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}

	if (intersectAAFace<AABBFaces::KINDS_ORDER[4]>(
		boxFaces.faces[4].first, boxFaces.faces[4].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret) {
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}

	if (intersectAAFace<AABBFaces::KINDS_ORDER[5]>(
		boxFaces.faces[5].first, boxFaces.faces[5].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret) {
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}
	
	if (isIntersect) return RayRelation::Intersect;
	return RayRelation::None;
}