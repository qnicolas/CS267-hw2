#include "common.h"
#include <cmath>
#include <iostream>

// Apply the force from neighbor to particle
void apply_force(particle_t& particle, particle_t& neighbor) {
    // Calculate Distance
    double dx = neighbor.x - particle.x;
    double dy = neighbor.y - particle.y;
    double r2 = dx * dx + dy * dy;

    // Check if the two particles should interact
    if (r2 > cutoff * cutoff)
        return;

    r2 = fmax(r2, min_r * min_r);
    double r = sqrt(r2);

    // Very simple short-range repulsive force
    double coef = (1 - cutoff / r) / r2 / mass;
    particle.ax += coef * dx;
    particle.ay += coef * dy;
}

// Integrate the ODE
void move(particle_t& p, double size) {
    // Slightly simplified Velocity Verlet integration
    // Conserves energy better than explicit Euler method
    p.vx += p.ax * dt;
    p.vy += p.ay * dt;
    p.x += p.vx * dt;
    p.y += p.vy * dt;

    // Bounce from walls
    while (p.x < 0 || p.x > size) {
        p.x = p.x < 0 ? -p.x : 2 * size - p.x;
        p.vx = -p.vx;
    }

    while (p.y < 0 || p.y > size) {
        p.y = p.y < 0 ? -p.y : 2 * size - p.y;
        p.vy = -p.vy;
    }
}

static int nbinsx;
static particle_t*** bins;
static int** bin_particlecount;
static double dxbin;

void init_simulation(particle_t* parts, int num_parts, double size) {
    nbinsx = (int)((double) size / (cutoff)) + 1;
    //static const int nbins = nbinsx*nbinsx;
    // bins = (particle_t***) malloc(nbinsx * nbinsx * num_parts * sizeof(particle_t));
    // bin_particlecount = (int**) malloc(nbinsx * nbinsx * sizeof(int)); 
    
    /// ALLOCATE MEMORY ///
    bins = new particle_t**[nbinsx];
    for (int ib = 0; ib < nbinsx; ++ib) {
        bins[ib] = new particle_t*[nbinsx];
        for (int jb = 0; jb < nbinsx; ++jb) {
            bins[ib][jb] = new particle_t[num_parts];
        }
    }
    bin_particlecount = new int*[nbinsx];
    for (int ib = 0; ib < nbinsx; ++ib) {
        bin_particlecount[ib] = new int[nbinsx];
    }
    
    /// INITIALIZE ///
    dxbin = size / (double) nbinsx;
    std::cout << nbinsx << std::endl;
    // populate bins 
    for (int i = 0; i < num_parts; ++i) {
        int ib = (int)(parts[i].x / dxbin);
        int jb = (int)(parts[i].y / dxbin);
        bins[ib][jb] = &parts[i];
        bin_particlecount[ib][jb]++;
    }
}

void simulate_one_step(particle_t* parts, int num_parts, double size) {
    // Compute Forces
    for (int ib = 0; ib < nbinsx; ++ib) {
        for (int jb = 0; jb < nbinsx; ++jb) {
            particle_t* bin = bins[ib][jb];
            std::cout << "1" << std::endl;
            for (int i = 0; i < bin_particlecount[ib][jb]; ++i) {
                particle_t particle = bin[i];
                particle.ax = particle.ay = 0;
                for (int local_i = fmax(0,ib-1); local_i <= fmin(nbinsx-1,ib+1); ++local_i){
                    for (int local_j = fmax(0,jb-1); local_j <= fmin(nbinsx-1,jb+1); ++local_j){
                        particle_t* local_bin = bins[local_i][local_j];
                        for (int j = 0; j < bin_particlecount[local_i][local_j]; ++i) {
                            apply_force(particle, local_bin[j]);
                        }
                    }
                }
            }
        }
    }

    // Move Particles
    for (int i = 0; i < num_parts; ++i) {
        move(parts[i], size);
    }
    for (int ib = 0; ib < nbinsx; ++ib) {
        for (int jb = 0; jb < nbinsx; ++jb) {
            bin_particlecount[ib][jb]=0;
        }
    }
    // populate bins 
    for (int i = 0; i < num_parts; ++i) {
        int ib = (int)(parts[i].x / dxbin);
        int jb = (int)(parts[i].y / dxbin);
        bins[ib][jb] = &parts[i];
        bin_particlecount[ib][jb]++;
    }
}
