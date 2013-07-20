#include "PartReader.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <limits>


using std::string;
using std::vector;
using std::istringstream;
using std::ifstream;
using std::cout;
using std::cerr;
using std::endl;
using std::runtime_error;
using std::numeric_limits;


vector<Particle> PartReader::read(const string &filename) const {
  vector<Particle> particles; // Return vector
  string line; // Complete line

  ifstream ifs(filename.c_str());

  if ( !ifs ) {
    throw runtime_error("Could not open particle file!");
  }

  // Get number of particles
  size_t numParticles = 0;
  Particle particle;

  getline(ifs, line);

  if ( !ifs ) {
    throw runtime_error("Could not parse number of particles!");
  }

  istringstream ssn(line);
  ssn >> numParticles;

  for (size_t lineNumber = 0; lineNumber < numParticles; ++lineNumber) {

    if ( !ifs ) {
      cerr << "Error parsing particle in line "
           << (lineNumber + 2) << endl;
      break;
    }

    getline(ifs, line);
    istringstream ss(line);

    // mass
    string tmpStr;

    ss >> tmpStr;

    ss.str(line);
    ss >> particle.m;

    if ( ss.fail() ) {
      cerr << "Error parsing particle mass in line "
           << (lineNumber + 2) << endl;
      break;
    }

    // position vector
    for (int i = 0; i < DIMENSIONS; ++i) {
      ss >> particle.x[i];

      if ( ss.fail() ) {
        cerr << "Error parsing particle position " << i << " in line "
             << (lineNumber + 2) << endl;
        break;
      }
    }

    // velocity vector
    for (int i = 0; i < DIMENSIONS; ++i) {
      ss >> particle.v[i];

      if ( ss.fail() ) {
        cerr << "Error parsing particle velocity " << i << " in line "
             << (lineNumber + 2) << endl;
        break;
      }
    }

    particles.push_back(particle);
  }

  ifs.close();

  return particles;
}
