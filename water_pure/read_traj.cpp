#include "traj_reader/reader.hpp"

inline double cube(const double x) noexcept
{
    return x * x * x;
}

int main()
{
    traj_reader::traj trj; // Array of frames

    std::string filename = "traj.000000.out";

    std::cout << "Reading " << filename << ": " << std::flush;

    // Reads trajectory
    int natoms = traj_reader::read(filename, trj);

    // Number of frames
    int nframes = trj.size();

    // Number of atoms
    std::cout << nframes << " frame(s), " << natoms << " atoms in each frame.\n";

    if (nframes == 0)
    {
        std::cerr << "\nERROR: Number of frames is 0.\n";
        exit(1);
    }

    if (natoms == 0)
    {
        std::cerr << "\nERROR: Number of atoms is 0.\n";
        exit(1);
    }

    // Box size
    float L0 = trj[0].box.x; // [nm]

    // Volume
    double V0 = cube(L0); // [nm^3]

    // Density
    // std::vector<double> dens
    double dens = 0;

    // Loop over frames
    for (int i = 0; i < nframes; ++i)
    {
        // Frame header (Frame number, current time step, number of atoms, current time, box size)
        int step = trj[i].step;
        int atoms = trj[i].natoms;
        float time = trj[i].time; // [ps]
        float L = trj[i].box.x;   // [nm]

        if ((atoms != natoms) || (natoms != trj[i].r.size()))
        {
            std::cerr << "\nERROR: Inconsistent number of atoms.\n";
            exit(1);
        }

        // Loop over atoms
        for (int n = 0; n < natoms; ++n)
        {
            float m = trj[i].m[n]; // Atom's mass

            traj_reader::float_vec r = trj[i].r[n]; // Coordinate vector
            traj_reader::float_vec v = trj[i].v[n]; // Velocity vector
            // traj_reader::float_vec f = trj[i].f[n]; // Force vector - NOT USED

            dens += m;
        }
    }

    std::cout << "Density: " << dens / nframes / V0 * 1.66 << '\n';

    return 0;
}
