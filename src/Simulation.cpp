#include "Simulation.hpp"

#include <cstdio>

#if defined(USE_CGL_SHARING)

#if defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#elif defined(UNIX)
#include <GL/glx.h>
#else
#include <GL/glx.h>
#endif // UNIX

#endif // USE_CGL_SHARING


using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::runtime_error;
using std::max;
using std::ceil;


#if !defined(USE_LINKEDCELL)
// RADIX SORT CONSTANTS
static const unsigned int _ITEMS = 16;
static const unsigned int _GROUPS = 16;
static const unsigned int _BITS = 6;
static const unsigned int _RADIX = 1 << _BITS;
static unsigned int _NKEYS = 0;
static const unsigned int _HISTOSPLIT = 512;
#endif


Simulation::Simulation(const ConfigParameters &parameters,
                       const vector<Particle> &particles,
                       const map<string, cl::Kernel> kernels,
                       const cl::Context &clContext,
                       const cl::Device &clDevice,
                       const GLuint sharingBufferID)
    : mCLContext(clContext),
      mCLDevice(clDevice),
      mKernels(kernels),
      mTimestepLength(parameters.timeStepLength),
      mTimeEnd(parameters.timeEnd),
      mNumParticles( particles.size() ),
      mBufferSizeParticles( particles.size() * sizeof(cl_float4) ),
      mBufferSizeCells( parameters.xN *parameters.yN *parameters.zN
                        * sizeof(cl_int) ),
      mBufferSizeParticlesList( particles.size() * sizeof(cl_int) ),
      mBufferSizeScalingFactors( particles.size() * sizeof(cl_float) ),
      mParticles(particles),
      mPositions(NULL),
      mVelocities(NULL),
      mCells(NULL),
      mParticlesList(NULL),
      mRestDens(parameters.restDensity),
      mWaveGenerator(0.0f),
      mSharingBufferID(sharingBufferID)
{

#if defined(USE_DEBUG)
    cout << "[START] Simulation::Simulation" << endl;
#endif // USE_DEBUG

    // Assign vector type parameters
    // (cannot be done in initializer list this way)
    mSystemSizeMin.s[0] = parameters.xMin;
    mSystemSizeMin.s[1] = parameters.yMin;
    mSystemSizeMin.s[2] = parameters.zMin;
    mSystemSizeMin.s[3] = 0.0f;

    mSystemSizeMax.s[0] = parameters.xMax;
    mSystemSizeMax.s[1] = parameters.yMax;
    mSystemSizeMax.s[2] = parameters.zMax;
    mSystemSizeMax.s[3] = 0.0f;

    mNumberCells.s[0] = parameters.xN;
    mNumberCells.s[1] = parameters.yN;
    mNumberCells.s[2] = parameters.zN;

    mCellLength.s[0] = (parameters.xMax - parameters.xMin) / parameters.xN;
    mCellLength.s[1] = (parameters.yMax - parameters.yMin) / parameters.yN;
    mCellLength.s[2] = (parameters.zMax - parameters.zMin) / parameters.zN;
    mCellLength.s[3] = 0.0f;

#if defined(USE_DEBUG)
    cout << "[END] Simulation::Simulation" << endl;
#endif // USE_DEBUG

}

Simulation::~Simulation ()
{
    glFinish();
    mQueue.finish();

    // Delete buffers
    delete[] mCells;
    delete[] mPositions;
    delete[] mVelocities;
    delete[] mScalingFactors;
    delete[] mDeltaVelocity;
    delete[] mParticlesList;
    delete[] mVorticityForces;
#if !defined(USE_LINKEDCELL)
    delete[] mRadixCells;
#endif // USE_LINKEDCELL
}


