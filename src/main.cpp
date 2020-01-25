#include <iostream>
#include <fstream>
#include <cstdio>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include "window.h"
#include "voxel_renderer.h"
#include "particle_renderer.h"
#include "sim.h"
#include "camera.h"
#include "misc.h"
#include <map>

using namespace std;


void write_particles(std::ofstream& fout, const std::vector<glm::vec3>& particles) {
    int size = particles.size();
    fout.write((char*)&size, 4);
    for (const glm::vec3& p : particles) {
        fout.write((char*)&p.x, 4);
        fout.write((char*)&p.y, 4);
        fout.write((char*)&p.z, 4);
    }
}

std::vector<glm::vec3> read_particles(std::ifstream& fin) {
    if (fin.eof()) {
        fin.clear();
        fin.seekg(0);
    }

    char buf[4];

    int size;
    fin.read(buf, 4);
    memcpy(&size, buf, 4);
    

    std::vector<glm::vec3> particles(size);

    for (int i = 0; i < size; ++i) {
        glm::vec3& p = particles[i];
        fin.read(buf, 4);
        memcpy(&p.x, buf, 4);
        fin.read(buf, 4);
        memcpy(&p.y, buf, 4);
        fin.read(buf, 4);
        memcpy(&p.z, buf, 4);
    }

    return particles;
}


int main() {
    glm::ivec2 screen_size{2*640, 2*480};
    int grid_size = 50;
    Window window("Liquid Sim", screen_size);    
    VoxelRenderer voxel_renderer(grid_size, screen_size);
    ParticleRenderer particle_renderer(grid_size, 1./grid_size, screen_size);
    Sim sim(grid_size);
    voxel_renderer.set_voxels(sim.update_voxels(2));

    Camera camera;
    float camera_dist = 3;
    camera.orbit(1200, 500, camera_dist, {0,0,0});
    SDL_SetRelativeMouseMode(SDL_TRUE);

    GLubyte screenshot[screen_size.x * screen_size.y * 3];


    bool running  = true;
    int voxel_type = 2;

    //std::ifstream fin("rec1.bin", ios::binary);
    std::ofstream fout("rec1.bin", ios::binary);

    int frame = 0;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEMOTION) {
                camera.orbit(event.motion.xrel, event.motion.yrel, camera_dist, {0,0,0});
            } else if (event.type == SDL_MOUSEWHEEL) {
                camera_dist = glm::clamp(camera_dist - event.wheel.y * .1f, 1.5f, 3.0f);
                camera.orbit(0, 0, camera_dist, {0,0,0});
            }
        }

        
        glm::mat4 proj = glm::perspective(glm::radians(90.0f), float(screen_size.x)/screen_size.y, 0.1f, 10.0f);
        const glm::mat4& view = camera.get_view();
        glm::mat4 model(1.0);
        model = glm::scale(model, glm::vec3(2));




        std::vector<glm::vec3> particles;

        sim.update();
        for (glm::vec3 p : sim.get_particles())
            particles.push_back(p);
        write_particles(fout, particles);

        //particles = read_particles(fin);

        particle_renderer.set_offsets(particles);




        glClearColor(.4, .4, .4, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);


        voxel_renderer.render(proj, view, model);
        glClear(GL_DEPTH_BUFFER_BIT);
        //vecfield_renderer.render_arrow(proj, view, model, debug_vec, sim.interp_uvw(debug_vec_grid));
        //glClear(GL_DEPTH_BUFFER_BIT);
        particle_renderer.render(proj, view, model, voxel_renderer.get_depth_texture(), voxel_renderer.get_normal_texture());
        window.swap();

        //if (frame % 2 == 0) {
        //    glReadPixels(0, 0, screen_size.x, screen_size.y, GL_BGR, GL_UNSIGNED_BYTE, screenshot);
        //    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
        //            screenshot, screen_size.x, screen_size.y, 24, 3*screen_size.x,
        //            0x00000000, 0x00000000, 0x00000000, 0x00000000);
        //    //std::ostringstream sin;
        //    //sin << "output/" << std::setfill('0') << std::setw(5) << frame/2 << ".bmp";
        //    char filename[200];
        //    sprintf(filename, "output/%05i.bmp", frame/2);
        //    cout << filename << endl;
        //    SDL_SaveBMP(surface, filename);
        //    SDL_UnlockSurface(surface);
        //    SDL_FreeSurface(surface);
        //}
    
        ++frame;
        //std::cout << frame / 60.0 << std::endl;
    }



    return 0;
}
