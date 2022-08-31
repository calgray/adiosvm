#ifndef __BPWRITER_H__
#define __BPWRITER_H__

// #IO# include IO library
#include <adios2.h>
#include <mpi.h>

#include "gray-scott.h"
#include "settings.h"

/**
 * @brief ADIOS2 Tutorial writer for writing to a custom .bp
 * file format.
 *
 */
class Adios2Writer
{
public:
    Adios2Writer(const Settings &settings, const GrayScott &sim,
                 const adios2::IO &&io);
    void open(const std::string &fname);
    void write(int step, const GrayScott &sim);
    void close();

    void print_settings();

protected:
    const Settings &_settings;

    struct header
    {
        // original coder used z as slowest dim, x as fastest!!!
        unsigned long long z;
        unsigned long long y;
        unsigned long long x;
    };

    //#IO# declare ADIOS variables for engine, io, variables
    adios2::IO _io;
    adios2::Engine _writer;
    // 3D variables
    adios2::Variable<double> _varU;
    adios2::Variable<double> _varV;
    // information: <int> step   is a single value
    adios2::Variable<int> _varStep;
};

#endif
