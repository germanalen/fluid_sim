#ifndef SIM_H
#define SIM_H

#include <glm/glm.hpp>
#include <vector>
#include <list>
#include <functional>

class Sim {
public:
    enum CellType {
        SOLID,
        AIR,
        FLUID
    };


    Sim(int grid_size);
    ~Sim();

    void update();

    const std::vector<float>& update_voxels(int type);
    const std::vector<glm::vec3>& update_vecfield_dirs();

    const std::list<glm::vec3>& get_particles() const { return particles; }
    const std::vector<float>& get_voxels() const { return voxels; }
    const std::vector<glm::vec3>& get_vecfield_dirs() const { return vecfield_dirs; }
private:
    void extrapolate(float ***f, const glm::ivec3& f_shape, 
                     std::function<bool (int x, int y, int z)> is_defined);
    void extrapolate_uvw();
    void advect(float ***f0, float ***f1, const glm::ivec3& f_shape, const glm::vec3& offset);
    void project();
    void clear(); // sets velocities in air and solids to 0

    float div(int i, int j, int k);
    void a_dot(float ***x, float ***ax); // ax = A*x for conjugate gradient algorithm
    float v_dot(float ***v1, float ***v2); // v1.T*v2
    void v_assign(float ***v, float ***v1); // v = v1
    void v_add(float ***v, float ***v1, float s, float ***v2); // v=v1+s*v2

    // Not defined for faces of edge cells (x,y,z = .5 or x,y,z = grid_size-1.5)
    float interp(float ***f, const glm::vec3& pos, const glm::vec3& offset) const;
    glm::vec3 interp_uvw(const glm::vec3& pos) const;

    inline float& um(int i, int j, int k) { return u[i-1][j][k]; }
    inline float& up(int i, int j, int k) { return u[i][j][k]; }
    inline float& vm(int i, int j, int k) { return v[i][j-1][k]; }
    inline float& vp(int i, int j, int k) { return v[i][j][k]; }
    inline float& wm(int i, int j, int k) { return w[i][j][k-1]; }
    inline float& wp(int i, int j, int k) { return w[i][j][k]; }

    inline float& pum(int i, int j, int k) { return p[i][j][k]; }
    inline float& pup(int i, int j, int k) { return p[i+1][j][k]; }
    inline float& pvm(int i, int j, int k) { return p[i][j][k]; }
    inline float& pvp(int i, int j, int k) { return p[i][j+1][k]; }
    inline float& pwm(int i, int j, int k) { return p[i][j][k]; }
    inline float& pwp(int i, int j, int k) { return p[i][j][k+1]; }

    inline CellType& cum(int i, int j, int k) { return c[i][j][k]; }
    inline CellType& cup(int i, int j, int k) { return c[i+1][j][k]; }
    inline CellType& cvm(int i, int j, int k) { return c[i][j][k]; }
    inline CellType& cvp(int i, int j, int k) { return c[i][j+1][k]; }
    inline CellType& cwm(int i, int j, int k) { return c[i][j][k]; }
    inline CellType& cwp(int i, int j, int k) { return c[i][j][k+1]; }

    template<typename T>
    static T*** arr_new(const glm::ivec3& shape, T v);
    template<typename T>
    static void arr_delete(T*** a, const glm::ivec3& shape);
    template<typename T>
    static void arr_fill(T*** a, int i0, int i1, int j0, int j1, int k0, int k1, T v);


    const float dx = 1, dt = 1/60.0/1.0;
    const float rho = 1;
    int grid_size;

    glm::ivec3 grid_shape; 

    float ***u, ***v, ***w;    
    glm::ivec3 u_shape, v_shape, w_shape;

    CellType ***c;

    std::list<glm::vec3> particles;

    // temp variables
    float ***p;
    float ***a_diag, ***a_pi, ***a_pj, ***a_pk;
    float ***x, ***ax, ***r, ***b, ***d, ***ad;
    float ***tu, ***tv, ***tw;

    float ***dens;

    // visualization stuff
    std::vector<float> voxels;
    std::vector<glm::vec3> vecfield_dirs;
};

#endif
