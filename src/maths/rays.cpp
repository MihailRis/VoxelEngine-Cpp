
#include "rays.h"
#include "aabb.h"

#include "glm/glm.hpp"

std::unordered_map<rayvec3, AABBFaces> Rays::raysBoxCache_ = {};
const rayvec3 X_AXIS = rayvec3(1,0,0), Y_AXIS = rayvec3(0,1,0), Z_AXIS = rayvec3(0,0,1);

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
RayRelation Rays::rayIntersectAAFace<AAFaceKind::Xperp>( 
	               const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner, //y and z global coords of opposite corner
				   glm::ivec3& normal_ret,
				   scalar_t& distance_ret //sinonym of rayCoef
				   ){
	if (fabs(glm::dot(rayDir, X_AXIS)) < 1.0E-8){ //precision
		return RayRelation::Parallel;
	}

	scalar_t rayCoef = (faceMin.x - rayOrigin.x) / (rayDir.x);// equivalent to distance if raydir normalized
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = {faceMin.x,
						rayCoef*rayDir.y + rayOrigin.y,
						rayCoef*rayDir.z + rayOrigin.z};

	if (intersectPoint.y >= faceMin.y
	&& intersectPoint.y <= faceOppositeCorner[0]
	&& intersectPoint.z >= faceMin.z
	&& intersectPoint.z <= faceOppositeCorner[1]){
		distance_ret = rayCoef; // believe that raydir normalized
		if (rayDir.x > 0) normal_ret = -X_AXIS;
		else normal_ret = X_AXIS;
		return RayRelation::Intersect; 
	} 
	return RayRelation::None;
}

template <>
RayRelation Rays::rayIntersectAAFace<AAFaceKind::Yperp>(
	               const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner, //x and z global coords of opposite corner
				   glm::ivec3& normal_ret,
				   scalar_t& distance_ret
				   ){
	if (fabs(glm::dot(rayDir, Y_AXIS)) < 1.0E-8){ //precision
		return RayRelation::Parallel;
	}

	scalar_t rayCoef = (faceMin.y - rayOrigin.y) / (rayDir.y);// equivalent to distance if raydir normalized
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = { rayCoef *rayDir.x + rayOrigin.x,
						faceMin.y,
						rayCoef*rayDir.z + rayOrigin.z};

	if (intersectPoint.x >= faceMin.x  //Face-hit check
	&& intersectPoint.x <= faceOppositeCorner[0] 
	&& intersectPoint.z >= faceMin.z 
	&& intersectPoint.z <= faceOppositeCorner[1] ){
		distance_ret = rayCoef; // believe that raydir normalized
		if (rayDir.y > 0) normal_ret = -Y_AXIS;
		else normal_ret = Y_AXIS;
		return RayRelation::Intersect;
	} 
	return RayRelation::None;
}

template <>
RayRelation Rays::rayIntersectAAFace<AAFaceKind::Zperp>(
	               const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner, //x and y global coords of opposite corner
				   glm::ivec3& normal_ret,
				   scalar_t& distance_ret
					){
	if (fabs(glm::dot(rayDir, Z_AXIS)) < 1.0E-8){ //precision
		return RayRelation::Parallel;
	}
	
	scalar_t rayCoef = (faceMin.z - rayOrigin.z) / (rayDir.z); // equivalent to distance if raydir normalized
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = { rayCoef *rayDir.x + rayOrigin.x,
						rayCoef*rayDir.y + rayOrigin.y,
						faceMin.z};
		
	if (intersectPoint.x >= faceMin.x  //Face-hit check
	&& intersectPoint.x <= faceOppositeCorner[0] 
	&& intersectPoint.y >= faceMin.y  
	&& intersectPoint.y <= faceOppositeCorner[1]  ){
		distance_ret = rayCoef; // believe that raydir normalized
		if (rayDir.z > 0) normal_ret = -Z_AXIS;
		else normal_ret = Z_AXIS;
		return RayRelation::Intersect;
	}
	return RayRelation::None;
}

template <>
RayRelation Rays::isRayIntersectsAAFace<AAFaceKind::Xperp>(
				const rayvec3& rayOrigin, 
				const rayvec3& rayDir,
				const rayvec3& faceMin,
				const rayvec2& faceOppositeCorner
				){ 
	if (fabs(glm::dot(rayDir, X_AXIS)) < 1.0E-8) { //precision of "parallelity"
		return RayRelation::Parallel;
	}

	scalar_t rayCoef = (faceMin.x - rayOrigin.x) / (rayDir.x);
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = { faceMin.x,
						rayCoef * rayDir.y + rayOrigin.y,
						rayCoef * rayDir.z + rayOrigin.z };

	if (intersectPoint.y >= faceMin.y
		&& intersectPoint.y <= faceOppositeCorner[0]
		&& intersectPoint.z >= faceMin.z
		&& intersectPoint.z <= faceOppositeCorner[1]) {
		return RayRelation::Intersect;
	}
	return RayRelation::None;
}

