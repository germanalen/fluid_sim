#include "sim.h"
#include <iostream>
#include <glm/gtc/random.hpp>
#include "misc.h"

using std::cout;
using std::endl;


void print(float ***f, glm::ivec3 f_shape) {
    for (int i = 0; i < f_shape.x; ++i) {
        for (int j = 0; j < f_shape.y; ++j) {
            for (int k = 0; k < f_shape.z; ++k) {
                cout << f[i][j][k] << '|';
            }
            cout << endl;
        }
        cout << endl;
    }
    cout << "-----------------------------" << endl;
}



Sim::Sim(int grid_size) : 
    grid_size{grid_size},
    voxels(grid_size * grid_size * grid_size, 0),
    vecfield_dirs(1000)
{
    u_shape = {grid_size-1, grid_size, grid_size};
    v_shape = {grid_size, grid_size-1, grid_size};
    w_shape = {grid_size, grid_size, grid_size-1};

    u = arr_new(u_shape, 0.0f);
    v = arr_new(v_shape, 0.0f);
    w = arr_new(w_shape, 0.0f);


    for (int i = 1; i < 9; ++i) {
        for (int j = 1; j < 3; ++j) {
            for (int k = 1; k < 9; ++k) {
                um(i,j,k) = 1;
                //vm(j,i,k) = 1;
                //wm(j,k,i) = 1;
            }
        }
    }
    //um(7,7,7) = 1;


    grid_shape = {grid_size, grid_size, grid_size};
    c = arr_new(grid_shape, AIR);
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            c[0][i][j] = c[grid_size-1][i][j] = SOLID;
            c[i][0][j] = c[i][grid_size-1][j] = SOLID;
            c[i][j][0] = c[i][j][grid_size-1] = SOLID;
        }
    }
    arr_fill(c, 1,grid_size/2, int(grid_size*3/4.),int(grid_size*3/4.)+1, 1,grid_size-1, SOLID);
    arr_fill(c, int(grid_size*.3), int(grid_size*.7), int(grid_size/2.), int(grid_size/2.)+1, 1,grid_size-1, SOLID);
    arr_fill(c, 1,grid_size/2, int(grid_size*3/4.)+1,grid_size-1, 1,grid_size-1, FLUID);
    
    //arr_fill(c, 1, grid_size/2, 1, grid_size-1, 1, grid_size/2, FLUID);
    //arr_fill(c,6,14,10,18,6,14,FLUID);
    //arr_fill(c,7,13,5,19,7,13,FLUID);
    //arr_fill(c,1,19,1,3,1,19,FLUID);
    //arr_fill(c, 1,4,1,19,10,14,SOLID);
    //arr_fill(c, 5,15,13,18,5,15,FLUID);
    
    //c[1][8][5] = FLUID;


    for (int i = 1; i < grid_size - 1; ++i) {
        for (int j = 1; j < grid_size - 1; ++j) {
            for (int k = 1; k < grid_size - 1; ++k) {
                if (c[i][j][k] == FLUID) {
                    for (int p = 0; p < 8; ++p) {
                        glm::vec3 pos = glm::vec3(i,j,k) + glm::linearRand(glm::vec3(-.5), glm::vec3(.5));
                        particles.push_back(pos);
                    }
                }
            }
        }
    }

    p = arr_new(grid_shape, 0.0f);
    a_diag = arr_new(grid_shape, 0.0f);
    a_pi = arr_new(grid_shape, 0.0f);
    a_pj = arr_new(grid_shape, 0.0f);
    a_pk = arr_new(grid_shape, 0.0f);
    
    x = arr_new(grid_shape, 0.0f);
    ax = arr_new(grid_shape, 0.0f);
    r = arr_new(grid_shape, 0.0f);
    b = arr_new(grid_shape, 0.0f);
    d = arr_new(grid_shape, 0.0f);
    ad = arr_new(grid_shape, 0.0f);
    tu = arr_new(u_shape, 0.0f);
    tv = arr_new(v_shape, 0.0f);
    tw = arr_new(w_shape, 0.0f);

    dens = arr_new(grid_shape, 0.0f);
    for (int i = 3; i < 4; ++i) {
        for (int j = 1; j < 3; ++j) {
            for (int k = 3; k < 4; ++k) {
                dens[i][j][k] = 10;
            }
        }
    }



    clear();
    //for (int i = 0; i < 1000; ++i) {
    //    project();
    //    clear();
    //}
}