void
Simulation::init(void)
{

#if defined(USE_DEBUG)
    cout << "[START] Simulation::init" << endl;
    cout << "Number of particles: " << mNumParticles << endl;
#endif // USE_DEBUG

    // setup buffers
    mPositions = new cl_float4[mNumParticles];
    mPredicted = new cl_float4[mNumParticles];
    mVelocities = new cl_float4[mNumParticles];
    mScalingFactors = new cl_float[mNumParticles];
    mDelta = new cl_float4[mNumParticles];
    mDeltaVelocity = new cl_float4[mNumParticles];
    mVorticityForces = new cl_float4[mNumParticles];
#if !defined(USE_LINKEDCELL)
    mRadixCells = new cl_uint2[_NKEYS];
#endif // USE_LINKEDCELL

    // Initialize particle arrays
    for (cl_uint i = 0; i < mNumParticles; ++i)
    {
        Particle p = mParticles[i];

        mPositions[i].s[0] = p.x[0];
        mPositions[i].s[1] = p.x[1];
        mPositions[i].s[2] = p.x[2];
        // to save space we use the 4th component for the mass and timestep term
        mPositions[i].s[3] = 0.0f;

        mPredicted[i].s[0] = 0.0f;
        mPredicted[i].s[1] = 0.0f;
        mPredicted[i].s[2] = 0.0f;
        mPredicted[i].s[3] = 0.0f;

        mVelocities[i].s[0] = p.v[0];
        mVelocities[i].s[1] = p.v[1];
        mVelocities[i].s[2] = p.v[2];
        // to save space we use the 4th component for the timestep
        mVelocities[i].s[3] = p.m;

        mScalingFactors[i] = 0.0f;

        mDelta[i].s[0] = 0.0f;
        mDelta[i].s[1] = 0.0f;
        mDelta[i].s[2] = 0.0f;
        mDelta[i].s[3] = 0.0f;

        mDeltaVelocity[i].s[0] = 0.0f;
        mDeltaVelocity[i].s[1] = 0.0f;
        mDeltaVelocity[i].s[2] = 0.0f;
        mDeltaVelocity[i].s[3] = 0.0f;

        mVorticityForces[i].s[0] = 0.0f;
        mVorticityForces[i].s[1] = 0.0f;
        mVorticityForces[i].s[2] = 0.0f;
        mVorticityForces[i].s[3] = 0.0f;
    }

    mQueue = cl::CommandQueue(mCLContext, mCLDevice);

    const cl_uint globalSize = ceil(mNumParticles / 32.0f) * 32;

    mGlobalRange = cl::NDRange(globalSize);
    mLocalRange = cl::NullRange;

#if defined(USE_CGL_SHARING)
    // context has to be setup to allow gl/cl sharing
    // TODO: buffer could be changed to be CL_MEM_WRITE_ONLY
    // but for debugging also reading it might be helpful
#if defined(USE_DEBUG)
    printf("simulation::init: sharingID: %d\n", mSharingBufferID);
#endif // USE_DEBUG

    mPositionsBuffer = cl::BufferGL(mCLContext, CL_MEM_READ_WRITE,
                                    mSharingBufferID);

    vector<cl::Memory> sharedBuffers;

    sharedBuffers.push_back(mPositionsBuffer);
    mQueue.enqueueAcquireGLObjects(&sharedBuffers);
    mQueue.enqueueWriteBuffer(mPositionsBuffer, CL_TRUE,
                              0, mBufferSizeParticles, mPositions);
    mQueue.enqueueReleaseGLObjects(&sharedBuffers);
    mQueue.finish();
#else
    mPositionsBuffer = cl::Buffer(mCLContext,
                                  CL_MEM_READ_WRITE, mBufferSizeParticles);
    mQueue.enqueueWriteBuffer(mPositionsBuffer, CL_TRUE,
                              0, mBufferSizeParticles, mPositions);
#endif // USE_CGL_SHARING

    mPredictedBuffer = cl::Buffer(mCLContext,
                                  CL_MEM_READ_WRITE, mBufferSizeParticles);
    mQueue.enqueueWriteBuffer(mPredictedBuffer, CL_TRUE,
                              0, mBufferSizeParticles, mPredicted);

    mVelocitiesBuffer = cl::Buffer(mCLContext,
                                   CL_MEM_READ_WRITE, mBufferSizeParticles);
    mQueue.enqueueWriteBuffer(mVelocitiesBuffer, CL_TRUE,
                              0, mBufferSizeParticles, mVelocities);

    mDeltaBuffer = cl::Buffer(mCLContext,
                              CL_MEM_READ_WRITE, mBufferSizeParticles);
    mQueue.enqueueWriteBuffer(mDeltaBuffer, CL_TRUE,
                              0, mBufferSizeParticles, mDelta);

    mDeltaVelocityBuffer = cl::Buffer(mCLContext,
                                      CL_MEM_READ_WRITE, mBufferSizeParticles);
    mQueue.enqueueWriteBuffer(mDeltaVelocityBuffer, CL_TRUE,
                              0, mBufferSizeParticles, mDeltaVelocity);

    mScalingFactorsBuffer = cl::Buffer(mCLContext, CL_MEM_READ_WRITE,
                                       mBufferSizeScalingFactors);
    mQueue.enqueueWriteBuffer(mScalingFactorsBuffer, CL_TRUE,
                              0, mBufferSizeScalingFactors, mScalingFactors);

    mVorticityBuffer = cl::Buffer(mCLContext, CL_MEM_READ_WRITE,
                                  mBufferSizeParticles);
    mQueue.enqueueWriteBuffer(mVorticityBuffer, CL_TRUE,
                              0, mBufferSizeParticles, mVorticityForces);

#if !defined(USE_LINKEDCELL)
    // get closest multiple to of items/groups
    if (mNumParticles % (_ITEMS * _GROUPS) == 0)
    {
        _NKEYS = mNumParticles;
    }
    else
    {
        _NKEYS = mNumParticles + (_ITEMS * _GROUPS)
                 - mNumParticles % (_ITEMS * _GROUPS);
    }

    mRadixCellsBuffer = cl::Buffer(mCLContext, CL_MEM_READ_WRITE,
                                   sizeof(cl_uint2) * _NKEYS);
    mRadixCellsOutBuffer = cl::Buffer(mCLContext, CL_MEM_READ_WRITE,
                                      sizeof(cl_uint2) * _NKEYS);
    mFoundCellsBuffer = cl::Buffer(mCLContext, CL_MEM_READ_WRITE,
                                   sizeof(cl_int2) * mNumberCells.s[0]
                                   * mNumberCells.s[1] * mNumberCells.s[2]);

    mRadixHistogramBuffer = cl::Buffer(mCLContext, CL_MEM_READ_WRITE,
                                       sizeof(cl_uint) * _RADIX * _ITEMS * _GROUPS);
    mRadixGlobSumBuffer = cl::Buffer(mCLContext, CL_MEM_READ_WRITE,
                                     sizeof(cl_uint) * _HISTOSPLIT);
#endif // USE_LINKEDCELL

    mQueue.finish();
}

