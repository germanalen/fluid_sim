#ifndef VECFIELD_RENDERER_H
#define VECFIELD_RENDERER_H

#include <glm/glm.hpp>
#include <vector>
#include "shader.h"

// !!! assumes grid is 10x10x10 !!!
class VecfieldRenderer {
public:
    
    VecfieldRenderer();


    void render(const glm::mat4& proj,
                const glm::mat4& view,
                const glm::mat4& model);

    void render_arrow(const glm::mat4& proj,
                      const glm::mat4& view,
                      const glm::mat4& model,
                      const glm::vec3& pos,
                      const glm::vec3& dir);

    void set_dirs(std::vector<glm::vec3> dirs);
    void set_max_len(float max_len);
private:
    std::vector<glm::vec3> dirs;
    float max_len = 1;


    Shader shader;
    Shader arrow_shader;

    GLuint vao;
    GLuint vbo;
    int vbo_size;
};

#endif
