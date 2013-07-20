#include "DataLoader.hpp"

#include <mach-o/dyld.h>
#include <limits.h>
#include <libgen.h>

DataLoader::DataLoader () {
  char path[PATH_MAX + 1];
  char absolute_path[PATH_MAX + 1];
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) == 0) {
    realpath(path, absolute_path);
  } else {
    //TODO: Throw error
  }

  rootDirectory = dirname(absolute_path);
}

DataLoader::~DataLoader() {}

const string DataLoader::getPathForScenario(const string scenario) {
  return rootDirectory + "/data/scenarios/" + scenario;
}
const string DataLoader::getPathForKernel(const string kernel) {
  return rootDirectory + "/data/kernels/" + kernel;
}
const string DataLoader::getPathForShader(const string shader) {
  return rootDirectory + "/data/shaders/" + shader;
}
const string DataLoader::getPathForTexture(const string texture) {
  return rootDirectory + "/data/textures/" + texture;
}
