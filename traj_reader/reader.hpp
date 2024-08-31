#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define NO_MD_FORCES

namespace traj_reader
{

/*
 * Base type - single precision
 */
typedef float float_type;

/*
 * 3D vector
 */
struct float_vec
{
    float_type x, y, z;
};

/*
 * Contains data related to a single frame
 */
struct frame
{
    int natoms{0}; // Number of atoms
    int step{0};   // Current time step

    float_type time{0.0}; // Current time

    float_vec box; // Box size

    std::vector<float_type> mass; // Mass

    std::vector<float_vec> r; // Coordinate
    std::vector<float_vec> v; // Velocity
#ifdef MD_FORCES
    std::vector<float_vec> f; // Force
#endif
};

/*
 * A vector of frames - trajectory
 */
typedef std::vector<frame> traj;

/*
 * Reads trajectory from the output file.
 * Returns the number of atoms or 0 in case of error.
 */
int read(const std::string &fname, traj &trj)
{
    // Open the binary file for reading
    std::ifstream in_file(fname, std::ios::binary);

    // Check if the file opened successfully
    if (!in_file && in_file.is_open())
    {
        std::cerr << "Error opening file for reading: " << fname << std::endl;
        return 0;
    }

    // Create an empty frame
    frame f;

    // Number of atoms
    int natoms{0};

    // Read file
    while (!in_file.eof())
    {
        // Read header
        in_file.read(reinterpret_cast<char *>(&f.step), sizeof(f.step));
        in_file.read(reinterpret_cast<char *>(&f.natoms), sizeof(f.natoms));
        in_file.read(reinterpret_cast<char *>(&f.time), sizeof(f.time));
        in_file.read(reinterpret_cast<char *>(&f.box.x), sizeof(f.box.x));
        in_file.read(reinterpret_cast<char *>(&f.box.y), sizeof(f.box.y));
        in_file.read(reinterpret_cast<char *>(&f.box.z), sizeof(f.box.z));

        if (f.natoms <= 0)
        {
            std::cerr << "Error in trajectory file " << fname << ": Natoms = " << f.natoms << std::endl;
            return 0;
        }

        // Allocate memory for the frame if needed
        if (f.mass.size() == 0)
        {
            natoms = f.natoms; // Number of atoms should not change
            f.mass.resize(natoms);
            f.r.resize(natoms);
            f.v.resize(natoms);
#ifdef MD_FORCES
            f.f.resize(natoms);
#endif
        }

        if (natoms != f.natoms)
        {
            std::cerr << "Error in trajectory file " << fname << ": Inconsistent number of atoms." << std::endl;
            return 0;
        }

        // Read frame
        for (int n = 0; n < natoms; n++)
        {
            in_file.read(reinterpret_cast<char *>(&f.mass[n]), sizeof(float_type));
            in_file.read(reinterpret_cast<char *>(&f.r[n].x), sizeof(float_type));
            in_file.read(reinterpret_cast<char *>(&f.v[n].x), sizeof(float_type));
#ifdef MD_FORCES
            in_file.read(reinterpret_cast<char *>(&f.f[n].x), sizeof(float_type));
#endif
            in_file.read(reinterpret_cast<char *>(&f.r[n].y), sizeof(float_type));
            in_file.read(reinterpret_cast<char *>(&f.v[n].y), sizeof(float_type));
#ifdef MD_FORCES
            in_file.read(reinterpret_cast<char *>(&f.f[n].y), sizeof(float_type));
#endif
            in_file.read(reinterpret_cast<char *>(&f.r[n].z), sizeof(float_type));
            in_file.read(reinterpret_cast<char *>(&f.v[n].z), sizeof(float_type));
#ifdef MD_FORCES
            in_file.read(reinterpret_cast<char *>(&f.f[n].z), sizeof(float_type));
#endif
        }

        // Add frame to the trajectory
        if (!in_file.eof())
        {
            trj.emplace_back(f);
        }
    }

    // Close the file
    in_file.close();

    return natoms;
}

} // namespace traj_reader