Sim::~Sim() {
    arr_delete(u, u_shape);
    arr_delete(v, v_shape);
    arr_delete(w, w_shape);
    arr_delete(c, grid_shape);
    arr_delete(p, grid_shape);
    arr_delete(a_diag, grid_shape);
    arr_delete(a_pi, grid_shape);
    arr_delete(a_pj, grid_shape);
    arr_delete(a_pk, grid_shape);
}


void Sim::update() {
    extrapolate_uvw();

    float eps = 1e-6;
    for (glm::vec3& p : particles) {
        glm::vec3 vel = interp_uvw(p);
        p = glm::clamp(p + vel * dt, glm::vec3(.5 + eps), glm::vec3(grid_size - 1.5 - eps));
    }

    for (auto i = particles.begin(); i != particles.end(); ) {
        glm::ivec3 coord(glm::round(*i));
        if (c[coord.x][coord.y][coord.z] == SOLID) {
            i = particles.erase(i);
        } else {
            ++i;
        }
    }
    
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            for (int k = 0; k < grid_size; ++k) {
                if (c[i][j][k] == FLUID)
                    c[i][j][k] = AIR;
            }
        }
    }

    for (glm::vec3& p : particles) {
        glm::ivec3 coord(glm::round(p));
        int i = coord.x;
        int j = coord.y;
        int k = coord.z;
        if (c[i][j][k] == AIR) {
            c[i][j][k] = FLUID;
        }
    }



    //advect(dens, glm::ivec3(10), glm::vec3(0));

    advect(u, tu, u_shape, glm::vec3(-.5,0,0));
    advect(v, tv, v_shape, glm::vec3(0,-.5,0));
    advect(w, tw, w_shape, glm::vec3(0,0,-.5));

    for (int i = 0; i < u_shape.x; ++i) {
        for (int j = 0; j < u_shape.y; ++j) {
            for (int k = 0; k < u_shape.z; ++k) {
                u[i][j][k] = tu[i][j][k];
            }
        }
    }

    for (int i = 0; i < v_shape.x; ++i) {
        for (int j = 0; j < v_shape.y; ++j) {
            for (int k = 0; k < v_shape.z; ++k) {
                v[i][j][k] = tv[i][j][k] - 9.8f * dt;
            }
        }
    }

    for (int i = 0; i < w_shape.x; ++i) {
        for (int j = 0; j < w_shape.y; ++j) {
            for (int k = 0; k < w_shape.z; ++k) {
                w[i][j][k] = tw[i][j][k];
            }
        }
    }

    clear();
    project();
    clear();

}


const std::vector<float>& Sim::update_voxels(int type) {
    for (int x = 0; x < grid_size; ++x) {
        for (int y = 0; y < grid_size; ++y) {
            for (int z = 0; z < grid_size; ++z) {
                float v = 0;

                switch (type) {
                    case 0: 
                        if (c[x][y][z] == FLUID) {
                            v = div(x,y,z);
                        }
                        break;
                    case 1:
                        v = p[x][y][z] * .005;
                        break;
                    case 2:
                        //if (c[x][y][z] == FLUID) v = -1;
                        //if (c[x][y][z] == AIR) v = 1;
                        if (0 < x && x < grid_size-1 && 0 < y && y < grid_size-1 && 0 < z && z < grid_size-1)
                            if (c[x][y][z] == SOLID) v = -1;
                        break;
                    case 3:
                        v = glm::clamp(dens[x][y][z], 0.0f, 1.0f);
                        break;
                }

                voxels[(x * grid_size + y) * grid_size + z] = v;
            }
        }
        //voxels[(0 * grid_size + 0) * grid_size + x] = 1;
    }
    return voxels;
}


