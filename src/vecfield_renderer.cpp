#include "vecfield_renderer.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>


VecfieldRenderer::VecfieldRenderer() :
    shader{{"shaders/vecfield.vrtx", GL_VERTEX_SHADER},
           {"shaders/vecfield.frgmnt", GL_FRAGMENT_SHADER}},
    arrow_shader{{"shaders/arrow.vrtx", GL_VERTEX_SHADER},
                 {"shaders/arrow.frgmnt", GL_FRAGMENT_SHADER}}
{
    std::vector<float> vertices = {
        0, 0, 0,
        1, 1, 1,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    vbo_size = vertices.size();


}


void VecfieldRenderer::render(const glm::mat4& proj,
                              const glm::mat4& view,
                              const glm::mat4& model) {


    shader.use();
    shader.set("proj", proj);
    shader.set("view", view);
    shader.set("model", model);
    shader.setv("dirs", dirs.data(), dirs.size());
    shader.set("max_len", max_len);
    
    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_LINES, 0, vbo_size, dirs.size());
    glBindVertexArray(0);

}



void VecfieldRenderer::render_arrow(const glm::mat4& proj,
                                    const glm::mat4& view,
                                    const glm::mat4& model,
                                    const glm::vec3& pos,
                                    const glm::vec3& dir) {
    arrow_shader.use();
    arrow_shader.set("proj", proj);
    arrow_shader.set("view", view);
    arrow_shader.set("model", model);
    arrow_shader.set("pos", pos);
    arrow_shader.set("dir", dir);
    arrow_shader.set("max_len", max_len);
    
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, vbo_size);
    glBindVertexArray(0);

   
}


void VecfieldRenderer::set_dirs(std::vector<glm::vec3> dirs) {
    assert(dirs.size() == 1000);
    this->dirs = std::move(dirs);
}

void VecfieldRenderer::set_max_len(float max_len) {
    this->max_len = max_len;
}
