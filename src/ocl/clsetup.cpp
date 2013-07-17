#include "clsetup.hpp"


using std::string;
using std::vector;
using std::ifstream;
using std::runtime_error;
using std::make_pair;
using std::cout;
using std::cerr;
using std::cin;
using std::istreambuf_iterator;
using std::ostream;
using std::endl;


ostream &operator<<(ostream &os, const cl::Platform &platform)
{
    os << "CL_PLATFORM_VERSION    = "
       << platform.getInfo<CL_PLATFORM_VERSION>() << endl;
    os << "CL_PLATFORM_NAME       = "
       << platform.getInfo<CL_PLATFORM_NAME>() << endl;
    os << "CL_PLATFORM_VENDOR     = "
       << platform.getInfo<CL_PLATFORM_VENDOR>() << endl;
    os << "CL_PLATFORM_EXTENSIONS = "
       << platform.getInfo<CL_PLATFORM_EXTENSIONS>() << endl;

    return os;
}

ostream &operator<<(ostream &os, const cl::Device &device)
{
    os << "CL_DEVICE_EXTENSIONS                    = "
       << device.getInfo<CL_DEVICE_EXTENSIONS>() << endl;
    os << "CL_DEVICE_GLOBAL_MEM_SIZE               = "
       << device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() << endl;
    os << "CL_DEVICE_LOCAL_MEM_SIZE                = "
       << device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() << endl;
    os << "CL_DEVICE_MAX_CLOCK_FREQUENCY           = "
       << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << endl;
    os << "CL_DEVICE_MAX_COMPUTE_UNITS             = "
       << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << endl;
    os << "CL_DEVICE_MAX_CONSTANT_ARGS             = "
       << device.getInfo<CL_DEVICE_MAX_CONSTANT_ARGS>() << endl;
    os << "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE      = "
       << device.getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>() << endl;
    os << "CL_DEVICE_MAX_MEM_ALLOC_SIZE            = "
       << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() << endl;
    os << "CL_DEVICE_MAX_PARAMETER_SIZE            = "
       << device.getInfo<CL_DEVICE_MAX_PARAMETER_SIZE>() << endl;
    os << "CL_DEVICE_MAX_WORK_GROUP_SIZE           = "
       << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << endl;
    os << "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS      = "
       << device.getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>() << endl;

    vector<size_t> vecSizes = device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();

    os << "CL_DEVICE_MAX_WORK_ITEM_SIZES           = ";
    os << "[";

    for (unsigned int i = 0; i < vecSizes.size(); i++)
    {
        os << vecSizes.at(i);

        if (i < vecSizes.size() - 1)
        {
            os << ", ";
        }
    }

    os << "]" << endl;

    os << "CL_DEVICE_NAME                          = "
       << device.getInfo<CL_DEVICE_NAME>() << endl;
    os << "CL_DEVICE_VENDOR                        = "
       << device.getInfo<CL_DEVICE_VENDOR>() << endl;
    os << "CL_DEVICE_VERSION                       = "
       << device.getInfo<CL_DEVICE_VERSION>() << endl;
    os << "CL_DRIVER_VERSION                       = "
       << device.getInfo<CL_DRIVER_VERSION>() << endl;
    os << "CL_DEVICE_PROFILE                       = "
       << device.getInfo<CL_DEVICE_PROFILE>() << endl;

    return os;
}


vector<cl::Platform>
CSetupCL::getPlatforms(void) const
{
    vector<cl::Platform> platforms;
    unsigned int platformNumber = 0;

    // Get platforms available
    cl::Platform::get(&platforms);

    // Print info about found platforms
    cout << "Available platforms: " << endl;

    for (vector<cl::Platform>::const_iterator cit = platforms.begin();
            cit != platforms.end(); ++cit)
    {
        cout << "Platform number #" << platformNumber << ":" << endl;
        this->printPlatformInfo(*cit);

        ++platformNumber;
    }

    if (platforms.size() == 0)
    {
        throw runtime_error("No platforms found.");
    }

    return platforms;
}

cl::Platform
CSetupCL::selectPlatform(void) const
{
    vector<cl::Platform> platforms;
    size_t platformID = 0;

    platforms = this->getPlatforms();

    // Select platform
    if (platforms.size() > 1)
    {
        platformID = platforms.size();

        while ( platformID >= platforms.size() )
        {
            cout << "Select a valid platform: " << endl;
            cin >> platformID;
        }
    }

    return platforms.at(platformID);
}

cl::Context
CSetupCL::createContext(cl_context_properties properties[],
                        const cl_device_type deviceType) const
{
    return cl::Context(deviceType, properties);
}

cl::Context
CSetupCL::createContext(const cl::Platform &platform,
                        const cl_device_type deviceType) const
{
    // Create context
    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties) (platform)(),
        0
    };

    return cl::Context(deviceType, properties);
}

vector<cl::Device>
CSetupCL::getDevices(const cl::Context &context) const
{
    unsigned int numberDevices = 0;
    vector<cl::Device> devices;

    // Get a vector of devices on this platform
    devices = context.getInfo<CL_CONTEXT_DEVICES>();

    if (devices.size() == 0)
    {
        throw runtime_error("No devices found!");
    }

    // Print info about found devices
    cout << "Found devices: " << endl;

    for (vector<cl::Device>::const_iterator cit = devices.begin();
            cit != devices.end(); ++cit)
    {
        cout << "Device number #" << numberDevices << " :" << endl;
        this->printDeviceInfo(*cit);

        ++numberDevices;
    }

    return devices;
}