const std::vector<glm::vec3>& Sim::update_vecfield_dirs() {
    assert(grid_size == 10);
    for (int x = 0; x < grid_size; ++x) {
        for (int y = 0; y < grid_size; ++y) {
            for (int z = 0; z < grid_size; ++z) {
                glm::vec3 dir(0);
                if (c[x][y][z] == FLUID || c[x][y][z] == AIR) {
                    dir.x = (um(x,y,z) + up(x,y,z)) * .5;
                    dir.y = (vm(x,y,z) + vp(x,y,z)) * .5;
                    dir.z = (wm(x,y,z) + wp(x,y,z)) * .5;
                }
                dir = dir / glm::sqrt(glm::length(dir));

                vecfield_dirs[(x * grid_size + y) * grid_size + z] = dir;
            }
        }
    }
    return vecfield_dirs;
}


// biased
void Sim::extrapolate(float ***f, const glm::ivec3& f_shape, 
                      std::function<bool (int i, int j, int k)> is_defined) {
    for (int i = 1; i < f_shape.x - 1; ++i) {
        for (int j = 1; j < f_shape.y - 1; ++j) {
            for (int k = 1; k < f_shape.z - 1; ++k) {
                if (is_defined(i,j,k) && !is_defined(i-1,j,k))
                    f[i-1][j][k] = f[i][j][k];
                if (is_defined(i,j,k) && !is_defined(i+1,j,k))
                    f[i+1][j][k] = f[i][j][k];
                if (is_defined(i,j,k) && !is_defined(i,j-1,k))
                    f[i][j-1][k] = f[i][j][k];
                if (is_defined(i,j,k) && !is_defined(i,j+1,k))
                    f[i][j+1][k] = f[i][j][k];
                if (is_defined(i,j,k) && !is_defined(i,j,k-1))
                    f[i][j][k-1] = f[i][j][k];
                if (is_defined(i,j,k) && !is_defined(i,j,k+1))
                    f[i][j][k+1] = f[i][j][k];
            }
        }
    }
}


void Sim::extrapolate_uvw() {
    extrapolate(u, u_shape, 
            [&](int i, int j, int k){
                return cum(i,j,k) == FLUID || cup(i,j,k) == FLUID; 
            });
    extrapolate(v, v_shape, 
            [&](int i, int j, int k){
                return cvm(i,j,k) == FLUID || cvp(i,j,k) == FLUID; 
            });
    extrapolate(w, w_shape, 
            [&](int i, int j, int k){
                return cwm(i,j,k) == FLUID || cwp(i,j,k) == FLUID; 
            });
}


void Sim::advect(float ***f0, float ***f1, const glm::ivec3& f_shape, const glm::vec3& offset) {
    for (int i = 1; i < f_shape.x - 1; ++i) {
        for (int j = 1; j < f_shape.y - 1; ++j) {
            for (int k = 1; k < f_shape.z - 1; ++k) {
                glm::vec3 pos = glm::vec3(i,j,k) - offset;
                glm::vec3 from = pos - dt * interp_uvw(pos) * 1.0f;
                f1[i][j][k] = interp(f0, from, offset);
            }
        }
    }
}


