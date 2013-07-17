#include "ConfigReader.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cassert>


using std::string;
using std::istringstream;
using std::ifstream;
using std::cout;
using std::cerr;
using std::endl;
using std::runtime_error;


ConfigParameters ConfigReader::read(const string &filename) const
{
    ConfigParameters parameters;

    size_t lineNumber = 1;
    string line; // Complete line
    string parameter; // Parameter found

    ifstream ifs( filename.c_str() );

    if ( !ifs ) {
        throw runtime_error("Could not open parameter file!");
    }

    while ( getline(ifs, line) ) {

        if ( !ifs.good() ) {
            cerr << "Error parsing parameters in line " << lineNumber << endl;
            break;
        }

        if (line.size() > 0) {
            istringstream ss(line);

            if ( !(ss >> parameter) ) {
                cerr << "Unable to read parameter" << endl;
            } else {
                if ( parameter == "part_input_file" ) {
                    ss >> parameters.partInputFile;
                } else if ( parameter == "timestep_length" ) {
                    ss >> parameters.timeStepLength;
                } else if ( parameter == "time_end" ) {
                    ss >> parameters.timeEnd;
                } else if ( parameter == "part_out_freq" ) {
                    ss >> parameters.partOutFreq;
                } else if ( parameter == "part_out_name_base" ) {
                    ss >> parameters.partOutNameBase;
                } else if ( parameter == "vtk_out_freq" ) {
                    ss >> parameters.vtkOutFreq;
                } else if ( parameter == "vtk_out_name_base" ) {
                    ss >> parameters.vtkOutNameBase;
                } else if ( parameter == "cl_workgroup_1dsize" ) {
                    ss >> parameters.clWorkGroupSize1D;
                } else if ( parameter == "x_min" ) {
                    ss >> parameters.xMin;
                } else if ( parameter == "x_max" ) {
                    ss >> parameters.xMax;
                } else if ( parameter == "y_min" ) {
                    ss >> parameters.yMin;
                } else if ( parameter == "y_max" ) {
                    ss >> parameters.yMax;
                } else if ( parameter == "z_min" ) {
                    ss >> parameters.zMin;
                } else if ( parameter == "z_max" ) {
                    ss >> parameters.zMax;
                } else if ( parameter == "x_n" ) {
                    ss >> parameters.xN;
                } else if ( parameter == "y_n" ) {
                    ss >> parameters.yN;
                } else if ( parameter == "z_n" ) {
                    ss >> parameters.zN;
                } else if ( parameter == "restdensity" ) {
                    ss >> parameters.restDensity;
                } else {
                    cerr << "Unknown parameter " << parameter << endl
                         << "Leaving it out." << endl;
                }
            }
        }

        ++lineNumber;
    }

    ifs.close();

    return parameters;
}
