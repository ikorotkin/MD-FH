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
constexpr auto N_grids{3};

// Grids for post-processing
std::array<int, N_grids> grids = {0, 0, 0};

/*
 * Output data structure
 */
struct Data
{
    std::array<std::vector<double>, N_grids> dens;

    std::array<std::vector<double>, N_grids> mom_x;
    std::array<std::vector<double>, N_grids> mom_y;
    std::array<std::vector<double>, N_grids> mom_z;

    std::array<std::vector<double>, N_grids> ekin_xx;
    std::array<std::vector<double>, N_grids> ekin_yy;
    std::array<std::vector<double>, N_grids> ekin_zz;
    std::array<std::vector<double>, N_grids> ekin_xy;
    std::array<std::vector<double>, N_grids> ekin_xz;
    std::array<std::vector<double>, N_grids> ekin_yz;

    std::array<double, N_grids> cell_volume;

    double time{0.0};

    int step{0};

    /*
     * Resets statistics
     */
    void reset()
    {
        for (int i = 0; i < N_grids; ++i)
        {
            dens[i].clear();
            mom_x[i].clear();
            mom_y[i].clear();
            mom_z[i].clear();
            ekin_xx[i].clear();
            ekin_yy[i].clear();
            ekin_zz[i].clear();
            ekin_xy[i].clear();
            ekin_xz[i].clear();
            ekin_yz[i].clear();

            dens[i].resize(cube(grids[i]), 0.0);
            mom_x[i].resize(cube(grids[i]), 0.0);
            mom_y[i].resize(cube(grids[i]), 0.0);
            mom_z[i].resize(cube(grids[i]), 0.0);
            ekin_xx[i].resize(cube(grids[i]), 0.0);
            ekin_yy[i].resize(cube(grids[i]), 0.0);
            ekin_zz[i].resize(cube(grids[i]), 0.0);
            ekin_xy[i].resize(cube(grids[i]), 0.0);
            ekin_xz[i].resize(cube(grids[i]), 0.0);
            ekin_yz[i].resize(cube(grids[i]), 0.0);

            cell_volume[i] = 0.0;
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
 * Writes a single float value in binary format
 */
inline void write_float(std::ofstream &f, float val)
{
    f.write(reinterpret_cast<char *>(&val), sizeof(float));
}

/*
 * Saves data to data files in binary format
 */
int save_data_bin(const std::vector<Data> &collection)
{
    for (int ng = 0; ng < N_grids; ++ng)
    {
        std::ofstream outfile;

        std::string fname = "output_" + std::to_string(grids[ng]) + ".dat";

        outfile.open(fname, std::ios::binary);

        if (outfile && outfile.is_open())
        {
            for (const auto &data : collection)
            {
                write_float(outfile, data.time);
                write_float(outfile, data.cell_volume[ng]);

                for (int ind = 0; ind < data.dens[ng].size(); ++ind)
                {
                    write_float(outfile, data.dens[ng][ind]);
                    write_float(outfile, data.mom_x[ng][ind]);
                    write_float(outfile, data.mom_y[ng][ind]);
                    write_float(outfile, data.mom_z[ng][ind]);
                    write_float(outfile, data.ekin_xx[ng][ind]);
                    write_float(outfile, data.ekin_yy[ng][ind]);
                    write_float(outfile, data.ekin_zz[ng][ind]);
                    write_float(outfile, data.ekin_xy[ng][ind]);
                    write_float(outfile, data.ekin_xz[ng][ind]);
                    write_float(outfile, data.ekin_yz[ng][ind]);
                }
            }

            outfile.close();
        }
        else
        {
            return -1; // Error opening file
        }
    }

    return 0;
}

/*
 * Entry point
 */
int main(int argc, char *argv[])
{
    // Check if filename and box size arguments are provided
    if (argc < 3)
    {
        std::cerr << "ERROR: No file name and/or box size provided.\n";
        return 1;
    }

    // Copy the file name and the box size
    std::string filename = argv[1];
    std::string boxsize = argv[2];

    // Set grids for averaging
    if (boxsize == "7")
    {
        grids = {2, 5, 10};
    }
    else if (boxsize == "10")
    {
        grids = {3, 7, 15};
    }
    else if (boxsize == "15")
    {
        grids = {5, 10, 22};
    }
    else
    {
        std::cerr << "ERROR: Box size (second argument) should be 7, 10 or 15.\n";
        return 1;
    }

    std::cout << "\nREADER: Reading " << filename << " (L = " << boxsize << " nm)..." << std::endl;

    // Array of frames
    traj_reader::traj trj;

    // Reads trajectory
    int natoms = traj_reader::read(filename, trj);

    // Number of frames
    int nframes = trj.size();

    std::cout << "\n"
              << nframes << " frame(s), " << natoms << " atoms in each frame.\n";

    if (nframes == 0)
    {
        std::cerr << "\nERROR: Number of frames is 0.\n";
        return 1;
    }

    if (natoms == 0)
    {
        std::cerr << "\nERROR: Number of atoms is 0.\n";
        return 1;
    }

    // Box size
    float L0 = trj[0].box.x; // [nm]

    // Box volume
    double V0 = cube<double>(L0); // [nm^3]

    std::cout << "\nREADER: Processing..." << std::endl;

    // Output data
    Data data;
    std::vector<Data> collection;

    // Loop over frames
    for (int i = 0; i < nframes; ++i)
    {
        // Reset statistics
        data.reset();

        // Frame header (frame number, current time step, number of atoms, current time, box size)
        int step = trj[i].step;
        int atoms = trj[i].natoms;
        float time = trj[i].time; // [ps]
        float L = trj[i].box.x;   // [nm]

        if ((atoms != natoms) || (natoms != trj[i].r.size()))
        {
            std::cerr << "\nERROR: Inconsistent number of atoms.\n";
            return 1;
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
                    return 1;
                }

                // Global index (0..N^3-1)
                int ind = i + N * j + N * N * k;

                // Density
                data.dens[ng][ind] += mass;

                // Momentum
                data.mom_x[ng][ind] += mass * v.x;
                data.mom_y[ng][ind] += mass * v.y;
                data.mom_z[ng][ind] += mass * v.z;

                // Velocity tensor
                data.ekin_xx[ng][ind] += mass * v.x * v.x;
                data.ekin_yy[ng][ind] += mass * v.y * v.y;
                data.ekin_zz[ng][ind] += mass * v.z * v.z;
                data.ekin_xy[ng][ind] += mass * v.x * v.y;
                data.ekin_xz[ng][ind] += mass * v.x * v.z;
                data.ekin_yz[ng][ind] += mass * v.y * v.z;

                // Time and step
                data.time = time;
                data.step = step;

            } // Grids

        } // Atoms

        // Update averaged values.
        // Loop over grids.
        for (int ng = 0; ng < N_grids; ++ng)
        {
            // Control volumes: N x N x N
            int N = grids[ng];
            int N3 = cube<int>(N);

            // CV volume [nm]
            const double V_cell = cube<double>(L / N);

            data.cell_volume[ng] = V_cell;

            // Loop over cells in the grid
            for (int ind = 0; ind < N3; ++ind)
            {
                data.dens[ng][ind] /= V_cell;

                data.mom_x[ng][ind] /= V_cell;
                data.mom_y[ng][ind] /= V_cell;
                data.mom_z[ng][ind] /= V_cell;

                data.ekin_xx[ng][ind] /= (data.dens[ng][ind] * V_cell);
                data.ekin_yy[ng][ind] /= (data.dens[ng][ind] * V_cell);
                data.ekin_zz[ng][ind] /= (data.dens[ng][ind] * V_cell);
                data.ekin_xy[ng][ind] /= (data.dens[ng][ind] * V_cell);
                data.ekin_xz[ng][ind] /= (data.dens[ng][ind] * V_cell);
                data.ekin_yz[ng][ind] /= (data.dens[ng][ind] * V_cell);
            }
        }

        // Add frame to collection of frames
        try
        {
            collection.emplace_back(data);
        }
        catch (const std::exception &e)
        {
            std::cerr << "\nERROR: " << e.what() << "\nOut of memory?\n";
        }

    } // Frames

    std::cout << "\n...done.\n";
    std::cout << "\nREADER: Writing..." << std::endl;

    if (save_data_bin(collection))
    {
        std::cerr << "\nERROR: Could not save data.\n";
        return 1;
    }

    std::cout << "\n...done.\n";

    return 0;
}
