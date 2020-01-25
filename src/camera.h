#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
    Camera();

    void orbit(float dx, float dy, float dist, glm::vec3 center);
    glm::mat4 get_view() const;
    glm::mat4 get_view_no_translate() const;
private:
    float phi;
    float theta;

    glm::vec3 dir, pos, right;
};


#endif
