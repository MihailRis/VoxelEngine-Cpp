#ifndef WINDOW_CAMERA_HPP_
#define WINDOW_CAMERA_HPP_

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class Camera {
    float fov = 1.0f;
public:
    glm::vec3 front {};
    glm::vec3 up {};
    glm::vec3 right {};
    glm::vec3 dir {};

    glm::vec3 position {};

    float zoom = 1.0f;
    glm::mat4 rotation {1.0f};
    bool perspective = true;
    bool flipped = false;
    float aspect = 0.0f;

    Camera() {
        updateVectors();
    }
    Camera(glm::vec3 position, float fov);

    void updateVectors();
    void rotate(float x, float y, float z);

    glm::mat4 getProjection();
    glm::mat4 getView(bool position = true);
    glm::mat4 getProjView(bool position = true);

    void setFov(float fov);
    float getFov() const;
};

#endif  // WINDOW_CAMERA_HPP_
