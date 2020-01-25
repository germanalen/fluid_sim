#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glm/glm.hpp>
#include <GL/glew.h>

class Framebuffer {
public:
    Framebuffer(const glm::ivec2& size);
    Framebuffer(const Framebuffer& f) = delete;
    void operator=(const Framebuffer& f) = delete;

    GLuint attach_texture(GLenum color_attachment,
                          GLint internal_format, GLenum format, GLenum type, 
                          GLint wrap_s, GLint wrap_t, 
                          const GLfloat *border_color);


    GLuint get_id() const { return id; }
    const glm::ivec2& get_size() const { return size; }
private:
    glm::ivec2 size;
    GLuint id;
};


#endif
