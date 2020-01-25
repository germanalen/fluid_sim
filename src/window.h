#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

class Window {
public:
    Window(std::string const& title, glm::ivec2 const& size);
    ~Window();

    void swap() { SDL_GL_SwapWindow(window); }

    void save_screenshot(const std::string& filepath);
private:
    static void sdl_check(int code);

    SDL_Window* window;
    SDL_GLContext context;
};


#endif
