#ifndef __DATALOADER_HPP
#define __DATALOADER_HPP

#include <string>
using std::string;

class DataLoader
{
public:
    DataLoader();
    ~DataLoader();

    const string getPathForScenario(const string scenario);
    const string getPathForKernel(const string kernel);
    const string getPathForShader(const string shader);
    const string getPathForTexture(const string texture);
private:
    string rootDirectory;
};

#endif // __DATALOADER_HPP
