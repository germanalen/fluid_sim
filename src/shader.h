#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string>
#include <initializer_list>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(std::initializer_list<std::pair<std::string, GLuint>> paths);
    ~Shader();
    void use() const {glUseProgram(Shader::program);};
    inline GLint get_loc(const char* name) const {return glGetUniformLocation(program, name);};
    GLuint getID() const { return program; }

    void set(const std::string& name, bool p);
    void set(const std::string& name, int p);
    void set(const std::string& name, float p);
    void set(const std::string& name, const glm::vec2& p);
    void set(const std::string& name, const glm::vec3& p);
    void set(const std::string& name, const glm::ivec3& p);
    void set(const std::string& name, const glm::mat4& p);
    void set(const std::string& name, const glm::mat3& p);
    void setv(const std::string& name, int* p, int count);
    void setv(const std::string& name, const glm::ivec2* p, int count);
    void setv(const std::string& name, const glm::vec3* p, int count);
private:
    GLuint program;
    std::string read_file(const std::string& path);
    GLuint create_shader(const std::string& path, GLuint shader_type);
    bool check_error(GLuint program, bool isShader, const std::string& error_msg);
};

#endif
