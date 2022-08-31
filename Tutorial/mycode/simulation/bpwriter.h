#ifndef __BPWRITER_H__
#define __BPWRITER_H__

// #IO# include IO library
#include <adios2.h>
#include <mpi.h>

#include "gray-scott.h"
#include "settings.h"

class BPWriter
{
public:
    BPWriter(const Settings &settings, const GrayScott &sim,
             const MPI_Comm comm, const adios2::IO &io);
    void open(const std::string &fname);
    void write(int step, const GrayScott &sim);
    void close();

    void print_settings();

protected:
    const Settings &settings;
    MPI_Comm comm;
    int err, cmode;
    MPI_Datatype memtype, filetype;

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
    adios2::Variable<int> varStep;
    adios2::Variable<double> varU;
};

#endif
