#ifndef VOXEL_RENDERER_H
#define VOXEL_RENDERER_H

#include "shader.h"
#include <glm/glm.hpp>
#include <vector>
#include "framebuffer.h"

class VoxelRenderer {
public:
    VoxelRenderer(int grid_size, const glm::ivec2& screen_size);

    void render(const glm::mat4& proj,
                const glm::mat4& view,
                const glm::mat4& model);

    const std::vector<float>& get_voxels() const { return voxels; }
    void set_voxels(std::vector<float> voxels);

    GLuint get_depth_texture() const { return depth_texture; }
    GLuint get_normal_texture() const { return normal_texture; }
private:
    static void generate_sphere(std::vector<float>& voxels, int grid_size,
                                const glm::vec3& pos, float r);

    
    int grid_size;
    std::vector<float> voxels;

    GLuint texture;

    Shader shader;

    GLuint cube_vao;
    GLuint cube_vbo;
    int cube_vbo_size;

    Framebuffer framebuffer;
    GLuint depth_texture;
    GLuint normal_texture;
};


#endif
