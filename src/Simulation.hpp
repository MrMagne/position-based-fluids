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

#include <GLFW/glfw3.h>

// Macro used for the end of cell list
static const int END_OF_CELL_LIST = -1;

using std::map;
using std::vector;
using std::string;


/**
*  \brief CParser
*/
class Simulation {
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
  void dumpData( cl_float4 * (&positions),
                 cl_float4 * (&velocities) );

  cl_uint getNumberParticles() const {
    return mNumParticles;
  }

  cl_float getSizeXmin() const {
    return mSystemSizeMin.s[0];
  }

  cl_float getSizeXmax() const {
    return mSystemSizeMax.s[0];
  }

  cl_float
  getSizeYmin(void) const {
    return mSystemSizeMin.s[1];
  }

  cl_float
  getSizeYmax(void) const {
    return mSystemSizeMax.s[1];
  }

  cl_float
  getSizeZmin(void) const {
    return mSystemSizeMin.s[2];
  }

  cl_float
  getSizeZmax(void) const {
    return mSystemSizeMax.s[2];
  }

  const cl_float4
  getSizesMin(void) const {
    return mSystemSizeMin;
  }

  const cl_float4
  getSizesMax(void) const {
    return mSystemSizeMax;
  }

  // Setter

  void
  setWaveGenerator(const double value) {
    mWaveGenerator = value;
  }

private:

  // Sizes of domain
  cl_float4 mSystemSizeMin;
  cl_float4 mSystemSizeMax;

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
  cl_float mTimestepLength;
  cl_float mTimeEnd;

  const cl_uint mNumParticles;

  const size_t mBufferSizeParticles;
  const size_t mBufferSizeCells;
  const size_t mBufferSizeParticlesList;
  const size_t mBufferSizeScalingFactors;

  // Reference to a vector of particles to setup data arrays in OpenCL
  const vector<Particle> &mParticles;

  // The host memory holding the simulation data
  cl_float4 *mPositions;
  cl_float4 *mVelocities;
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
  cl_float4 mCellLength;

  // Array for the cells
  cl_int *mCells;
  cl_int *mParticlesList;

  // Rest density
  const cl_float mRestDens;

  // For generating waves
  cl_float mWaveGenerator;

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
