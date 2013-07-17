#ifndef __PARAMETERS_HPP
#define __PARAMETERS_HPP

#include <string>

#include "hesp.hpp"


using std::string;


struct ConfigParameters {
    string partInputFile;
    hesp_float timeStepLength;
    hesp_float timeEnd;
    cl_uint partOutFreq;
    string partOutNameBase;
    cl_uint vtkOutFreq;
    string vtkOutNameBase;
    cl_uint clWorkGroupSize1D;
    hesp_float xMin;
    hesp_float xMax;
    hesp_float yMin;
    hesp_float yMax;
    hesp_float zMin;
    hesp_float zMax;
    hesp_float xN;
    hesp_float yN;
    hesp_float zN;
    hesp_float restDensity;
};

#endif // __PARAMETERS_HPP