void
Simulation::initCells(void)
{
    const cl_uint cellCount = mNumberCells.s[0]
                              * mNumberCells.s[1] * mNumberCells.s[2];
    mCells = new cl_int[cellCount];
    mParticlesList = new cl_int[mNumParticles];

    // Init cells
    for (cl_uint i = 0; i < cellCount; ++i)
    {
        mCells[i] = END_OF_CELL_LIST;
    }

    // Init particles
    for (cl_uint i = 0; i < mNumParticles; ++i)
    {
        mParticlesList[i] = END_OF_CELL_LIST;
    }

    // Write buffer for cells
    mCellsBuffer = cl::Buffer(mCLContext, CL_MEM_READ_WRITE, mBufferSizeCells);
    mQueue.enqueueWriteBuffer(mCellsBuffer, CL_TRUE,
                              0, mBufferSizeCells, mCells);

    // Write buffer for particles
    mParticlesListBuffer = cl::Buffer(mCLContext,
                                      CL_MEM_READ_WRITE, mBufferSizeParticlesList);
    mQueue.enqueueWriteBuffer(mParticlesListBuffer, CL_TRUE,
                              0, mBufferSizeParticlesList, mParticlesList);
}

void
Simulation::updatePositions(void)
{
    mKernels["updatePositions"].setArg(0, mPositionsBuffer);
    mKernels["updatePositions"].setArg(1, mPredictedBuffer);
    mKernels["updatePositions"].setArg(2, mVelocitiesBuffer);
    mKernels["updatePositions"].setArg(3, mNumParticles);

    mQueue.enqueueNDRangeKernel(mKernels["updatePositions"], 0,
                                mGlobalRange, mLocalRange);
}

