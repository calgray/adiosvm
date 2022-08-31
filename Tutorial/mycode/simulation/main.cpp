#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// #IO# include IO library
#include "oompi.hpp"
#include <mpi.h>

#include "adios2writer.h"
#include "gray-scott.h"

void print_settings(const Settings &s)
{
    std::cout << "grid:             " << s.L << "x" << s.L << "x" << s.L
              << std::endl;
    std::cout << "steps:            " << s.steps << std::endl;
    std::cout << "plotgap:          " << s.plotgap << std::endl;
    std::cout << "F:                " << s.F << std::endl;
    std::cout << "k:                " << s.k << std::endl;
    std::cout << "dt:               " << s.dt << std::endl;
    std::cout << "Du:               " << s.Du << std::endl;
    std::cout << "Dv:               " << s.Dv << std::endl;
    std::cout << "noise:            " << s.noise << std::endl;
    std::cout << "output:           " << s.output << std::endl;
    std::cout << "adios_config:     " << s.adios_config << std::endl;
}

void print_simulator_settings(const GrayScott &s)
{
    std::cout << "process layout:   " << s.npx << "x" << s.npy << "x" << s.npz
              << std::endl;
    std::cout << "local grid size:  " << s.size_x << "x" << s.size_y << "x"
              << s.size_z << std::endl;
}

int main(int argc, char **argv)
{
    oompi::MPI mpi(&argc, &argv);
    auto world = mpi.comm_world();
    int wrank = world.rank();

    // Custom communicator group. We don't necessarily need to
    // create this but a seperate group is assigned per program
    // as cross messaging isn't needed.
    const unsigned int color = 1;
    auto comm = world.split(color, wrank);
    int rank = comm.rank();
    int procs = comm.size();

    if (argc < 2)
    {
        if (rank == 0)
        {
            std::cerr << "Too few arguments" << std::endl;
            std::cerr << "Usage: gray-scott settings.json" << std::endl;
        }
        world.abort(EXIT_FAILURE);
    }

    Settings settings = Settings::from_json(argv[1]);

    GrayScott sim(settings, comm.get());
    sim.init();

    // #IO# Need to initialize IO library
    //
    // MPIIOWriter writer(settings, sim, comm);

    adios2::ADIOS adios("adios2.xml", comm.get());
    Adios2Writer writer(settings, sim, adios.DeclareIO("SimulationOutput"));

    writer.open(settings.output);
    if (rank == 0)
    {
        writer.print_settings();
        std::cout << "========================================" << std::endl;
        print_settings(settings);
        print_simulator_settings(sim);
        std::cout << "========================================" << std::endl;
    }

    for (int i = 0; i < settings.steps;)
    {
        for (int j = 0; j < settings.plotgap; j++)
        {
            sim.iterate();
            i++;
        }

        // log on node-0
        if (rank == 0)
        {
            std::cout << "Simulation at step " << i
                      << " publishing output step     " << i / settings.plotgap
                      << std::endl;
        }
        writer.write(i, sim);
    }
    writer.close();
}