void Sim::project() {
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            for (int k = 0; k < grid_size; ++k) {
                a_diag[i][j][k] = 0;
                a_pi[i][j][k] = 0;
                a_pj[i][j][k] = 0;
                a_pk[i][j][k] = 0;
                if (c[i][j][k] == FLUID) {
                    if (c[i-1][j][k] != SOLID)
                        a_diag[i][j][k] += 1;
                    if (c[i+1][j][k] != SOLID)
                        a_diag[i][j][k] += 1;
                    if (c[i][j-1][k] != SOLID)
                        a_diag[i][j][k] += 1;
                    if (c[i][j+1][k] != SOLID)
                        a_diag[i][j][k] += 1;
                    if (c[i][j][k-1] != SOLID)
                        a_diag[i][j][k] += 1;
                    if (c[i][j][k+1] != SOLID)
                        a_diag[i][j][k] += 1;

                    if (c[i+1][j][k] == FLUID)
                        a_pi[i][j][k] = -1;
                    if (c[i][j+1][k] == FLUID)
                        a_pj[i][j][k] = -1;
                    if (c[i][j][k+1] == FLUID)
                        a_pk[i][j][k] = -1;
                }

                b[i][j][k] = 0;
                if (c[i][j][k] == FLUID)
                    b[i][j][k] = -(dx * dx * dx * rho / dt) * div(i,j,k);
            }
        }
    }



    a_dot(x, ax);
    
    v_add(r,b,-1,ax);
    v_assign(d,r);

    float delta_new = v_dot(r,r);
    for (int i = 0; i < 100; ++i) {
        if (delta_new < 1e-5)
            break;

        a_dot(d, ad);

        float alpha = delta_new/v_dot(d, ad);
        v_add(x,x,alpha,d);

        a_dot(x, ax);
        v_add(r,b,-1,ax);

        float delta_old = delta_new;
        delta_new = v_dot(r,r);
        float beta = delta_new/delta_old;

        v_add(d,r,beta,d);
        
        // if diverging (liquid completely surrounded by solid)
        if (delta_new > delta_old) {
            break;
        }
    }
    
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            for (int k = 0; k < grid_size; ++k) {
                if (c[i][j][k] == FLUID)
                    p[i][j][k] = x[i][j][k];
                else
                    p[i][j][k] = 0;
            }
        }
    }



    // update u,v,w
    for (int i = 1; i < grid_size-1; ++i) {
        for (int j = 1; j < grid_size-1; ++j) {
            for (int k = 1; k < grid_size-1; ++k) {
                if (cum(i,j,k) == SOLID || cup(i,j,k) == SOLID)
                    u[i][j][k] = 0;
                else
                    u[i][j][k] -= dt / (rho * dx) * (pup(i,j,k) - pum(i,j,k));
                
                if (cvm(i,j,k) == SOLID || cvp(i,j,k) == SOLID)
                    v[i][j][k] = 0;
                else
                    v[i][j][k] -= dt / (rho * dx) * (pvp(i,j,k) - pvm(i,j,k));
                
                if (cwm(i,j,k) == SOLID || cwp(i,j,k) == SOLID)
                    w[i][j][k] = 0;
                else
                    w[i][j][k] -= dt / (rho * dx) * (pwp(i,j,k) - pwm(i,j,k));
            }
        }
    }
    
}


void Sim::clear() {
    for (int i = 1; i < grid_size - 1; ++i) {
        for (int j = 1; j < grid_size - 1; ++j) {
            for (int k = 1; k < grid_size - 1; ++k) {
                if (c[i][j][k] != FLUID) {
                    if (c[i-1][j][k] != FLUID)
                        um(i,j,k) = 0;
                    if (c[i+1][j][k] != FLUID)
                        up(i,j,k) = 0;
                    if (c[i][j-1][k] != FLUID)
                        vm(i,j,k) = 0;
                    if (c[i][j+1][k] != FLUID)
                        vp(i,j,k) = 0;
                    if (c[i][j][k-1] != FLUID)
                        wm(i,j,k) = 0;
                    if (c[i][j][k+1] != FLUID)
                        wp(i,j,k) = 0;
                }
            }
        }
    }

    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            up(0,i,j) = um(grid_size-1,i,j) = 0;
            vp(i,0,j) = vm(i,grid_size-1,j) = 0;
            wp(i,j,0) = wm(i,j,grid_size-1) = 0;
        }
    }
    
}


inline float Sim::div(int i, int j, int k) {
    assert(c[i][j][k] == FLUID);
    return (up(i,j,k) - um(i,j,k))/dx + (vp(i,j,k) - vm(i,j,k))/dx + (wp(i,j,k) - wm(i,j,k))/dx;
}




void Sim::a_dot(float ***x, float ***ax) {
    for (int i = 1; i < grid_size-1; ++i) {
        for (int j = 1; j < grid_size-1; ++j) {
            for (int k = 1; k < grid_size-1; ++k) {
                if (c[i][j][k] == FLUID) {
                    ax[i][j][k] = a_diag[i][j][k] * x[i][j][k]
                        + a_pi[i][j][k]   * x[i+1][j][k]
                        + a_pi[i-1][j][k] * x[i-1][j][k]
                        + a_pj[i][j][k]   * x[i][j+1][k]
                        + a_pj[i][j-1][k] * x[i][j-1][k]
                        + a_pk[i][j][k]   * x[i][j][k+1]
                        + a_pk[i][j][k-1] * x[i][j][k-1];
                }
            }
        }
    }
}