void
Simulation::updateVelocities(void)
{
    mKernels["updateVelocities"].setArg(0, mPositionsBuffer);
    mKernels["updateVelocities"].setArg(1, mPredictedBuffer);
    mKernels["updateVelocities"].setArg(2, mVelocitiesBuffer);
    mKernels["updateVelocities"].setArg(3, mDeltaVelocityBuffer);
    mKernels["updateVelocities"].setArg(4, mVorticityBuffer);
    mKernels["updateVelocities"].setArg(5, mTimestepLength);
    mKernels["updateVelocities"].setArg(6, mNumParticles);

    mQueue.enqueueNDRangeKernel(mKernels["updateVelocities"], 0,
                                mGlobalRange, mLocalRange);
}

void
Simulation::applyVorticityAndViscosity(void)
{
    mKernels["applyVorticityAndViscosity"].setArg(0, mPredictedBuffer);
    mKernels["applyVorticityAndViscosity"].setArg(1, mVelocitiesBuffer);
    mKernels["applyVorticityAndViscosity"].setArg(2, mDeltaVelocityBuffer);
    mKernels["applyVorticityAndViscosity"].setArg(3, mVorticityBuffer);
#if defined(USE_LINKEDCELL)
    mKernels["applyVorticityAndViscosity"].setArg(4, mCellsBuffer);
    mKernels["applyVorticityAndViscosity"].setArg(5, mParticlesListBuffer);
#else
    mKernels["applyVorticityAndViscosity"].setArg(4, mRadixCellsBuffer);
    mKernels["applyVorticityAndViscosity"].setArg(5, mFoundCellsBuffer);
#endif // USE_LINKEDCELL
    mKernels["applyVorticityAndViscosity"].setArg(6, mSystemSizeMin);
    mKernels["applyVorticityAndViscosity"].setArg(7, mSystemSizeMax);
    mKernels["applyVorticityAndViscosity"].setArg(8, mCellLength);
    mKernels["applyVorticityAndViscosity"].setArg(9, mNumberCells);
    mKernels["applyVorticityAndViscosity"].setArg(10, mNumParticles);

    mQueue.enqueueNDRangeKernel(mKernels["applyVorticityAndViscosity"],
                                0, mGlobalRange, mLocalRange);
}

void
Simulation::predictPositions(void)
{
    mKernels["predictPositions"].setArg(0, mPositionsBuffer);
    mKernels["predictPositions"].setArg(1, mPredictedBuffer);
    mKernels["predictPositions"].setArg(2, mVelocitiesBuffer);
    mKernels["predictPositions"].setArg(3, mSystemSizeMin);
    mKernels["predictPositions"].setArg(4, mSystemSizeMax);
    mKernels["predictPositions"].setArg(5, mTimestepLength);
    mKernels["predictPositions"].setArg(6, mNumParticles);

    mQueue.enqueueNDRangeKernel(mKernels["predictPositions"], 0,
                                mGlobalRange, mLocalRange);
}

void
Simulation::updatePredicted(void)
{
    mKernels["updatePredicted"].setArg(0, mPredictedBuffer);
    mKernels["updatePredicted"].setArg(1, mDeltaBuffer);
    mKernels["updatePredicted"].setArg(2, mNumParticles);

    mQueue.enqueueNDRangeKernel(mKernels["updatePredicted"], 0,
                                mGlobalRange, mLocalRange);
}

void
Simulation::computeDelta(void)
{
    mKernels["computeDelta"].setArg(0, mDeltaBuffer);
    mKernels["computeDelta"].setArg(1, mPredictedBuffer);
    mKernels["computeDelta"].setArg(2, mScalingFactorsBuffer);
#if defined(USE_LINKEDCELL)
    mKernels["computeDelta"].setArg(3, mCellsBuffer);
    mKernels["computeDelta"].setArg(4, mParticlesListBuffer);
#else
    mKernels["computeDelta"].setArg(3, mRadixCellsBuffer);
    mKernels["computeDelta"].setArg(4, mFoundCellsBuffer);
#endif
    mKernels["computeDelta"].setArg(5, mTimestepLength);
    mKernels["computeDelta"].setArg(6, mSystemSizeMin);
    mKernels["computeDelta"].setArg(7, mSystemSizeMax);
    mKernels["computeDelta"].setArg(8, mCellLength);
    mKernels["computeDelta"].setArg(9, mNumberCells);
    mKernels["computeDelta"].setArg(10, mWaveGenerator);
    mKernels["computeDelta"].setArg(11, mRestDens);
    mKernels["computeDelta"].setArg(12, mNumParticles);

    mQueue.enqueueNDRangeKernel(mKernels["computeDelta"], 0,
                                mGlobalRange, mLocalRange);
}

