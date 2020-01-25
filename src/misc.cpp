#include "misc.h"


std::ostream& operator<<(std::ostream& out, const glm::vec3& v) {
    out << "[" << v.x << "," << v.y << "," << v.z << "]";
    return out;
}