float Sim::v_dot(float ***v1, float ***v2) {
    float res = 0;
    for (int i = 1; i < grid_size-1; ++i) {
        for (int j = 1; j < grid_size-1; ++j) {
            for (int k = 1; k < grid_size-1; ++k) {
                if (c[i][j][k] == FLUID) {
                    res += v1[i][j][k] * v2[i][j][k];
                }
            }
        }
    }
    return res;
}


void Sim::v_assign(float ***v, float ***v1) {
    for (int i = 1; i < grid_size-1; ++i) {
        for (int j = 1; j < grid_size-1; ++j) {
            for (int k = 1; k < grid_size-1; ++k) {
                if (c[i][j][k] == FLUID) {
                    v[i][j][k] = v1[i][j][k];
                }
            }
        }
    }
}


void Sim::v_add(float ***v, float ***v1, float s, float ***v2) {
    for (int i = 1; i < grid_size-1; ++i) {
        for (int j = 1; j < grid_size-1; ++j) {
            for (int k = 1; k < grid_size-1; ++k) {
                if (c[i][j][k] == FLUID) {
                    v[i][j][k] = v1[i][j][k] + s * v2[i][j][k];
                }
            }
        }
    }
}


float Sim::interp(float ***f, const glm::vec3& pos, const glm::vec3& offset) const {
    assert(.5 < pos.x && pos.x < grid_size - 1.5);
    assert(.5 < pos.y && pos.y < grid_size - 1.5);
    assert(.5 < pos.z && pos.z < grid_size - 1.5);
    
    glm::vec3 d = glm::fract(pos + offset);
    glm::ivec3 coord(pos + offset);

    float f000 = f[coord.x][coord.y][coord.z];
    float f001 = f[coord.x][coord.y][coord.z+1];
    float f010 = f[coord.x][coord.y+1][coord.z];
    float f011 = f[coord.x][coord.y+1][coord.z+1];
    float f100 = f[coord.x+1][coord.y][coord.z];
    float f101 = f[coord.x+1][coord.y][coord.z+1];
    float f110 = f[coord.x+1][coord.y+1][coord.z];
    float f111 = f[coord.x+1][coord.y+1][coord.z+1];

    float f00 = f000*(1-d.x) + f100*d.x;
    float f01 = f001*(1-d.x) + f101*d.x;
    float f10 = f010*(1-d.x) + f110*d.x;
    float f11 = f011*(1-d.x) + f111*d.x;

    float f0 = f00*(1-d.y) + f10*d.y;
    float f1 = f01*(1-d.y) + f11*d.y;

    float f_ = f0*(1-d.z) + f1*d.z;

    return f_;
}

glm::vec3 Sim::interp_uvw(const glm::vec3& pos) const {
    glm::ivec3 coord(glm::round(pos));
    assert(.5 < pos.x && pos.x < grid_size - 1.5);
    assert(.5 < pos.y && pos.y < grid_size - 1.5);
    assert(.5 < pos.z && pos.z < grid_size - 1.5);
    //assert(c[coord.x][coord.y][coord.z] == FLUID);

    glm::vec3 uvw;
    uvw.x = interp(u, pos, glm::vec3(-.5,0,0));
    uvw.y = interp(v, pos, glm::vec3(0,-.5,0));
    uvw.z = interp(w, pos, glm::vec3(0,0,-.5));
    return uvw;
}


template<typename T>
T*** Sim::arr_new(const glm::ivec3& shape, T v) {
    T*** a = new T**[shape.x];
    for (int x = 0; x < shape.x; ++x) {
        a[x] = new T*[shape.y];
        for (int y = 0; y < shape.y; ++y) {
            a[x][y] = new T[shape.z];
            for (int z = 0; z < shape.z; ++z) {
                a[x][y][z] = v;
            }
        }
    }
    return a;
}

template<typename T>
void Sim::arr_delete(T*** a, const glm::ivec3& shape) {
    for (int x = 0; x < shape.x; ++x) {
        for (int y = 0; y < shape.y; ++y) {
            delete[] a[x][y];
        }
        delete[] a[x];
    }
    delete[] a;
}



template<typename T>
void Sim::arr_fill(T*** a, int i0, int i1, int j0, int j1, int k0, int k1, T v) {
    for (int i = i0; i < i1; ++i) {
        for (int j = j0; j < j1; ++j) {
            for (int k = k0; k < k1; ++k) {
                a[i][j][k] = v;
            }
        }
    }
}













