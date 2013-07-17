#ifndef __RUNNER_HPP
#define __RUNNER_HPP

#include "hesp.hpp"
#include "Parameters.hpp"
#include "Simulation.hpp"
#include "visual/visual.hpp"


class Runner
{
private:
    // Avoid copy
    Runner &operator=(const Runner &other);
    Runner (const Runner &other);

public:
    Runner () {}

    void run(const ConfigParameters &parameters,
             Simulation &simulation,
             CVisual &renderer) const;

};

#endif // __RUNNER_HPP
