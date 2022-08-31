#include "adios2writer.h"

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

Adios2Writer::Adios2Writer(const Settings &settings, const GrayScott &sim,
                           const adios2::IO &&io)
: _settings(settings), _io(io),
  _varU(_io.DefineVariable<double>("U", {settings.L, settings.L, settings.L},
                                   {sim.offset_z, sim.offset_y, sim.offset_x},
                                   {sim.size_z, sim.size_y, sim.size_x}, true)),
  _varV(_io.DefineVariable<double>("V", {settings.L, settings.L, settings.L},
                                   {sim.offset_z, sim.offset_y, sim.offset_x},
                                   {sim.size_z, sim.size_y, sim.size_x})),
  _varStep(_io.DefineVariable<int>("step"))
{
    // #IO# make initializations, declarations as necessary
    // information from settings
    _io.DefineAttribute<double>("F", settings.F);
    _io.DefineAttribute<double>("k", settings.k);
    _io.DefineAttribute<double>("dt", settings.dt);
    _io.DefineAttribute<double>("Du", settings.Du);
    _io.DefineAttribute<double>("Dv", settings.Dv);
    _io.DefineAttribute<double>("noise", settings.noise);

    // u and v has ghost cells
    // local memory block size: {sim.size_z + 2, sim.size_y + 2, sim.size_x + 2}
    // output data starts at offset: {1, 1, 1}
    _varU.SetMemorySelection(
        {{1, 1, 1}, {sim.size_z + 2, sim.size_y + 2, sim.size_x + 2}});
    _varV.SetMemorySelection(
        {{1, 1, 1}, {sim.size_z + 2, sim.size_y + 2, sim.size_x + 2}});
}

void Adios2Writer::open(const std::string &fname)
{
    _writer = _io.Open(fname, adios2::Mode::Write);
}

void Adios2Writer::write(int step, const GrayScott &sim)
{
    /* sim.u_ghost() provides access to the U variable as is */
    /* sim.u_noghost() provides a contiguous copy without the ghost cells */
    const std::vector<double> &u = sim.u_ghost();
    const std::vector<double> &v = sim.v_ghost();
    _writer.BeginStep();
    _writer.Put(_varStep, step);
    _writer.Put(_varU, u.data());
    _writer.Put(_varV, v.data());
    _writer.EndStep();
}

void Adios2Writer::close() { _writer.Close(); }

void Adios2Writer::print_settings()
{
    std::cout << "Simulation writes data using engine type: "
              << _io.EngineType() << std::endl;
}
