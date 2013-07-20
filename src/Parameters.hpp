#ifndef __PARAMETERS_HPP
#define __PARAMETERS_HPP

#include <string>

#include "hesp.hpp"


using std::string;


struct ConfigParameters
{
    string partInputFile;
    cl_float timeStepLength;
    cl_float timeEnd;
    cl_uint partOutFreq;
    string partOutNameBase;
    cl_uint vtkOutFreq;
    string vtkOutNameBase;
    cl_uint clWorkGroupSize1D;
    cl_float xMin;
    cl_float xMax;
    cl_float yMin;
    cl_float yMax;
    cl_float zMin;
    cl_float zMax;
    cl_float xN;
    cl_float yN;
    cl_float zN;
    cl_float restDensity;
};

#endif // __PARAMETERS_HPP
