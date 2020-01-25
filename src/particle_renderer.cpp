#include "particle_renderer.h"
#include <iostream>


ParticleRenderer::ParticleRenderer(
        int grid_size, float scale, 
        const glm::ivec2& screen_size) :
    scale{scale},
    grid_size{grid_size},
    depth_shader{{"shaders/particles_depth.vrtx", GL_VERTEX_SHADER},
                 {"shaders/particles_depth.frgmnt", GL_FRAGMENT_SHADER}},
    smooth_depth_shader{{"shaders/particles_smooth.vrtx", GL_VERTEX_SHADER},
                 {"shaders/particles_smooth.frgmnt", GL_FRAGMENT_SHADER}},
    depth_buffer(screen_size),
    thickness_shader{{"shaders/particles_depth.vrtx", GL_VERTEX_SHADER},
                 {"shaders/particles_thickness.frgmnt", GL_FRAGMENT_SHADER}},
    blur_shader{{"shaders/particles_smooth.vrtx", GL_VERTEX_SHADER},
                 {"shaders/particles_blur.frgmnt", GL_FRAGMENT_SHADER}},
    thickness_buffer(screen_size),
    surface_shader{{"shaders/particles_surface.vrtx", GL_VERTEX_SHADER},
                 {"shaders/particles_surface.frgmnt", GL_FRAGMENT_SHADER}}
{
    std::vector<float> vertices = {
        0,0,
        1,0,
        1,1,
        0,0,
        1,1,
        0,1
    };


    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);
    glEnableVertexAttribArray(0);
    

    glGenBuffers(1, &offsets_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, offsets_vbo);
    glBufferData(GL_ARRAY_BUFFER, offsets.size()*sizeof(glm::vec3), offsets.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
    vbo_size = vertices.size();

    
    // depth
    float border_color[] = {0, 0, 0, 0};
    depth_texture0 = depth_buffer.attach_texture(GL_COLOR_ATTACHMENT0, GL_R32F, GL_RED, GL_FLOAT,
                                            GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, border_color);

    depth_texture1 = depth_buffer.attach_texture(GL_COLOR_ATTACHMENT1, GL_R32F, GL_RED, GL_FLOAT,
                                            GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, border_color);

    // thickness 
    thickness_texture0 = thickness_buffer.attach_texture(GL_COLOR_ATTACHMENT0, GL_R32F, GL_RED, GL_FLOAT,
                                            GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, border_color);
    thickness_texture1 = thickness_buffer.attach_texture(GL_COLOR_ATTACHMENT1, GL_R32F, GL_RED, GL_FLOAT,
                                            GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, border_color);
}


ParticleRenderer::~ParticleRenderer() {

}


void ParticleRenderer::render(const glm::mat4& proj,
                              const glm::mat4& view,
                              const glm::mat4& model,
                              GLuint voxel_depth_texture, 
                              GLuint voxel_normal_texture) {
    glBindVertexArray(vao);
    
    // depth pass
    glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer.get_id());
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    depth_shader.use();
    depth_shader.set("proj", proj);
    depth_shader.set("view", view);
    depth_shader.set("model", model);
    depth_shader.set("scale", scale);
    glBindBuffer(GL_ARRAY_BUFFER, offsets_vbo);
    glBufferData(GL_ARRAY_BUFFER, offsets.size()*sizeof(glm::vec3), offsets.data(), GL_STATIC_DRAW);
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, vbo_size, offsets.size());

    // depth smooth pass x
    glDrawBuffer(GL_COLOR_ATTACHMENT1);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    smooth_depth_shader.use();
    smooth_depth_shader.set("proj", proj);
    smooth_depth_shader.set("depth_texture", 0);
    smooth_depth_shader.set("dir", glm::vec2(1,0));
    smooth_depth_shader.set("scale", scale);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_texture0);

    glDrawArrays(GL_TRIANGLES, 0, vbo_size);
    
    // depth smooth pass y
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    smooth_depth_shader.set("dir", glm::vec2(0,1));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_texture1);

    glDrawArrays(GL_TRIANGLES, 0, vbo_size);


    // thickness pass
    GLint viewport_rect[4];
    glGetIntegerv(GL_VIEWPORT, viewport_rect);
    glViewport(0, 0, thickness_buffer.get_size().x, thickness_buffer.get_size().y);

    glBindFramebuffer(GL_FRAMEBUFFER, thickness_buffer.get_id());
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    thickness_shader.use();
    thickness_shader.set("proj", proj);
    thickness_shader.set("view", view);
    thickness_shader.set("model", model);
    thickness_shader.set("scale", scale);
    thickness_shader.set("voxel_depth_texture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, voxel_depth_texture);

    glBindBuffer(GL_ARRAY_BUFFER, offsets_vbo);
    glBufferData(GL_ARRAY_BUFFER, offsets.size()*sizeof(glm::vec3), offsets.data(), GL_STATIC_DRAW);
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, vbo_size, offsets.size());

    glDisable(GL_BLEND);


    // thickness blur pass x
    glDrawBuffer(GL_COLOR_ATTACHMENT1);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    blur_shader.use();
    blur_shader.set("proj", proj);
    blur_shader.set("in_texture", 0);
    blur_shader.set("dir", glm::vec2(1,0));
    blur_shader.set("scale", scale);
    glBindTexture(GL_TEXTURE_2D, thickness_texture0);

    glDrawArrays(GL_TRIANGLES, 0, vbo_size);
    
    // thickness blur pass y
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    blur_shader.set("dir", glm::vec2(0,1));
    glBindTexture(GL_TEXTURE_2D, thickness_texture1);

    glDrawArrays(GL_TRIANGLES, 0, vbo_size);

    glViewport(viewport_rect[0],viewport_rect[1],viewport_rect[2],viewport_rect[3]);



    // surface pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(.5, .5, .5, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    surface_shader.use();
    surface_shader.set("proj", proj);
    surface_shader.set("view", view);
    surface_shader.set("depth_texture", 0);
    surface_shader.set("thickness_texture", 1);
    surface_shader.set("voxel_depth_texture", 2);
    surface_shader.set("voxel_normal_texture", 3);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_texture0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, thickness_texture0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, voxel_depth_texture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, voxel_normal_texture);
    
    glDrawArrays(GL_TRIANGLES, 0, vbo_size);
    glDisable(GL_BLEND);
    

    glBindVertexArray(0);
}


void ParticleRenderer::set_offsets(const std::vector<glm::vec3>& offsets) {
    this->offsets.resize(offsets.size());
    for (size_t i = 0; i < offsets.size(); ++i)
        this->offsets[i] = (offsets[i] + .5f)/float(grid_size) - .5f;
}


