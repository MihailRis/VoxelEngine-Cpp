
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
				   rayvec3& intersectPoint_ret
				   ){
	if (fabs(glm::dot(rayDir, X_AXIS)) < 1.0E-8){ //precision
		return RayRelation::Parallel;
	}

	glm::float64 rayCoef = (faceMin.x - rayOrigin.x) / (rayDir.x);
	intersectPoint_ret = {faceMin.x, 
						rayCoef*rayDir.y + rayOrigin.y, 
						rayCoef*rayDir.z + rayOrigin.z};
	
	if (rayDir.x > 0){
		if (intersectPoint_ret.y >= faceMin.y
		&& intersectPoint_ret.y <= faceOppositeCorner[0]
		&& intersectPoint_ret.z >= faceMin.z
		&& intersectPoint_ret.z <= faceOppositeCorner[1]){ 
			return RayRelation::Intersect; 
		} 
	}
	else{
		if (intersectPoint_ret.y <= faceMin.y
		&& intersectPoint_ret.y >= faceOppositeCorner[0]
		&& intersectPoint_ret.z <= faceMin.z
		&& intersectPoint_ret.z >= faceOppositeCorner[1]){ 
			return RayRelation::Intersect; 
		} 
	}
	return RayRelation::None;
}

template <>
RayRelation Rays::rayIntersectAAFace<AAFaceKind::Yperp>(
	               const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner, //x and z global coords of opposite corner
				   rayvec3& intersectPoint_ret
				   ){
	if (fabs(glm::dot(rayDir, Y_AXIS)) < 1.0E-8){ //precision
		return RayRelation::Parallel;
	}

	glm::float64 rayCoef = (faceMin.y - rayOrigin.y) / (rayDir.y);
	intersectPoint_ret = {rayCoef*rayDir.x + rayOrigin.x,
						faceMin.y,
						rayCoef*rayDir.z + rayOrigin.z};
	
	if (rayDir.y > 0){
		if (intersectPoint_ret.x >= faceMin.x  //Face-hit check
		&& intersectPoint_ret.x <= faceOppositeCorner[0] 
		&& intersectPoint_ret.z >= faceMin.z 
		&& intersectPoint_ret.z <= faceOppositeCorner[1] ){ 
			return RayRelation::Intersect; 
		} 
	}
	else{
		if (intersectPoint_ret.x <= faceMin.x  //Face-hit check for negative dir.
		&& intersectPoint_ret.x >= faceOppositeCorner[0] 
		&& intersectPoint_ret.z <= faceMin.z 
		&& intersectPoint_ret.z >= faceOppositeCorner[1]){ 
			return RayRelation::Intersect; 
		}
	}
	return RayRelation::None;
}

template <>
RayRelation Rays::rayIntersectAAFace<AAFaceKind::Zperp>(
	               const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner, //x and y global coords of opposite corner
				   rayvec3& intersectPoint_ret
					){
	if (fabs(glm::dot(rayDir, Z_AXIS)) < 1.0E-8){ //precision
		return RayRelation::Parallel;
	}
	
	glm::float64 rayCoef = (faceMin.z - rayOrigin.z) / (rayDir.z);
	intersectPoint_ret = {rayCoef*rayDir.x + rayOrigin.x,
						rayCoef*rayDir.y + rayOrigin.y,
						faceMin.z};

	if (rayDir.z > 0){
		if (intersectPoint_ret.x >= faceMin.x  //Face-hit check
		&& intersectPoint_ret.x <= faceOppositeCorner[0] 
		&& intersectPoint_ret.y >= faceMin.y  
		&& intersectPoint_ret.y <= faceOppositeCorner[1]  ){ 
			return RayRelation::Intersect; 
		}
	}
	else{
		if (intersectPoint_ret.x <= faceMin.x    //Face-hit check
		&& intersectPoint_ret.x >= faceOppositeCorner[0]  
		&& intersectPoint_ret.y <= faceMin.y  
		&& intersectPoint_ret.y >= faceOppositeCorner[1]  ){ 
			return RayRelation::Intersect; 
		} 
	}
	return RayRelation::None;
}

double Rays::updateNormal(
	               double newDistApprox,
				   const glm::ivec3& newNormal,
				   double currentDistApprox,
				   glm::ivec3& normal_ret
				   ){
					if (newDistApprox < currentDistApprox){
						currentDistApprox = newDistApprox;
						normal_ret = newNormal;
					}
					return currentDistApprox;
				   }

