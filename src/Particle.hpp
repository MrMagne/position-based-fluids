#ifndef __PARTICLE_HPP
#define __PARTICLE_HPP

#include "hesp.hpp"

static const int DIMENSIONS = 3;


struct Particle {
    hesp_float m; /**< Mass */
    hesp_float x[DIMENSIONS]; /**< Position X */
    hesp_float v[DIMENSIONS]; /**< Position Y */
    hesp_float a[DIMENSIONS]; /**< Position Z */
};

#endif // __PARTICLE_HPP
