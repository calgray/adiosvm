#include "bpwriter.h"

#include <iostream>

#define CHECK_ERR(func)                                                        \
    {                                                                          \
        if (err != MPI_SUCCESS)                                                \
        {                                                                      \
            int errorStringLen;                                                \
            char errorString[MPI_MAX_ERROR_STRING];                            \
            MPI_Error_string(err, errorString, &errorStringLen);               \
            printf("Error at line %d: calling %s (%s)\n", __LINE__, #func,     \
                   errorString);                                               \
        }                                                                      \
    }

BPWriter::BPWriter(const Settings &settings, const GrayScott &sim,
                   const MPI_Comm comm, const adios2::IO &io)
: settings(settings), comm(comm), _io(io)
{
    // #IO# make initializations, declarations as necessary
    // information from settings
    // <double>("F", settings.F);
    // <double>("k", settings.k);
    // <double>("dt", settings.dt);
    // <double>("Du", settings.Du);
    // <double>("Dv", settings.Dv);
    // <double>("noise", settings.noise);

    // 3D variables
    // <double> U, V
    // global dimensions: {settings.L, settings.L, settings.L},
    // starting offset:   {sim.offset_z, sim.offset_y, sim.offset_x},
    // local block size:  {sim.size_z, sim.size_y, sim.size_x}

    // u and v has ghost cells
    // local memory block size: {sim.size_z + 2, sim.size_y + 2, sim.size_x + 2}
    // output data starts at offset: {1, 1, 1}

    // information: <int> step   is a single value

    varStep = _io.DefineVariable<int>("step");
    varU =
        _io.DefineVariable<double>("U", {settings.L, settings.L, settings.L},
                                   {sim.offset_z, sim.offset_y, sim.offset_x},
                                   {sim.size_z, sim.size_y, sim.size_x}, true);
    varU.SetMemorySelection(
        {{1, 1, 1}, {sim.size_z + 2, sim.size_y + 2, sim.size_x + 2}});
    _io.DefineAttribute<double>("F", settings.F);

    /*
     *  MPI Subarray data type for writing/reading parallel distributed arrays
     *  See case II for array with ghost cells:
     *  https://wgropp.cs.illinois.edu/courses/cs598-s15/lectures/lecture33.pdf
     */

    int mshape[3] = {(int)sim.size_z + 2, (int)sim.size_y + 2,
                     (int)sim.size_x + 2};
    int mstart[3] = {1, 1, 1};
    int mcount[3] = {(int)sim.size_z, (int)sim.size_y, (int)sim.size_x};
    err = MPI_Type_create_subarray(3, mshape, mcount, mstart, MPI_ORDER_C,
                                   MPI_DOUBLE, &memtype);
    CHECK_ERR(MPI_Type_create_subarray for memory type)
    err = MPI_Type_commit(&memtype);
    CHECK_ERR(MPI_Type_commit for memory type)

    int fshape[3] = {(int)settings.L, (int)settings.L, (int)settings.L};
    int fstart[3] = {(int)sim.offset_z, (int)sim.offset_y, (int)sim.offset_x};
    int fcount[3] = {(int)sim.size_z, (int)sim.size_y, (int)sim.size_x};
    err = MPI_Type_create_subarray(3, fshape, fcount, fstart, MPI_ORDER_C,
                                   MPI_DOUBLE, &filetype);
    CHECK_ERR(MPI_Type_create_subarray for file type)
    err = MPI_Type_commit(&filetype);
    CHECK_ERR(MPI_Type_commit for file type)
}

void BPWriter::open(const std::string &fname)
{
    _writer = _io.Open(fname, adios2::Mode::Write);
}

void BPWriter::write(int step, const GrayScott &sim)
{
    /* sim.u_ghost() provides access to the U variable as is */
    /* sim.u_noghost() provides a contiguous copy without the ghost cells */
    const std::vector<double> &u = sim.u_ghost();
    const std::vector<double> &v = sim.v_ghost();
    _writer.BeginStep();
    _writer.Put(varStep, step);
    _writer.Put(varU, u.data());
    _writer.EndStep();
}

void BPWriter::close() { _writer.Close(); }

void BPWriter::print_settings()
{
    std::cout << "Simulation writes data using engine type: "
              << _io.EngineType() << std::endl;
}
