/*
 * Camera.h
 *
 *  Created on: Feb 11, 2020
 *      Author: MihailRis
 */

#ifndef WINDOW_CAMERA_H_
#define WINDOW_CAMERA_H_

#include <glm/glm.hpp>
using namespace glm;

class Camera {
	void updateVectors();
public:
	vec3 front;
	vec3 up;
	vec3 right;

	vec3 position;
	float fov;
	mat4 rotation;
	Camera(vec3 position, float fov);

	void rotate(float x, float y, float z);

	mat4 getProjection();
	mat4 getView();
};

#endif /* WINDOW_CAMERA_H_ */
