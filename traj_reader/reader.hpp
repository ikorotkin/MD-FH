#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace traj_reader
{

typedef float float_type;

struct float_vec
{
    float_type x, y, z;
};

typedef std::vector<float> mass; // Mass

struct frame
{
    int natoms{0};   // Number of atoms
    int step{0};     // Current time step
    float time{0.0}; // Current time

    float_vec box; // Box size

    std::vector<float_vec> r; // Coordinate
    std::vector<float_vec> v; // Velocity
    std::vector<float_vec> f; // Force
};

typedef std::vector<frame> traj; // Trajectory

/*
 * Reads trajectory from the output file
 */
int read(const std::string &fname, traj &trj, mass &m)
{
    // Open the binary file for reading
    std::ifstream in_file(fname, std::ios::binary);

    // Check if the file opened successfully
    if (!in_file && in_file.is_open())
    {
        std::cerr << "Error opening file for reading: " << fname << std::endl;
        return 0;
    }

    // Read number of atoms
    int natoms{0};
    in_file.read(reinterpret_cast<char *>(&natoms), sizeof(natoms));

    // Read atom masses
    m.clear();
    m.resize(natoms);
    const int m_size = sizeof(m[0]);
    for (int n = 0; n < natoms; n++)
    {
        in_file.read(reinterpret_cast<char *>(&m[n]), m_size);
    }

    // Create a frame and allocate memory
    frame f;
    f.r.resize(natoms);
    f.v.resize(natoms);
    f.f.resize(natoms);

    while (!in_file.eof())
    {
        // Read header
        in_file.read(reinterpret_cast<char *>(&f.step), sizeof(f.step));
        in_file.read(reinterpret_cast<char *>(&f.natoms), sizeof(f.natoms));
        in_file.read(reinterpret_cast<char *>(&f.time), sizeof(f.time));
        in_file.read(reinterpret_cast<char *>(&f.box.x), sizeof(f.box.x));
        in_file.read(reinterpret_cast<char *>(&f.box.y), sizeof(f.box.y));
        in_file.read(reinterpret_cast<char *>(&f.box.z), sizeof(f.box.z));

        const int float_type_size = sizeof(float_type);

        // Read frame
        for (int n = 0; n < natoms; n++)
        {
            in_file.read(reinterpret_cast<char *>(&f.r[n].x), float_type_size);
            in_file.read(reinterpret_cast<char *>(&f.v[n].x), float_type_size);
            in_file.read(reinterpret_cast<char *>(&f.f[n].x), float_type_size);
            in_file.read(reinterpret_cast<char *>(&f.r[n].y), float_type_size);
            in_file.read(reinterpret_cast<char *>(&f.v[n].y), float_type_size);
            in_file.read(reinterpret_cast<char *>(&f.f[n].y), float_type_size);
            in_file.read(reinterpret_cast<char *>(&f.r[n].z), float_type_size);
            in_file.read(reinterpret_cast<char *>(&f.v[n].z), float_type_size);
            in_file.read(reinterpret_cast<char *>(&f.f[n].z), float_type_size);
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