void
Simulation::computeScaling(void)
{
    mKernels["computeScaling"].setArg(0, mPredictedBuffer);
    mKernels["computeScaling"].setArg(1, mScalingFactorsBuffer);
#if defined(USE_LINKEDCELL)
    mKernels["computeScaling"].setArg(2, mCellsBuffer);
    mKernels["computeScaling"].setArg(3, mParticlesListBuffer);
#else
    mKernels["computeScaling"].setArg(2, mRadixCellsBuffer);
    mKernels["computeScaling"].setArg(3, mFoundCellsBuffer);
#endif
    mKernels["computeScaling"].setArg(4, mSystemSizeMin);
    mKernels["computeScaling"].setArg(5, mCellLength);
    mKernels["computeScaling"].setArg(6, mNumberCells);
    mKernels["computeScaling"].setArg(7, mRestDens);
    mKernels["computeScaling"].setArg(8, mNumParticles);

    mQueue.enqueueNDRangeKernel(mKernels["computeScaling"], 0,
                                mGlobalRange, mLocalRange);
}

void
Simulation::updateCells(void)
{
    mKernels["initCellsOld"].setArg(0, mCellsBuffer);
    mKernels["initCellsOld"].setArg(1, mParticlesListBuffer);
    mKernels["initCellsOld"].setArg(2, mNumberCells.s[0]
                                    * mNumberCells.s[1] * mNumberCells.s[2]);
    mKernels["initCellsOld"].setArg(3, mNumParticles);

    mQueue.enqueueNDRangeKernel(mKernels["initCellsOld"], 0,
                                cl::NDRange( max(mBufferSizeParticlesList,
                                        mBufferSizeCells) ), mLocalRange);

    mKernels["updateCells"].setArg(0, mPredictedBuffer);
    mKernels["updateCells"].setArg(1, mCellsBuffer);
    mKernels["updateCells"].setArg(2, mParticlesListBuffer);
    mKernels["updateCells"].setArg(3, mCellLength);
    mKernels["updateCells"].setArg(4, mNumberCells);
    mKernels["updateCells"].setArg(5, mSystemSizeMin);
    mKernels["updateCells"].setArg(6, mNumParticles);

    mQueue.enqueueNDRangeKernel(mKernels["updateCells"], 0,
                                mGlobalRange, mLocalRange);
}

