#pragma once
#include <mpi.h>
#include <numeric>
#include <stdexcept>

namespace oompi
{
class Error : public std::runtime_error
{
public:
    Error(int mpicode) : std::runtime_error(std::to_string(mpicode)) {}
};

class Communicator
{
    MPI_Comm _comm;
    int _rank;
    int _size;

public:
    Communicator(MPI_Comm comm) : _comm(comm) {}
    MPI_Comm &get() { return _comm; }
    const MPI_Comm &get() const { return _comm; }
    int rank() const
    {
        int rank;
        MPI_Comm_rank(_comm, &rank);
        return _rank;
    }
    int size() const
    {
        int size;
        int err;
        if (err = MPI_Comm_size(_comm, &size))
        {
            throw Error(err);
        }
        return size;
    }
    Communicator split(unsigned int color, int rank)
    {
        MPI_Comm comm;
        int err;
        if (err = MPI_Comm_split(_comm, color, rank, &comm))
        {
            throw Error(err);
        }
        return Communicator(comm);
    }
    void abort(int errcode)
    {
        int err;
        if (err = MPI_Abort(_comm, errcode))
        {
            throw Error(err);
        }
    }
};

class MPI
{
public:
    MPI(int *argc, char ***argv)
    {
        int err;
        if (err = MPI_Init(argc, argv))
        {
            throw Error(err);
        }
    }
    ~MPI()
    {
        int err;
        if (err = MPI_Finalize())
        {
            std::cerr << "MPI Finalize returned " << err << "\n";
        }
    }

    constexpr Communicator comm_world() { return Communicator(MPI_COMM_WORLD); }
};
}
