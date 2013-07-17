#ifndef __PART_READER_HPP
#define __PART_READER_HPP

#include <vector>
#include <string>

#include "../hesp.hpp"
#include "../Particle.hpp"


using std::string;
using std::vector;


class PartReader
{
private:
    // Avoid copy
    PartReader& operator=(PartReader& other);
    PartReader (const PartReader &other);

public:
    PartReader () {}

    vector<Particle> read(const string &filename) const;

};

#endif // __PART_READER_HPP
