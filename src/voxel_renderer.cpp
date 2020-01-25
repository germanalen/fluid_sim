#include "voxel_renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include <SDL2/SDL.h> // SDL_GetTicks()

VoxelRenderer::VoxelRenderer(int grid_size, const glm::ivec2& screen_size) :
        grid_size{grid_size},
        shader{{"shaders/voxels.vrtx", GL_VERTEX_SHADER},
               {"shaders/voxels.frgmnt", GL_FRAGMENT_SHADER}},
        framebuffer(screen_size)
{
    //////////////////////////////////////////////////
    std::vector<float> vertices = {-0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5};
    
    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &cube_vbo);
    
    glBindVertexArray(cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    cube_vbo_size = vertices.size();

    //////////////////////////////////////////////////
    voxels.resize(grid_size * grid_size * grid_size);
    generate_sphere(voxels, grid_size, glm::vec3((grid_size-1)/2.0), (grid_size-1)/2.0);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F,
                 grid_size, grid_size, grid_size,
                 0, GL_RED, GL_FLOAT,
                 &voxels[0]);
    glBindTexture(GL_TEXTURE_3D, 0);

    //////////////////////////////////////////////////
    float border_color[] = {0, 0, 0, 0};
    depth_texture = framebuffer.attach_texture(GL_COLOR_ATTACHMENT0, GL_R32F, GL_RED, GL_FLOAT,
                                               GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, border_color);
    normal_texture = framebuffer.attach_texture(GL_COLOR_ATTACHMENT1, GL_RGB16F, GL_RGB, GL_FLOAT,
                                                GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, border_color);

}


void VoxelRenderer::render(const glm::mat4& proj,
                           const glm::mat4& view,
                           const glm::mat4& model) {
    
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get_id());
    GLenum buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat3 normal_matrix = glm::mat3(view) * glm::mat3(model);
    glm::mat3 inv_normal_matrix = glm::inverse(normal_matrix);


    shader.use();
    shader.set("proj", proj);
    shader.set("view", view);
    shader.set("model", model);
    shader.set("normal_matrix", normal_matrix);
    shader.set("inv_normal_matrix", inv_normal_matrix);
    shader.set("grid_shape", glm::ivec3(grid_size));
    
    shader.set("voxels", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,
                    grid_size, grid_size, grid_size,
                    GL_RED, GL_FLOAT,
                    &voxels[0]);

    glBindVertexArray(cube_vao);
    glDrawArrays(GL_TRIANGLES, 0, cube_vbo_size);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void VoxelRenderer::set_voxels(std::vector<float> voxels) {
    assert(voxels.size() == this->voxels.size());
    this->voxels = std::move(voxels);
}


void VoxelRenderer::generate_sphere(std::vector<float>& voxels, int grid_size, 
                                    const glm::vec3& pos, float r) {
    int cr = (int)glm::ceil(r);
    glm::ivec3 ipos(pos);
    for (int x = ipos.x - cr; x < ipos.x + cr; ++x) {
        for (int y = ipos.y - cr; y < ipos.y + cr; ++y) {
            for (int z = ipos.z - cr; z < ipos.z + cr; ++z) {
                if (glm::distance(glm::vec3(x,y,z), pos) < r)
                    voxels[(x * grid_size + y) * grid_size + z] = 1;
            }
        }
    }
}