#if !defined(USE_LINKEDCELL)
void
Simulation::radix(void)
{
    const static unsigned int _CELLSTOTAL = mNumberCells.s[0]
                                            * mNumberCells.s[1]
                                            * mNumberCells.s[2];
    // round up to the next power of 2
    const static unsigned int _TOTALBITS = ceilf(ceilf( log2f(_CELLSTOTAL) )
                                           / (float) _BITS) * _BITS;
    const static unsigned int _MAXINT = 1 << (_TOTALBITS - 1);
    const static unsigned int _PASS = _TOTALBITS / _BITS;
    static const int _MAXMEMCACHE = std::max(_HISTOSPLIT, _ITEMS * _GROUPS
                                    * _RADIX / _HISTOSPLIT);

    cl::Kernel calcHashKernel = mKernels["calcHash"];

    mKernels["calcHash"].setArg(0, mPredictedBuffer);
    mKernels["calcHash"].setArg(1, mRadixCellsBuffer);
    mKernels["calcHash"].setArg(2, mCellLength);
    mKernels["calcHash"].setArg(3, mNumberCells);
    mKernels["calcHash"].setArg(4, mSystemSizeMin);
    mKernels["calcHash"].setArg(5, _MAXINT);
    mKernels["calcHash"].setArg(6, mNumParticles);
    mKernels["calcHash"].setArg(7, _NKEYS);

    mQueue.enqueueNDRangeKernel(mKernels["calcHash"], cl::NullRange,
                                cl::NDRange(_NKEYS), cl::NullRange);

    cl::Kernel reorderKernel = mKernels["reorder"];

    for (unsigned int pass = 0; pass < _PASS; pass++ )
    {
        //histogram
        mKernels["histogram"].setArg(0, mRadixCellsBuffer);
        mKernels["histogram"].setArg(1, mRadixHistogramBuffer);
        mKernels["histogram"].setArg(2, pass);
        mKernels["histogram"].setArg(3, sizeof(cl_uint) * _RADIX * _ITEMS, NULL);
        mKernels["histogram"].setArg(4, _NKEYS);
        mKernels["histogram"].setArg(5, _RADIX);
        mKernels["histogram"].setArg(6, _BITS);

        mQueue.enqueueNDRangeKernel(mKernels["histogram"], cl::NullRange,
                                    cl::NDRange(_ITEMS * _GROUPS),
                                    cl::NDRange(_ITEMS));

        //scan
        mKernels["scan"].setArg(0, mRadixHistogramBuffer);
        mKernels["scan"].setArg(1, sizeof(cl_uint) * _MAXMEMCACHE, NULL);
        mKernels["scan"].setArg(2, mRadixGlobSumBuffer);

        mQueue.enqueueNDRangeKernel(mKernels["scan"], cl::NullRange,
                                    cl::NDRange(_RADIX * _GROUPS * _ITEMS / 2),
                                    cl::NDRange((_RADIX * _GROUPS * _ITEMS / 2)
                                                / _HISTOSPLIT));

        mKernels["scan"].setArg(0, mRadixGlobSumBuffer);
        mKernels["scan"].setArg(2, mRadixHistogramBuffer);

        mQueue.enqueueNDRangeKernel(mKernels["scan"], cl::NullRange,
                                    cl::NDRange(_HISTOSPLIT / 2),
                                    cl::NDRange(_HISTOSPLIT / 2));

        mKernels["paste"].setArg(0, mRadixHistogramBuffer);
        mKernels["paste"].setArg(1, mRadixGlobSumBuffer);

        mQueue.enqueueNDRangeKernel(mKernels["paste"], cl::NullRange,
                                    cl::NDRange(_RADIX * _GROUPS * _ITEMS / 2),
                                    cl::NDRange((_RADIX * _GROUPS * _ITEMS / 2)
                                                / _HISTOSPLIT));

        //reorder
        mKernels["reorder"].setArg(0, mRadixCellsBuffer);
        mKernels["reorder"].setArg(1, mRadixCellsOutBuffer);
        mKernels["reorder"].setArg(2, mRadixHistogramBuffer);
        mKernels["reorder"].setArg(3, pass);
        mKernels["reorder"].setArg(4, sizeof(cl_uint) * _RADIX * _ITEMS, NULL);
        mKernels["reorder"].setArg(5, _NKEYS);
        mKernels["reorder"].setArg(6, _RADIX);
        mKernels["reorder"].setArg(7, _BITS);

        mQueue.enqueueNDRangeKernel(mKernels["reorder"], cl::NullRange,
                                    cl::NDRange(_ITEMS * _GROUPS),
                                    cl::NDRange(_ITEMS));

        cl::Buffer tmp = mRadixCellsBuffer;

        mRadixCellsBuffer = mRadixCellsOutBuffer;
        mRadixCellsOutBuffer = tmp;
    }

    mKernels["initCells"].setArg(0, mFoundCellsBuffer);
    mKernels["initCells"].setArg(1, mNumberCells.s[0]
                                 * mNumberCells.s[1] * mNumberCells.s[2]);

    mQueue.enqueueNDRangeKernel(mKernels["initCells"], cl::NullRange,
                                cl::NDRange(mNumberCells.s[0]
                                            * mNumberCells.s[1] * mNumberCells.s[2]),
                                cl::NullRange);

    mKernels["findCells"].setArg(0, mRadixCellsBuffer);
    mKernels["findCells"].setArg(1, mFoundCellsBuffer);
    mKernels["findCells"].setArg(2, mNumParticles);

    mQueue.enqueueNDRangeKernel(mKernels["findCells"], cl::NullRange,
                                cl::NDRange(mNumParticles), cl::NullRange);
}
#endif

