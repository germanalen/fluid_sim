#ifndef PARTICLE_RENDERER_H
#define PARTICLE_RENDERER_H

#include <glm/glm.hpp>
#include <vector>
#include "shader.h"
#include "framebuffer.h"

class ParticleRenderer {
public:
    ParticleRenderer(int grid_size, float scale, 
                     const glm::ivec2& screen_size);
    ~ParticleRenderer();
    
    void render(const glm::mat4& proj,
                const glm::mat4& view,
                const glm::mat4& model,
                GLuint voxel_depth_texture, GLuint voxel_normal_texture);

    void set_offsets(const std::vector<glm::vec3>& offsets);
private:
    std::vector<glm::vec3> offsets;
    float scale;
    int grid_size;

    GLuint vao;
    GLuint vbo;
    GLuint offsets_vbo;
    int vbo_size;
    
    Shader depth_shader;
    Shader smooth_depth_shader;
    Framebuffer depth_buffer;
    GLuint depth_texture0;
    GLuint depth_texture1;

    Shader thickness_shader;
    Shader blur_shader;
    Framebuffer thickness_buffer;
    GLuint thickness_texture0;
    GLuint thickness_texture1;

    Shader surface_shader;
};



#endif
