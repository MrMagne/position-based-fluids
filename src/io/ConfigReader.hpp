#ifndef __CONFIG_READER_HPP
#define __CONFIG_READER_HPP

#include <string>

#include "../hesp.hpp"
#include "../Parameters.hpp"


using std::string;


class ConfigReader
{
private:
    // Avoid copy
    ConfigReader& operator=(const ConfigReader& other);
    ConfigReader (const ConfigReader &other);

public:
    ConfigReader () {}

    ConfigParameters read(const string &filename) const;

};

#endif // __CONFIG_READER_HPP
