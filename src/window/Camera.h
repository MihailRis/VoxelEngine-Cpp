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
	vec3 dir;

	vec3 position;
	float fov;
	float zoom;
	mat4 rotation;
	bool perspective = true;
	bool flipped = false;
	float aspect = 0.0f;
	Camera(vec3 position, float fov);

	void rotate(float x, float y, float z);

	mat4 getProjection();
	mat4 getView();
};

#endif /* WINDOW_CAMERA_H_ */
