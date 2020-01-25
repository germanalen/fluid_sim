#include "camera.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "misc.h"
#include <iostream>

Camera::Camera() {
    phi = M_PI / 2;
    theta = M_PI / 2;
}

void Camera::orbit(float dx, float dy, float dist, glm::vec3 center) {
    phi += dx * .001;
    theta = glm::clamp(theta - dy * .001f, 0.01f, (float)M_PI - 0.01f);

    dir = glm::normalize(glm::vec3(sin(theta)*cos(phi), cos(theta), sin(theta)*sin(phi)));
    pos = center + dir * dist;
    right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), dir));
}


glm::mat4 Camera::get_view() const {
    glm::mat4 m = glm::translate(glm::mat4(1), -pos);
    return get_view_no_translate() * m;
}


glm::mat4 Camera::get_view_no_translate() const {
   glm::vec3 up = glm::cross(dir, right);
   double mat4[16] = {
       right.x,  up.x, dir.x,  0,
       right.y,  up.y, dir.y,  0,
       right.z,  up.z, dir.z,  0,
             0,     0,           0,  1
   };
   return glm::make_mat4(mat4);
}