template <>
RayRelation Rays::isRayIntersectsAAFace<AAFaceKind::Yperp>(
				const rayvec3& rayOrigin,
				const rayvec3& rayDir,
				const rayvec3& faceMin,
				const rayvec2& faceOppositeCorner
				) {
	if (fabs(glm::dot(rayDir, Y_AXIS)) < 1.0E-8) { //precision of "parallelity"
		return RayRelation::Parallel;
	}

	scalar_t rayCoef = (faceMin.y - rayOrigin.y) / (rayDir.y);
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = { rayCoef * rayDir.x + rayOrigin.x,
						faceMin.y,
						rayCoef * rayDir.z + rayOrigin.z };

	if (intersectPoint.x >= faceMin.x  //Face-hit check
		&& intersectPoint.x <= faceOppositeCorner[0]
		&& intersectPoint.z >= faceMin.z
		&& intersectPoint.z <= faceOppositeCorner[1]) {
		return RayRelation::Intersect;
	}
	return RayRelation::None;
}

template <>
RayRelation Rays::isRayIntersectsAAFace<AAFaceKind::Zperp>(
				const rayvec3& rayOrigin,
				const rayvec3& rayDir,
				const rayvec3& faceMin,
				const rayvec2& faceOppositeCorner
				) {
	if (fabs(glm::dot(rayDir, Z_AXIS)) < 1.0E-8) { //precision of "parallelity"
		return RayRelation::Parallel;
	}

	scalar_t rayCoef = (faceMin.z - rayOrigin.z) / (rayDir.z);
	if (rayCoef < 0) return RayRelation::None;
	rayvec3 intersectPoint = { rayCoef * rayDir.x + rayOrigin.x,
						rayCoef * rayDir.y + rayOrigin.y,
						faceMin.z };

	if (intersectPoint.x >= faceMin.x  //Face-hit check
		&& intersectPoint.x <= faceOppositeCorner[0]
		&& intersectPoint.y >= faceMin.y
		&& intersectPoint.y <= faceOppositeCorner[1]) {
		return RayRelation::Intersect;
	}
	return RayRelation::None;
}

RayRelation Rays::rayIntersectAABB(
                   const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
                   const rayvec3& boxPos,
				   const AABB& box,
				   float maxDist,
                   glm::ivec3& normal_ret,
				   scalar_t& distance_ret){
	if constexpr (IS_RAYS_BOX_CACHE_ON){

		if (raysBoxCache_.find(boxPos) != raysBoxCache_.end()){
			const AABBFaces& boxFaces = raysBoxCache_[boxPos];
			return rayIntersectAABBFaces(rayOrigin, rayDir, boxFaces, maxDist, normal_ret, distance_ret);
		} else {
			const AABBFaces& boxFaces = AABBFaces(boxPos, box);
			raysBoxCache_[boxPos] = boxFaces;
			return rayIntersectAABBFaces(rayOrigin, rayDir, boxFaces, maxDist, normal_ret, distance_ret);
		}

	} else {
		const AABBFaces& boxFaces = AABBFaces(boxPos, box);
		return rayIntersectAABBFaces(rayOrigin, rayDir, boxFaces, maxDist, normal_ret, distance_ret);
	}

}

RayRelation Rays::rayIntersectAABBFaces(
                   const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
                   const AABBFaces& boxFaces,
				   float maxDist,
                   glm::ivec3& normal_ret,
				   scalar_t& distance_ret){//TODO: points returning

	scalar_t faceDist;
	distance_ret = maxDist;
	glm::ivec3 bufNormal;
	//unsigned char intersectedCount = 0; //this code is very uncomfortable, DONT LEARN IT!
	bool isIntersect = false;

	if (rayIntersectAAFace<AABBFaces::KINDS_ORDER[0]>(
		rayOrigin, rayDir, boxFaces.faces[0].first, boxFaces.faces[0].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret){
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}

	if (rayIntersectAAFace<AABBFaces::KINDS_ORDER[1]>(
		rayOrigin, rayDir, boxFaces.faces[1].first, boxFaces.faces[1].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret) {
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}

	if (rayIntersectAAFace<AABBFaces::KINDS_ORDER[2]>(
		rayOrigin, rayDir, boxFaces.faces[2].first, boxFaces.faces[2].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret) {
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}

	if (rayIntersectAAFace<AABBFaces::KINDS_ORDER[3]>(
		rayOrigin, rayDir, boxFaces.faces[3].first, boxFaces.faces[3].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret) {
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}

	if (rayIntersectAAFace<AABBFaces::KINDS_ORDER[4]>(
		rayOrigin, rayDir, boxFaces.faces[4].first, boxFaces.faces[4].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret) {
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}

	if (rayIntersectAAFace<AABBFaces::KINDS_ORDER[5]>(
		rayOrigin, rayDir, boxFaces.faces[5].first, boxFaces.faces[5].second, bufNormal, faceDist
		) > RayRelation::None && faceDist < distance_ret) {
		isIntersect = true;
		normal_ret = bufNormal;
		distance_ret = faceDist;
	}
	
	if (isIntersect) return RayRelation::Intersect;
	return RayRelation::None;
}