vector<cl::Device>
CSetupCL::getDevices(const cl::Platform &platform,
                     const cl_device_type deviceType) const
{
    unsigned int numberDevices = 0;
    vector<cl::Device> devices;

    // Get a vector of devices on this platform
    platform.getDevices(deviceType, &devices);

    if (devices.size() == 0)
    {
        throw runtime_error("No devices found!");
    }

    // Print info about found devices
    cout << "Found devices: " << endl;

    for (vector<cl::Device>::const_iterator cit = devices.begin();
            cit != devices.end(); ++cit)
    {
        cout << "Device number #" << numberDevices << " :" << endl;
        this->printDeviceInfo(*cit);

        ++numberDevices;
    }

    return devices;
}

cl::Device
CSetupCL::selectDevice(const cl::Context &context) const
{
    size_t deviceID = 0;
    vector<cl::Device> devices;

    devices = this->getDevices(context);

    if (devices.size() > 1)
    {
        deviceID = devices.size();

        while ( deviceID >= devices.size() )
        {
            cout << "Select valid device to use: " << endl;
            cin >> deviceID;
        }
    }
    else if (devices.size() == 0)
    {
        throw runtime_error("No devices found!");
    }

    return devices.at(deviceID);
}

cl::Device
CSetupCL::selectDevice(const cl::Platform &platform,
                       const cl_device_type deviceType) const
{
    size_t deviceID = 0;
    vector<cl::Device> devices;

    devices = this->getDevices(platform, deviceType);

    if (devices.size() > 1)
    {
        deviceID = devices.size();

        while ( deviceID >= devices.size() )
        {
            cout << "Select valid device to use: " << endl;
            cin >> deviceID;
        }
    }
    else if (devices.size() == 0)
    {
        throw runtime_error("No devices found!");
    }

    return devices.at(deviceID);
}

string
CSetupCL::readSource(const string &filename) const
{
    ifstream ifs( filename.c_str() );
    string source;

    if ( !ifs.is_open() )
    {
        throw runtime_error("Could not open File!");
    }

    source = string( istreambuf_iterator<char>(ifs),
                     istreambuf_iterator<char>() );

    return source;
}

cl::Program
CSetupCL::createProgram(const vector<string> &sources,
                        const cl::Context &context,
                        const vector<cl::Device> &devices,
                        const string compileOptions) const
{
    cl::Program program;
    cl::Program::Sources programSource;

    for (vector<string>::const_iterator cit = sources.begin();
            cit != sources.end(); ++cit)
    {
        programSource.push_back( std::make_pair( cit->c_str(),
                                 cit->length() ) );
    }

    // Make program of the source code in the context
    program = cl::Program(context, programSource);

    try {
        // Build program for these devices
        program.build( devices, compileOptions.c_str() );

    } catch (const cl::Error &e) {
        cerr << e.what() << " (" << e.err() << ")" << endl
             << "Buildlog: " << this->getBuildLog(program, devices) << endl;
        exit(-1);
    }

    return program;
}

cl::Program
CSetupCL::createProgram(const vector<string> &sources,
                        const cl::Context &context,
                        const cl::Device &device,
                        const string compileOptions) const
{
    vector<cl::Device> devices;

    devices.push_back(device);

    return this->createProgram(sources, context, devices, compileOptions);
}

cl::Program
CSetupCL::createProgram(const string &source,
                        const cl::Context &context,
                        const vector<cl::Device> &devices,
                        const string compileOptions) const
{
    vector<string> sources;

    sources.push_back(source);

    return this->createProgram(sources, context, devices, compileOptions);
}

cl::Program
CSetupCL::createProgram(const string &source,
                        const cl::Context &context,
                        const cl::Device &device,
                        const string compileOptions) const
{
    vector<cl::Device> devices;
    vector<string> sources;

    devices.push_back(device);
    sources.push_back(source);

    return this->createProgram(sources, context, devices, compileOptions);
}

cl::Kernel
CSetupCL::createKernel(const string &programName,
                       const cl::Program &program) const
{
    return cl::Kernel( program, programName.c_str() );
}

vector<cl::Kernel>
CSetupCL::createKernels(cl::Program &program) const
{
    vector<cl::Kernel> kernels;

    program.createKernels(&kernels);

    return kernels;
}

map<string, cl::Kernel>
CSetupCL::createKernelsMap(cl::Program &program) const
{
    map<string, cl::Kernel> ret;
    vector<cl::Kernel> kernels;

    program.createKernels(&kernels);

    for (vector<cl::Kernel>::const_iterator cit = kernels.begin();
            cit != kernels.end(); ++cit)
    {
        string kernelName = cit->getInfo<CL_KERNEL_FUNCTION_NAME>();

        ret[kernelName] = *cit;
    }

    return ret;
}

string
CSetupCL::getBuildLog(const cl::Program &program,
                      const cl::Device &device) const
{
    string ret;

    program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &ret);

    return ret;
}

string
CSetupCL::getBuildLog(const cl::Program &program,
                      const vector<cl::Device> &devices) const
{
    string ret;
    string tmp;

    for (vector<cl::Device>::const_iterator cit = devices.begin();
            cit != devices.end(); ++cit)
    {
        ret += this->getBuildLog(program, *cit);
    }

    return ret;
}

void
CSetupCL::printPlatformInfo(const cl::Platform &platform) const
{
    cout << platform << endl;
}

void
CSetupCL::printDeviceInfo(const cl::Device &device) const
{
    cout << device << endl;
}
