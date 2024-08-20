#include "traj_reader/reader.hpp"

int main()
{
    traj_reader::mass m;
    traj_reader::traj trj;

    int natoms = traj_reader::read("traj.000001.out", trj, m);

    std::cout << "natoms = " << natoms << "\n";

    for(int i = 0; i < 10; i++)
    {
        std::cout << i << ":" << m[i] << "\n";
    }
    for(int i = natoms - 5; i < natoms; i++)
    {
        std::cout << i << ":" << m[i] << "\n";
    }

    std::cout << "\n\n" << "trj size: " << trj.size() << "\n";

    for(int i = 0; i < trj.size(); i++)
    {
        std::cout << i << ": " << trj[i].step << " " << trj[i].natoms << " " << trj[i].time << "ps " << trj[i].box.x << " " << trj[i].box.y << " " << trj[i].box.z << "\n";

        for(int n = 0; n < 3; n++)
        {
            std::cout << "  " << n << ": " << trj[i].r[n].x << " " << trj[i].r[n].y << " " << trj[i].r[n].z << " ## "
            << trj[i].v[n].x << " " << trj[i].v[n].y << " " << trj[i].v[n].z << " ## "
            << trj[i].f[n].x << " " << trj[i].f[n].y << " " << trj[i].f[n].z << "\n";
        }
        for(int n = trj[i].r.size()-3; n < trj[i].r.size(); n++)
        {
            std::cout << "  " << n << ": " << trj[i].r[n].x << " " << trj[i].r[n].y << " " << trj[i].r[n].z << " ## "
            << trj[i].v[n].x << " " << trj[i].v[n].y << " " << trj[i].v[n].z << " ## "
            << trj[i].f[n].x << " " << trj[i].f[n].y << " " << trj[i].f[n].z << "\n";
        }
    }

    return 0;
}
