#ifndef __SIMULATION_HPP
#define __SIMULATION_HPP

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <assert.h>
#include <algorithm>

#include "hesp.hpp"
#include "Parameters.hpp"
#include "Particle.hpp"

#if defined(GLFW3)
#include <GLFW/glfw3.h>
#else
#define GLFW_INCLUDE_GL3
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <GL/glfw.h>
#endif // GLFW3


// Macro used for the end of cell list
static const int END_OF_CELL_LIST = -1;


using std::map;
using std::vector;
using std::string;


/**
 *  \brief CParser
 */
class Simulation
{
private:
    // Avoid copy
    Simulation &operator=(const Simulation &other);
    Simulation (const Simulation &other);

public:

    /**
     *  \brief  Default constructor.
     */
    explicit Simulation(const ConfigParameters &parameters,
                        const vector<Particle> &particles,
                        const map<string, cl::Kernel> kernels,
                        const cl::Context &clContext,
                        const cl::Device &clDevice,
                        const GLuint sharingBufferID);

    /**
     *  \brief  Destructor.
     */
    ~Simulation ();

    void init(void);
    void initCells(void);
    void step(void);

    // Copy current positions and velocities
    void dumpData( hesp_float4 * (&positions),
                   hesp_float4 * (&velocities) );

    cl_uint
    getNumberParticles(void) const
    {
        return mNumParticles;
    }

    hesp_float
    getSizeXmin(void) const
    {
        return mSystemSizeMin.s[0];
    }

    hesp_float
    getSizeXmax(void) const
    {
        return mSystemSizeMax.s[0];
    }

    hesp_float
    getSizeYmin(void) const
    {
        return mSystemSizeMin.s[1];
    }

    hesp_float
    getSizeYmax(void) const
    {
        return mSystemSizeMax.s[1];
    }

    hesp_float
    getSizeZmin(void) const
    {
        return mSystemSizeMin.s[2];
    }

    hesp_float
    getSizeZmax(void) const
    {
        return mSystemSizeMax.s[2];
    }

    const hesp_float4
    getSizesMin(void) const
    {
        return mSystemSizeMin;
    }

    const hesp_float4
    getSizesMax(void) const
    {
        return mSystemSizeMax;
    }

    // Setter

    void
    setWaveGenerator(const double value)
    {
        mWaveGenerator = value;
    }

private:

    // Sizes of domain
    hesp_float4 mSystemSizeMin;
    hesp_float4 mSystemSizeMax;

    // OpenCL objects supplied by OpenCL setup
    const cl::Context &mCLContext;
    const cl::Device &mCLDevice;

    // holds all OpenCL kernels required for the simulation
    map<string, cl::Kernel> mKernels;

    // command queue all OpenCL calls are run on
    cl::CommandQueue mQueue;

    // ranges used for executing the kernels
    cl::NDRange mGlobalRange;
    cl::NDRange mLocalRange;

    // configuration parameters for the simulation
    hesp_float mTimestepLength;
    hesp_float mTimeEnd;

    const cl_uint mNumParticles;

    const size_t mBufferSizeParticles;
    const size_t mBufferSizeCells;
    const size_t mBufferSizeParticlesList;
    const size_t mBufferSizeScalingFactors;

    // Reference to a vector of particles to setup data arrays in OpenCL
    const vector<Particle> &mParticles;

    // The host memory holding the simulation data
    hesp_float4 *mPositions;
    hesp_float4 *mPredicted;
    hesp_float4 *mVelocities;
    hesp_float4 *mDelta;
    hesp_float4 *mDeltaVelocity;
    hesp_float *mScalingFactors;
    hesp_float4 *mVorticityForces;
#if !defined(USE_LINKEDCELL)
    cl_uint2 *mRadixCells;
#endif // USE_LINKEDCELL

    // The device memory buffers holding the simulation data
    cl::Buffer mCellsBuffer;
    cl::Buffer mParticlesListBuffer;
    cl::Buffer mPositionsBuffer;
    cl::Buffer mPredictedBuffer;
    cl::Buffer mVelocitiesBuffer;
    cl::Buffer mScalingFactorsBuffer;
    cl::Buffer mDeltaBuffer;
    cl::Buffer mDeltaVelocityBuffer;
    cl::Buffer mVorticityBuffer;

#if !defined(USE_LINKEDCELL)
    cl::Buffer mRadixCellsBuffer;
    cl::Buffer mRadixHistogramBuffer;
    cl::Buffer mRadixGlobSumBuffer;
    cl::Buffer mRadixCellsOutBuffer;
    cl::Buffer mFoundCellsBuffer;
#endif // USE_LINKEDCELL

    // Number of cells in each direction
    cl_int4 mNumberCells;

    // Lengths of each cell in each direction
    hesp_float4 mCellLength;

    // Array for the cells
    cl_int *mCells;
    cl_int *mParticlesList;

    // Rest density
    const hesp_float mRestDens;

    // For generating waves
    hesp_float mWaveGenerator;

    GLuint mSharingBufferID;

    // Private member functions
    void updateCells(void);
    void updatePositions(void);
    void updateVelocities(void);
    void applyVorticityAndViscosity(void);
    void predictPositions(void);
    void updatePredicted(void);
    void computeScaling(void);
    void computeDelta(void);
#if !defined(USE_LINKEDCELL)
    void radix(void);
#endif // USE_LINKEDCELL

};

#endif // __SIMULATION_HPP
