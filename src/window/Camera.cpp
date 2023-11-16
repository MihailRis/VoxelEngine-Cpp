#include "Camera.h"
#include "Window.h"

#include <glm/ext.hpp>

Camera::Camera(vec3 position, float fov) : position(position), fov(fov), zoom(1.0f), rotation(1.0f) {
	updateVectors();
}

void Camera::updateVectors(){
	front = vec3(rotation * vec4(0,0,-1,1));
	right = vec3(rotation * vec4(1,0,0,1));
	up = vec3(rotation * vec4(0,1,0,1));
	dir = vec3(rotation * vec4(0,0,-1,1));
	dir.y = 0;
	float len = length(dir);
	if (len > 0.0f){
		dir.x /= len;
		dir.z /= len;
	}
}

void Camera::rotate(float x, float y, float z){
	rotation = glm::rotate(rotation, z, vec3(0,0,1));
	rotation = glm::rotate(rotation, y, vec3(0,1,0));
	rotation = glm::rotate(rotation, x, vec3(1,0,0));

	updateVectors();
}

mat4 Camera::getProjection(){
	float aspect = this->aspect;
	if (aspect == 0.0f){
		aspect = (float)Window::width / (float)Window::height;
	}
	if (perspective)
		return glm::perspective(fov*zoom, aspect, 0.05f, 1500.0f);
	else
		if (flipped)
			return glm::ortho(0.0f, fov*aspect, fov, 0.0f);
		else
			return glm::ortho(0.0f, fov*aspect, 0.0f, fov);
}

mat4 Camera::getView(){
	if (perspective)
		return glm::lookAt(position, position+front, up);
	else
		return glm::translate(glm::mat4(1.0f), position);
}

mat4 Camera::getProjView(){
	return getProjection()*getView();
}