void
Simulation::step(void)
{

#if defined(USE_DEBUG)
    double start = 0.0f, end = 0.0f;
    double istart = 0.0f, iend = 0.0f;
#endif // USE_DEBUG

#if defined(USE_CGL_SHARING)
    // start = glfwGetTime();
    glFinish();
    vector<cl::Memory> sharedBuffers;
    sharedBuffers.push_back(mPositionsBuffer);
    mQueue.enqueueAcquireGLObjects(&sharedBuffers);
    // mQueue.finish();
    // end = glfwGetTime();
    // printf("acquiring gl:       %f msec\n", (end - start) * 1000);
#endif

    // start = glfwGetTime();
    this->predictPositions();
    // mQueue.finish();
    // end = glfwGetTime();
    // printf("predictPositons:    %f msec\n", (end - start) * 1000);

#if defined(USE_DEBUG)
    cout << "predictPositions \n" << endl;
#endif // USE_DEBUG

#if defined(USE_LINKEDCELL)
    // start = glfwGetTime();
    this->updateCells();
    // mQueue.finish();
    // end = glfwGetTime();
    // printf("updateCells:        %f msec\n", (end - start) * 1000);
#else
    // start = glfwGetTime();
    this->radix();
    // mQueue.finish();
    // end = glfwGetTime();
    // printf("r:%f\n", (end - start) * 1000);
#endif


#if defined(USE_DEBUG)
    cout << "updateCells \n" << endl;
#endif // USE_DEBUG

    // istart = glfwGetTime();

    const unsigned int solver_iterations = 4;

    for (unsigned int i = 0; i < solver_iterations; ++i)
    {
        // start = glfwGetTime();
        this->computeScaling();
        // mQueue.finish();
        // end = glfwGetTime();
        // printf("computeScaling:     %f msec\n", (end - start) * 1000);

#if defined(USE_DEBUG)
        cout << "computeScaling \n" << endl;
#endif // USE_DEBUG

        // start = glfwGetTime();
        this->computeDelta();
        // mQueue.finish();
        // end = glfwGetTime();
        // printf("computeDelta:       %f msec\n", (end - start) * 1000);

#if defined(USE_DEBUG)
        cout << "computeDelta \n" << endl;
#endif // USE_DEBUG

        // start = glfwGetTime();
        this->updatePredicted();
        // mQueue.finish();
        // end = glfwGetTime();
        // printf("updatePredicted:    %f msec\n", (end - start) * 1000);

#if defined(USE_DEBUG)
        cout << "updatePredicted \n" << endl;
#endif // USE_DEBUG

    }
    // mQueue.finish();
    // iend = glfwGetTime();
    // printf("iterations:         %f msec\n", (iend - istart) * 1000);

    // start = glfwGetTime();
    this->updateVelocities();
    // mQueue.finish();
    // end = glfwGetTime();
    // printf("updateVelocities:   %f msec\n", (end - start) * 1000);

#if defined(USE_DEBUG)
    cout << "updateVelocities \n" << endl;
#endif // USE_DEBUG

    // start = glfwGetTime();
    this->applyVorticityAndViscosity();
    // mQueue.finish();
    // end = glfwGetTime();
    // printf("applyViscosity:     %f msec\n", (end - start) * 1000);

#if defined(USE_DEBUG)
    cout << "applyVorticityAndViscosity \n" << endl;
#endif // USE_DEBUG

    // start = glfwGetTime();
    this->updatePositions();
    // mQueue.finish();
    // end = glfwGetTime();
    // printf("updatePositions:    %f msec\n", (end - start) * 1000);

#if defined(USE_DEBUG)
    cout << "updatePositions \n" << endl;
#endif // USE_DEBUG

#if defined(USE_CGL_SHARING)
    // start = glfwGetTime();
    mQueue.enqueueReleaseGLObjects(&sharedBuffers);
    mQueue.finish(); // clFinish()
    // end = glfwGetTime();
    // printf("releasing gl:       %f msec\n", (end - start) * 1000);
#endif

}

void
Simulation::dumpData( cl_float4 * (&positions), cl_float4 * (&velocities) )
{
    mQueue.enqueueReadBuffer(mPositionsBuffer, CL_TRUE,
                             0, mBufferSizeParticles, mPositions);
    mQueue.enqueueReadBuffer(mVelocitiesBuffer, CL_TRUE,
                             0, mBufferSizeParticles, mVelocities);

    // just a safety measure to be absolutely sure everything is transferred
    // from device to host
    mQueue.finish();

    positions = mPositions;
    velocities = mVelocities;
}
