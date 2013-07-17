#include "Runner.hpp"

#include <sstream>
#include <cstdio>

#if defined(MAKE_VIDEO)
#include <unistd.h>
#endif // MAKE_VIDEO

#if defined(GLFW3)
#include <GLFW/glfw3.h>
#else
#include <GL/glfw.h>
#endif // GLFW3


using std::string;
using std::ostringstream;
using std::cout;
using std::endl;


static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;


void Runner::run(const ConfigParameters &parameters,
                 Simulation &simulation,
                 CVisual &renderer) const
{
    hesp_float wave = 0.0f;
    bool shouldGenerateWaves = false;

#if defined(USE_DEBUG)
    cout << "[START] Runner" << endl;
#endif // USE_DEBUG

    const unsigned int numParticles = simulation.getNumberParticles();

    hesp_float time = 0.0f;

#if !defined(USE_CGL_SHARING)
    // Position and velocity arrays
    hesp_float4 *positions = NULL;
    hesp_float4 *velocities = NULL;
#endif // USE_CGL_SHARING

    // System sizes
    const hesp_float4 sizesMin = simulation.getSizesMin();
    const hesp_float4 sizesMax = simulation.getSizesMax();

    // Init
    simulation.init();

#if defined(USE_LINKEDCELL)
    simulation.initCells();
#endif // USE_LINKEDCELL

    renderer.initSystemVisual(sizesMin, sizesMax);

#if defined(USE_CGL_SHARING)
    renderer.initParticlesVisual(numParticles);
#else
    simulation.dumpData(positions, velocities);
    renderer.initParticlesVisual(positions, numParticles);
#endif // USE_CGL_SHARING

#if defined(MAKE_VIDEO)
    const string cmd = "ffmpeg -r 30 -f rawvideo -pix_fmt rgb24 "
                       "-s 1280x720 -an -i - -threads 2 -preset slow "
                       "-crf 18 -pix_fmt yuv420p -vf vflip -y output.mp4";

    // Frame data to write into
    const size_t nbytes = 3 * WINDOW_WIDTH * WINDOW_HEIGHT;
    char *framedata = new char[nbytes];

    FILE *ffmpeg;

    if ( !(ffmpeg = popen(cmd.c_str(), "w") ) )
    {
        perror("Error using ffmpeg!");
        exit(-1);
    }
#endif // MAKE_VIDEO

    const bool warmUp = false;
    const unsigned int warmCount = 100;

    if (warmUp)
    {
        for (unsigned int i = 0; i < warmCount; ++i)
        {
            simulation.step();
        }
    }

    do
    {
        // double start = glfwGetTime();

        simulation.step();

        // double end = glfwGetTime();
        //#if defined(USE_DEBUG)
        // printf("physics:           %f msec\n", (end - start) * 1000);
        //#endif // USE_DEBUG

        time += parameters.timeStepLength;

        if (shouldGenerateWaves)
        {
            static const hesp_float wave_push_length = (sizesMax.s[0]
                    - sizesMin.s[0]) / 3.0f;
            static const hesp_float wave_frequency = 1.0f;
            static const hesp_float wave_start = -M_PI / 2.0f;

            const hesp_float waveValue = sin(2.0f * M_PI * wave_frequency
                                             * wave + wave_start)
                                         * wave_push_length / 2.0f
                                         + wave_push_length / 2.0f;

            simulation.setWaveGenerator(waveValue);
            wave += parameters.timeStepLength;
        }

        // start = glfwGetTime();

        // Visualize particles
#if defined(USE_CGL_SHARING)
        renderer.visualizeParticles();
#else
        simulation.dumpData(positions, velocities);
        renderer.visualizeParticles(positions, velocities);
#endif // USE_CGL_SHARING
        
        renderer.checkInput(shouldGenerateWaves);

        // end = glfwGetTime();
        //#if defined(USE_DEBUG)
        // printf("graphics:          %f msec\n", (end - start) * 1000);
        //#endif // USE_DEBUG

        if ( !shouldGenerateWaves )
        {
            wave = 0.0f;
        }

#if defined(MAKE_VIDEO)
        glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, framedata);
        fwrite(framedata, 1, nbytes, ffmpeg);
#endif

#if defined(USE_DEBUG)
        cout << "Time: " << time << endl;
#endif // USE_DEBUG

    }
    while (time <= parameters.timeEnd);

#if defined(USE_DEBUG)
    cout << "[END] Runner" << endl;
#endif // USE_DEBUG

#if defined(MAKE_VIDEO)
    pclose(ffmpeg);
#endif

}
