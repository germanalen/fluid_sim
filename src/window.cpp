#include "window.h"
#include <stdexcept>
#include <iostream>


Window::Window(std::string const& title, glm::ivec2 const& size) {
    //////////////////////////////////////////////////
    if (SDL_Init(SDL_INIT_VIDEO) < 0) throw std::runtime_error("SDL_Init failed");

    window = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            size.x,
            size.y,
            SDL_WINDOW_OPENGL
    );

    if (!window) throw std::runtime_error("SDL_CreateWindow failed");

    //////////////////////////////////////////////////
    sdl_check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
    sdl_check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    sdl_check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3));

    sdl_check(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1));

    
    context = SDL_GL_CreateContext(window);
    
    sdl_check(SDL_GL_SetSwapInterval(0));

    //////////////////////////////////////////////////
    if (GLEW_OK != glewInit()) throw std::runtime_error("GLEW INIT Error");
}

Window::~Window() {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


void Window::sdl_check(int code) {
    if (code != 0) std::cerr << SDL_GetError() << std::endl;
}


void Window::save_screenshot(const std::string& filepath) {
    SDL_Renderer *renderer = SDL_GetRenderer(window);


    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    SDL_Surface *surface = SDL_CreateRGBSurface(0,width,height,32,
                                                0xff000000,
                                                0x00ff0000,
                                                0x0000ff00,
                                                0x000000ff);

    SDL_LockSurface(surface);
    SDL_RenderReadPixels(renderer,NULL,surface->format->format,
                         surface->pixels,surface->pitch);

    SDL_SaveBMP(surface, filepath.c_str());
    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);
}







