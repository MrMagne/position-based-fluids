#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

#if defined(USE_CGL_SHARING)

#if defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#elif defined(UNIX)
#include <GL/glx.h>
#else
#include <GL/glx.h>
#endif // UNIX

#endif // USE_CGL_SHARING

#include "hesp.hpp"
#include "ocl/clsetup.hpp"
#include "io/ConfigReader.hpp"
#include "io/PartReader.hpp"
#include "visual/visual.hpp"
#include "Simulation.hpp"
#include "Runner.hpp"

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;


using std::ofstream;
using std::vector;
using std::cout;
using std::cerr;
using std::cin;
using std::endl;
using std::string;
using std::exception;
using std::runtime_error;


// Function to create system with drop
void createSystem(const string &filename)
{
    ofstream ofs( filename.c_str() );

    if ( !ofs )
    {
        throw runtime_error("Error opening output file for creating a system");
    }

    const hesp_float radius = 0.04f;
    unsigned int count = 0;

    for (hesp_float y = 0.20f; y <= 0.30f; y += 0.01f)
    {
        for (hesp_float z = 0.20f; z <= 0.40f; z += 0.01f)
        {
            for (hesp_float x = 0.10f; x <= 0.50f; x += 0.01f)
            {
                ++count;
            }
        }
    }

    for (hesp_float y = -0.05f; y <= 0.05f; y += 0.01f)
    {
        for (hesp_float z = -0.05f; z <= 0.05f; z += 0.01f)
        {
            for (hesp_float x = -0.05f; x <= 0.05f; x += 0.01f)
            {
                if (sqrt(x * x + y * y + z * z) < radius)
                {
                    ++count;
                }
            }
        }
    }

    ofs << count << endl;

    for (hesp_float y = 0.20f; y <= 0.30f; y += 0.01f)
    {
        for (hesp_float z = 0.20f; z <= 0.40f; z += 0.01f)
        {
            for (hesp_float x = 0.10f; x <= 0.50f; x += 0.01f)
            {
                ofs << 1.0f << " " << x << " " << y << " " << z
                    << " " << 0.0f << " " << 0.0f << " " << 0.0f << endl;
            }
        }
    }

    for (hesp_float y = -0.05f; y <= 0.05f; y += 0.01f)
    {
        for (hesp_float z = -0.05f; z <= 0.05f; z += 0.01f)
        {
            for (hesp_float x = -0.05f; x <= 0.05f; x += 0.01f)
            {
                if (sqrt(x * x + y * y + z * z) < radius)
                {
                    ofs << 1.0f << " " << x + 0.325f << " "
                        << y + 0.95f << " " << z + 0.325f
                        << " " << 0.0f << " " << 0.0f << " " << 0.0f << endl;
                }
            }
        }
    }
}


int main(int argc, char *argv[])
{
    try
    {
        if ( (argc != 2) )
        {
            cerr << "usage: hesp <simulation_parameters_file>" << endl;
            exit(-1);
        }

        //createSystem("drop.in");

        // Reading the configuration file
        string parameters_filename = argv[1];
        ConfigReader configReader;
        ConfigParameters parameters = configReader.read(parameters_filename);

#if defined(__WIN32)
        // Not tested
        size_t found = parameters_filename.rfind("\\");
#else
        // CAUTION: Only works on Unix based systems which
        // use single forward slashes for directories
        size_t found = parameters_filename.rfind("/");
#endif

        string base_path;

        if (found != string::npos)
        {
            base_path = parameters_filename.replace(found + 1,
                                                    parameters_filename.length()
                                                    - (found + 1), "");
        }

        // reading the part(particle) file
        string part_filename = base_path + parameters.partInputFile;
        PartReader partReader;
        vector<Particle> particles = partReader.read(part_filename);

        // For visualization
        CVisual renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
        renderer.initWindow("HESP Project");

#if defined(USE_CGL_SHARING)
        GLuint sharingBufferID = renderer.createSharingBuffer( particles.size()
                                 * sizeof(hesp_float4) );
#else
        // TODO: probably better to modify the simulation constructor to only require
        // sharing buffer id if we actually compile with USE_CGL_SHARING enabled
        GLuint sharingBufferID = 0;
#endif // USE_CGL_SHARING

        // setup kernel sources
        CSetupCL clSetup;
        vector<string> kernelSources;
        string header = clSetup.readSource("hesp.hpp");
        string source;

        source = clSetup.readSource("kernels/predict_positions.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/init_cells_old.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/update_cells.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/compute_scaling.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/compute_delta.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/update_predicted.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/update_velocities.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/apply_vorticity_and_viscosity.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/update_positions.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/calc_hash.cl");
        kernelSources.push_back(header + source);
#if !defined(USE_LINKEDCELL)
        source = clSetup.readSource("kernels/radix_histogram.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/radix_scan.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/radix_paste.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/radix_reorder.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/init_cells.cl");
        kernelSources.push_back(header + source);
        source = clSetup.readSource("kernels/find_cells.cl");
        kernelSources.push_back(header + source);
#endif

        cout << "Setting up OpenCL..." << endl;

        cl::Platform platform = clSetup.selectPlatform();
        //cl::Platform platform = clSetup.getPlatforms()[1];

#if defined(USE_CGL_SHARING)

#if defined(__APPLE__)
        CGLContextObj glContext = CGLGetCurrentContext();
        CGLShareGroupObj shareGroup = CGLGetShareGroup(glContext);

        cl_context_properties properties[] =
        {
            CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
            (cl_context_properties)shareGroup,
            0
        };
#elif defined(UNIX)
        cl_context_properties properties[] =
        {
            CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
            CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
            CL_CONTEXT_PLATFORM, (cl_context_properties) (platform)(),
            0
        };
#else
        cl_context_properties properties[] =
        {
            CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
            CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
            CL_CONTEXT_PLATFORM, (cl_context_properties) (platform)(),
            0
        };
#endif // __APPLE__

#if defined(__APPLE__)
        vector<cl::Device> devices;
        // Get a vector of devices on this platform
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
        cl::Device device = devices.at(0);
#else
        cl::Device device = clSetup.selectDevice(platform);
        //cl::Device device = clSetup.getDevices(platform)[1];
#endif
        cl::Context context = clSetup.createContext(properties);
#else
        cl::Context context = clSetup.createContext(platform);
        cl::Device device = clSetup.selectDevice(context);
#endif // USE_CGL_SHARING

        string additionalCLflags;
        additionalCLflags += "-cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math ";

        // This maybe much more elegant
#ifdef USE_DOUBLE_PRECISION
        additionalCLflags += "-DUSE_DOUBLE_PRECISION ";
#endif // USE_DOUBLE_PRECISION

#ifdef USE_DEBUG
        additionalCLflags += "-DUSE_DEBUG ";
#endif // USE_DEBUG

#ifdef USE_LINKEDCELL
        additionalCLflags += "-DUSE_LINKEDCELL ";
#endif // USE_LINKEDCELL

        cl::Program program = clSetup.createProgram(kernelSources, context,
                              device, additionalCLflags);

        map<string, cl::Kernel> kernels = clSetup.createKernelsMap(program);
        Simulation simulation(parameters, particles, kernels,
                              context, device, sharingBufferID);

        Runner runner;
        runner.run(parameters, simulation, renderer);

    } catch (const cl::Error &ecl) {
        cerr << "OpenCL Error caught: " << ecl.what() << "(" << ecl.err() << ")" << endl;
        exit(-1);
    } catch (const exception &e) {
        cerr << "STD Error caught: " << e.what() << endl;
        exit(-1);
    }

    return 0;
}
