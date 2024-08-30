#include <array>

#include "traj_reader/reader.hpp"

/*
 * Returns x^3
 */
template <typename T>
T cube(const T x) noexcept
{
    return x * x * x;
}

// Number of grids for post-processing
constexpr auto N_grids{7};

// Grids for post-processing
std::array<int, N_grids> grids = {3, 5, 7, 10, 15, 21, 32};

/*
 * Output data structure
 */
struct Data
{
    std::array<std::vector<double>, N_grids> density;

    Data()
    {
        for (int i = 0; i < N_grids; ++i)
        {
            density[i].resize(cube(grids[i]), 0.0);
        }
    }
};

/*
 * Entry point
 */
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

    // Box volume
    double V0 = cube<double>(L0); // [nm^3]

    // Output data
    Data data;

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
            float mass = trj[i].mass[n]; // Atom's mass

            traj_reader::float_vec r = trj[i].r[n]; // Coordinate vector
            traj_reader::float_vec v = trj[i].v[n]; // Velocity vector

            // traj_reader::float_vec f = trj[i].f[n]; // Force vector - NOT USED

            // PBC
            while (r.x < 0.0)
            {
                r.x += L;
            }
            while (r.y < 0.0)
            {
                r.y += L;
            }
            while (r.z < 0.0)
            {
                r.z += L;
            }
            while (r.x >= L)
            {
                r.x -= L;
            }
            while (r.y >= L)
            {
                r.y -= L;
            }
            while (r.z >= L)
            {
                r.z -= L;
            }

            // Loop over grids
            for (int ng = 0; ng < N_grids; ++ng) // for (const auto &N : grids)
            {
                // Control volumes: N x N x N
                int N = grids[ng];

                // Local indexes
                int i = (r.x / L) * N;
                int j = (r.y / L) * N;
                int k = (r.z / L) * N;

                // Should never happen
                if (i < 0 || i >= N || j < 0 || j >= N || k < 0 || k >= N)
                {
                    std::cerr << "\nERROR: Incorrect Control Volume index.\n";
                    exit(1);
                }

                // Global index (0..N^3-1)
                int ind = i + N * j + N * N * k;

                // CV volume [nm]
                const double V = cube<double>(L / N);

                // Collect data
                data.density[ng][ind] += mass / V;

            } // Grids
        } // Atoms
    } // Frames

    return 0;
}