template <>
RayRelation Rays::isRayIntersectsAAFace<AAFaceKind::Xperp>(
				   const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner,
				   glm::ivec3& normal_ret,
				   double& currentDistApprox
				){ 
	if (fabs(glm::dot(rayDir, X_AXIS)) < 1.0E-8){ //precision of "parallelity"
		return RayRelation::Parallel;
	}

	glm::float64 rayCoef = (faceMin.x - rayOrigin.x);
	rayvec3 intersectPointMult = {faceMin.x * rayDir.x, 
						rayCoef*rayDir.y + rayOrigin.y * rayDir.x, 
						rayCoef*rayDir.z + rayOrigin.z * rayDir.x};
	
	if (rayDir.x > 0){
		if (intersectPointMult.y >= faceMin.y * rayDir.x  //Face-hit check
		&& intersectPointMult.y <= faceOppositeCorner[0] * rayDir.x
		&& intersectPointMult.z >= faceMin.z * rayDir.x
		&& intersectPointMult.z <= faceOppositeCorner[1] * rayDir.x){ 
			currentDistApprox = updateNormal(fabs(rayCoef * rayDir.y * rayDir.z), -X_AXIS, currentDistApprox, normal_ret);
			return RayRelation::Intersect; 
		} 
	}
	else{
		if (intersectPointMult.y <= faceMin.y * rayDir.x  //Face-hit check for negative dir.
		&& intersectPointMult.y >= faceOppositeCorner[0] * rayDir.x
		&& intersectPointMult.z <= faceMin.z * rayDir.x
		&& intersectPointMult.z >= faceOppositeCorner[1] * rayDir.x){ 
			currentDistApprox = updateNormal(fabs(rayCoef * rayDir.y * rayDir.z), X_AXIS, currentDistApprox, normal_ret);
			return RayRelation::Intersect; 
		}
	}
	return RayRelation::None;
}

template <>
RayRelation Rays::isRayIntersectsAAFace<AAFaceKind::Yperp>(
				   const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner,
				   glm::ivec3& normal_ret,
				   double& currentDistApprox
				){ 
	if (fabs(glm::dot(rayDir, Y_AXIS)) < 1.0E-8){ //precision of "parallelity"
		return RayRelation::Parallel;
	}

	glm::float64 rayCoef = (faceMin.y - rayOrigin.y);
	rayvec3 intersectPointMult = {rayCoef*rayDir.x + rayOrigin.x * rayDir.y,
						faceMin.y * rayDir.y,
						rayCoef*rayDir.z + rayOrigin.z * rayDir.y};
	
	if (rayDir.y > 0){
		if (intersectPointMult.x >= faceMin.x * rayDir.y  //Face-hit check
		&& intersectPointMult.x <= faceOppositeCorner[0] * rayDir.y
		&& intersectPointMult.z >= faceMin.z * rayDir.y
		&& intersectPointMult.z <= faceOppositeCorner[1] * rayDir.y){ 
			currentDistApprox = updateNormal(fabs(rayCoef * rayDir.x * rayDir.z), -Y_AXIS, currentDistApprox, normal_ret);
			return RayRelation::Intersect; 
		} 
	}
	else{
		if (intersectPointMult.x <= faceMin.x * rayDir.y  //Face-hit check for negative dir.
		&& intersectPointMult.x >= faceOppositeCorner[0] * rayDir.y
		&& intersectPointMult.z <= faceMin.z * rayDir.y
		&& intersectPointMult.z >= faceOppositeCorner[1] * rayDir.y){ 
			currentDistApprox = updateNormal(fabs(rayCoef * rayDir.x * rayDir.z), Y_AXIS, currentDistApprox, normal_ret);
			return RayRelation::Intersect; 
		}
	}
	return RayRelation::None;
}

template <>
RayRelation Rays::isRayIntersectsAAFace<AAFaceKind::Zperp>(
				   const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
				   const rayvec3& faceMin,
				   const rayvec2& faceOppositeCorner,
				   glm::ivec3& normal_ret,
				   double& currentDistApprox
				){ 
	if (fabs(glm::dot(rayDir, Z_AXIS)) < 1.0E-8){ //precision of "parallelity"
		return RayRelation::Parallel;
	}

	glm::float64 rayCoef = (faceMin.z - rayOrigin.z);
	rayvec3 intersectPointMult = {rayCoef*rayDir.x + rayOrigin.x * rayDir.z, 
						rayCoef*rayDir.y + rayOrigin.y * rayDir.z,
						faceMin.z * rayDir.z};
	
	if (rayDir.z > 0){
		if (intersectPointMult.x >= faceMin.x * rayDir.z  //Face-hit check
		&& intersectPointMult.x <= faceOppositeCorner[0] * rayDir.z
		&& intersectPointMult.y >= faceMin.y * rayDir.z
		&& intersectPointMult.y <= faceOppositeCorner[1] * rayDir.z){ 
			currentDistApprox = updateNormal(fabs(rayCoef * rayDir.x * rayDir.y), -Z_AXIS, currentDistApprox, normal_ret);
			return RayRelation::Intersect; 
		}
	}
	else{
		if (intersectPointMult.x <= faceMin.x * rayDir.z  //Face-hit check
		&& intersectPointMult.x >= faceOppositeCorner[0] * rayDir.z
		&& intersectPointMult.y <= faceMin.y * rayDir.z
		&& intersectPointMult.y >= faceOppositeCorner[1] * rayDir.z){ 
			currentDistApprox = updateNormal(fabs(rayCoef * rayDir.x * rayDir.y), Z_AXIS, currentDistApprox, normal_ret);
			return RayRelation::Intersect; 
		} 
	}
	return RayRelation::None;
}

