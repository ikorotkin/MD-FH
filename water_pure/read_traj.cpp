#include <array>
#include <fstream>

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

    std::array<std::vector<double>, N_grids> mom_x;
    std::array<std::vector<double>, N_grids> mom_y;
    std::array<std::vector<double>, N_grids> mom_z;

    std::array<std::vector<double>, N_grids> ekin_xx;
    std::array<std::vector<double>, N_grids> ekin_yy;
    std::array<std::vector<double>, N_grids> ekin_zz;
    std::array<std::vector<double>, N_grids> ekin_xy;
    std::array<std::vector<double>, N_grids> ekin_xz;
    std::array<std::vector<double>, N_grids> ekin_yz;

    double time{0.0};

    int step{0};

    /*
     * Resets statistics
     */
    void reset()
    {
        for (int i = 0; i < N_grids; ++i)
        {
            density[i].clear();
            mom_x[i].clear();
            mom_y[i].clear();
            mom_z[i].clear();
            ekin_xx[i].clear();
            ekin_yy[i].clear();
            ekin_zz[i].clear();
            ekin_xy[i].clear();
            ekin_xz[i].clear();
            ekin_yz[i].clear();

            density[i].resize(cube(grids[i]), 0.0);
            mom_x[i].resize(cube(grids[i]), 0.0);
            mom_y[i].resize(cube(grids[i]), 0.0);
            mom_z[i].resize(cube(grids[i]), 0.0);
            ekin_xx[i].resize(cube(grids[i]), 0.0);
            ekin_yy[i].resize(cube(grids[i]), 0.0);
            ekin_zz[i].resize(cube(grids[i]), 0.0);
            ekin_xy[i].resize(cube(grids[i]), 0.0);
            ekin_xz[i].resize(cube(grids[i]), 0.0);
            ekin_yz[i].resize(cube(grids[i]), 0.0);
        }

        time = 0.0;
        step = 0;
    }

    Data()
    {
        reset();
    }
};

/*
 * Saves data to data files
 */
int save_data(const std::vector<Data> &collection)
{
    for (int ng = 0; ng < N_grids; ++ng)
    {
        std::ofstream outfile;

        std::string fname = "output_" + std::to_string(grids[ng]) + ".dat";

        outfile.open(fname, std::ios::app);

        constexpr char delimiter = '\t';

        outfile << "time_step" << delimiter << "time" << delimiter
                << "density" << delimiter
                << "m_x" << delimiter << "m_y" << delimiter << "m_z" << delimiter
                << "Ekin_xx" << delimiter << "Ekin_yy" << delimiter << "Ekin_zz" << delimiter
                << "Ekin_xy" << delimiter << "Ekin_xz" << delimiter << "Ekin_yz" << '\n';

        std::size_t size = collection.size();

        for (const auto &data : collection)
        {
            outfile << data.step << delimiter << data.time;

            for (int ind = 0; ind < data.density[ng].size() - 1; ++ind)
            {
                outfile << delimiter << data.density[ng][ind];
            }

            outfile << '\n';
        }

        outfile.close();
    }

    return 0;
}

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

    std::cout << "Processing..." << std::flush;

    // Output data
    Data data;
    std::vector<Data> collection;

    // Loop over frames
    for (int i = 0; i < nframes; ++i)
    {
        // Reset statistics
        data.reset();

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
                const double V_cell = cube<double>(L / N);

                // Density
                data.density[ng][ind] += mass / V_cell;

                // Momentum
                data.mom_x[ng][ind] += mass * v.x / V_cell;
                data.mom_y[ng][ind] += mass * v.y / V_cell;
                data.mom_z[ng][ind] += mass * v.z / V_cell;

                // Kinetic energy tensor
                data.ekin_xx[ng][ind] += 0.5 * mass * v.x * v.x / V_cell;
                data.ekin_yy[ng][ind] += 0.5 * mass * v.y * v.y / V_cell;
                data.ekin_zz[ng][ind] += 0.5 * mass * v.z * v.z / V_cell;
                data.ekin_xy[ng][ind] += 0.5 * mass * v.x * v.y / V_cell;
                data.ekin_xz[ng][ind] += 0.5 * mass * v.x * v.z / V_cell;
                data.ekin_yz[ng][ind] += 0.5 * mass * v.y * v.z / V_cell;

                // Time and step
                data.time = time;
                data.step = step;

            } // Grids

        } // Atoms

        collection.emplace_back(data);

    } // Frames

    std::cout << "done.\n";
    std::cout << "Writing..." << std::flush;

    if (save_data(collection))
    {
        std::cerr << "\nERROR: Could not save data.\n";
        exit(1);
    };

    std::cout << "done.\n";

    return 0;
}
