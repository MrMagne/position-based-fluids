#ifndef __PARTICLE_HPP
#define __PARTICLE_HPP

#include "hesp.hpp"

static const int DIMENSIONS = 3;


struct Particle
{
    cl_float m; /**< Mass */
    cl_float x[DIMENSIONS]; /**< Position X */
    cl_float v[DIMENSIONS]; /**< Position Y */
    cl_float a[DIMENSIONS]; /**< Position Z */
};

#endif // __PARTICLE_HPP
