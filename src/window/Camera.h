#ifndef WINDOW_CAMERA_H_
#define WINDOW_CAMERA_H_

#include <glm/glm.hpp>

class Camera {
	void updateVectors();
	float fov;
public:
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 dir;

	glm::vec3 position;

	float zoom;
	glm::mat4 rotation;
	bool perspective = true;
	bool flipped = false;
	float aspect = 0.0f;
	Camera(glm::vec3 position, float fov);

	void rotate(float x, float y, float z);

	glm::mat4 getProjection();
	glm::mat4 getView(bool position=true);
	glm::mat4 getProjView(bool position=true);

	void setFov(float fov);
	float getFov() const;
};

#endif /* WINDOW_CAMERA_H_ */
