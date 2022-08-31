#ifndef __WRITER_H__
#define __WRITER_H__

// #IO# include IO library
#include <adios2.h>
#include <mpi.h>

#include "gray-scott.h"
#include "settings.h"

class MPIIOWriter
{
public:
    MPIIOWriter(const Settings &settings, const GrayScott &sim,
                const MPI_Comm comm);
    void open(const std::string &fname);
    void write(int step, const GrayScott &sim);
    void close();

    void print_settings();

protected:
    const Settings &settings;
    MPI_Comm comm;
    int nproc, rank;
    int err, cmode;
    MPI_File fh;
    MPI_Datatype memtype, filetype;

    struct header
    {
        // original coder used z as slowest dim, x as fastest!!!
        unsigned long long z;
        unsigned long long y;
        unsigned long long x;
    };
};

#endif
