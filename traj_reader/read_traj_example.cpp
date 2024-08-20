#include "traj_reader/reader.hpp"

int main()
{
    traj_reader::mass m;
    traj_reader::traj trj;

    int natoms = traj_reader::read("traj.000001.out", trj, m);

    // Number of atoms
    std::cout << "N_atoms = " << natoms << "\n\n";

    // Print first 5 atom masses
    for (int i = 0; i < 5; i++)
    {
        std::cout << "Atom " << i << ": m = " << m[i] << "\n";
    }

    // Print number of frames
    std::cout << "\nNumber of frames: " << trj.size() << "\n\n";

    // Print data from the last frame
    {
        int i = trj.size() - 1; // Frame index

        // Print the frame header (Frame number, current time step, number of atoms, current time, box size)
        std::cout << "Frame " << i << ":"
                  << " step=" << trj[i].step
                  << "; N_atoms=" << trj[i].natoms
                  << "; time=" << trj[i].time << " ps"
                  << "; box size: " << trj[i].box.x << " x " << trj[i].box.y << " x " << trj[i].box.z << " nm\n";

        // Print coordinates, velocities, forces for first 5 atoms in the frame
        for (int n = 0; n < 5; n++)
        {
            std::cout << " -- atom #" << n << ": "
                      << "coord = (" << trj[i].r[n].x << ", " << trj[i].r[n].y << ", " << trj[i].r[n].z << ");  "
                      << "velocity = (" << trj[i].v[n].x << ", " << trj[i].v[n].y << ", " << trj[i].v[n].z << ");  "
                      << "force = (" << trj[i].f[n].x << ", " << trj[i].f[n].y << ", " << trj[i].f[n].z << ")\n";
        }
    }

    return 0;
}