RayRelation Rays::rayIntersectAABB(
                   const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
                   const rayvec3& boxPos,
				   const AABB& box,
				   float maxDist,
				   rayvec3& pointIn_ret,
				   rayvec3& pointOut_ret,
                   glm::ivec3& normal_ret){
	if constexpr (IS_RAYS_BOX_CACHE_ON){

		if (raysBoxCache_.find(boxPos) != raysBoxCache_.end()){
			const AABBFaces& boxFaces = raysBoxCache_[boxPos];
			return rayIntersectAABBFaces(rayOrigin, rayDir, boxFaces, maxDist, pointIn_ret, pointOut_ret, normal_ret);
		} else {
			const AABBFaces& boxFaces = AABBFaces(boxPos, box);
			raysBoxCache_[boxPos] = boxFaces;
			return rayIntersectAABBFaces(rayOrigin, rayDir, boxFaces, maxDist, pointIn_ret, pointOut_ret, normal_ret);
		}

	} else {
		const AABBFaces& boxFaces = AABBFaces(boxPos, box);
		return rayIntersectAABBFaces(rayOrigin, rayDir, boxFaces, maxDist, pointIn_ret, pointOut_ret, normal_ret);
	}

}

RayRelation Rays::rayIntersectAABBFaces(
                   const rayvec3& rayOrigin, 
				   const rayvec3& rayDir,
                   const AABBFaces& boxFaces,
				   float maxDist,
				   rayvec3& pointIn_ret,
				   rayvec3& pointOut_ret,
                   glm::ivec3& normal_ret){//TODO: points returning
	RayRelation rel;
	double faceDistApprox = maxDist;
	unsigned char intersectedCount = 0; //this code is very uncomfortable, DONT LEARN IT!
	rel = isRayIntersectsAAFace<AABBFaces::KINDS_ORDER[0]>(
			rayOrigin, rayDir, boxFaces.faces[0].first, boxFaces.faces[0].second, normal_ret, faceDistApprox
			);
	if (rel > RayRelation::None){
		++intersectedCount;
	}

	rel = isRayIntersectsAAFace<AABBFaces::KINDS_ORDER[1]>(
			rayOrigin, rayDir, boxFaces.faces[1].first, boxFaces.faces[1].second, normal_ret, faceDistApprox
			);
	if (rel > RayRelation::None){
		++intersectedCount;
	}

	rel = isRayIntersectsAAFace<AABBFaces::KINDS_ORDER[2]>(
			rayOrigin, rayDir, boxFaces.faces[2].first, boxFaces.faces[2].second, normal_ret, faceDistApprox
			);
	if (rel > RayRelation::None){
		++intersectedCount;
	}

	rel = isRayIntersectsAAFace<AABBFaces::KINDS_ORDER[3]>(
			rayOrigin, rayDir, boxFaces.faces[3].first, boxFaces.faces[3].second, normal_ret, faceDistApprox
			);
	if (rel > RayRelation::None){
		++intersectedCount;
	}

	rel = isRayIntersectsAAFace<AABBFaces::KINDS_ORDER[4]>(
			rayOrigin, rayDir, boxFaces.faces[4].first, boxFaces.faces[4].second, normal_ret, faceDistApprox
			);
	if (rel > RayRelation::None){
		++intersectedCount;
	}

	rel = isRayIntersectsAAFace<AABBFaces::KINDS_ORDER[5]>(
			rayOrigin, rayDir, boxFaces.faces[5].first, boxFaces.faces[5].second, normal_ret, faceDistApprox
			);
	if (rel > RayRelation::None){
		++intersectedCount;
	}
	
	if (intersectedCount > 0) return RayRelation::Intersect;
	return RayRelation::None;
}