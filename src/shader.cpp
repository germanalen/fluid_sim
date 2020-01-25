#include <iostream>
#include "shader.h"
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <fstream>
#include <vector>

Shader::Shader(std::initializer_list<std::pair<std::string, GLuint>> paths){
	//std::cout << "Creating Shader..." << std::endl;
	program = glCreateProgram();
	std::vector<GLuint> shaders;
	for (auto& path : paths) {
		//std::cout << path.first << " " << path.second << std::endl;
		GLuint shader = create_shader(path.first, path.second);
		glAttachShader(program, shader);
		shaders.push_back(shader);
	}
	glLinkProgram(program);
	if (!check_error(program, false, "LINKING::ERROR")) exit(-1);
	for (auto& shader : shaders)
		glDeleteShader(shader);
}

Shader::~Shader() {
    glDeleteProgram(program);
}

std::string Shader::read_file(const std::string& path){
	std::ifstream shader_file(path);
	if (!shader_file.is_open())
	    std::cerr << "Can't open " << path << std::endl;
	std::stringstream buffer;
	buffer << shader_file.rdbuf();
	return buffer.str();
}

GLuint Shader::create_shader(const std::string& path, GLuint shader_type){
	std::string text = read_file(path);
	const GLchar* source = text.c_str();
	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	if (!check_error(shader, true, "SHADER::COMPILE ERROR")) exit(-1);
	
	return shader;
}

bool Shader::check_error(GLuint program, bool isShader,const std::string& error_msg){
	int success;
	char infoLog[512];
	if(isShader){
		glGetShaderiv(program, GL_COMPILE_STATUS, &success);
		if(!success){
			glGetShaderInfoLog(program, 512, NULL, infoLog);
			std::cerr << error_msg << std::endl << infoLog << std::endl;
			exit(1);
			return false;
		}
	}
	else{
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if(!success){
			glGetProgramInfoLog(program, 512,NULL, infoLog);
			std::cerr << error_msg << std::endl << infoLog << std::endl;
			return false;
		}
	}

	return true;
}


void Shader::set(const std::string& name, bool p) {
    glUniform1i(glGetUniformLocation(program, name.c_str()), p);
}

void Shader::set(const std::string& name, int p) {
    glUniform1i(glGetUniformLocation(program, name.c_str()), p);
}


void Shader::set(const std::string& name, float p) {
    glUniform1f(glGetUniformLocation(program, name.c_str()), p);
}

void Shader::set(const std::string& name, const glm::vec2& p) {
    glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(p));
}

void Shader::set(const std::string& name, const glm::vec3& p) {
    glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(p));
}

void Shader::set(const std::string& name, const glm::ivec3& p) {
    glUniform3iv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(p));
}

void Shader::set(const std::string& name, const glm::mat4& p) {
    glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(p));
}

void Shader::set(const std::string& name, const glm::mat3& p) {
    glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(p));
}

void Shader::setv(const std::string& name, int* p, int count) {
  	glUniform1iv(glGetUniformLocation(program, name.c_str()), count, p);
}

void Shader::setv(const std::string& name, const glm::ivec2* p, int count) {
  	glUniform2iv(glGetUniformLocation(program, name.c_str()), count, glm::value_ptr(*p));
}

void Shader::setv(const std::string& name, const glm::vec3* p, int count) {
  	glUniform3fv(glGetUniformLocation(program, name.c_str()), count, glm::value_ptr(*p));
}






