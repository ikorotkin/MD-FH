import numpy as np
from dataclasses import dataclass


@dataclass
class MD_Data:
    """
    Data structure that stores all cell-averaged values in the given Control Volume (i,j,k) vs time.
    """

    i: int  # i-coordinate of the cell
    j: int  # j-coordinate of the cell
    k: int  # k-coordinate of the cell

    time: np.float32  # Time in ps

    cell_volume: np.float32  # Cell volume [nm^3]

    dens: np.float32  # Averaged density [a.m.u. / nm^3]
    mom_x: np.float32  # Averaged X-momentum per unit volume [a.m.u. / (ps nm^2)]
    mom_y: np.float32  # Averaged Y-momentum per unit volume [a.m.u. / (ps nm^2)]
    mom_z: np.float32  # Averaged Z-momentum per unit volume [a.m.u. / (ps nm^2)]
    vv_xx: np.float32  # Velocity tensor (XX-component) [nm^2 / ps^2]
    vv_yy: np.float32  # Velocity tensor (YY-component) [nm^2 / ps^2]
    vv_zz: np.float32  # Velocity tensor (ZZ-component) [nm^2 / ps^2]
    vv_xy: np.float32  # Velocity tensor (XY-component) [nm^2 / ps^2]
    vv_xz: np.float32  # Velocity tensor (XZ-component) [nm^2 / ps^2]
    vv_yz: np.float32  # Velocity tensor (YZ-component) [nm^2 / ps^2]


def generate_MD_data_list(floats: list[np.float32], grid: int) -> list[MD_Data]:
    """
    Reads the data from array of floats.

    Args:
        floats (list[np.float32]): Array of floats from the binary file.
        grid (int): Number of Control Volumes along each axis in the file.

    Returns:
        list[MD_Data]: Array of MD_Data class objects.
    """

    # Create an empty list of MD_Data objects
    data_arr = []

    ind = 0

    # Loop over all the floats in the array
    while ind < len(floats):

        # Time [ps]
        time = floats[ind]

        # Cell volume [nm^3]
        cell_volume = floats[ind + 1]

        ind += 2

        # Loops over all Control Volumes
        for k in range(grid):
            for j in range(grid):
                for i in range(grid):

                    # Fetch the cell-averaged MD data
                    dens = floats[ind]
                    mom_x = floats[ind + 1]
                    mom_y = floats[ind + 2]
                    mom_z = floats[ind + 3]
                    vv_xx = floats[ind + 4]
                    vv_yy = floats[ind + 5]
                    vv_zz = floats[ind + 6]
                    vv_xy = floats[ind + 7]
                    vv_xz = floats[ind + 8]
                    vv_yz = floats[ind + 9]

                    ind += 10

                    # Create MD_Data object and append it to the array
                    data = MD_Data(
                        i,
                        j,
                        k,
                        time,
                        cell_volume,
                        dens,
                        mom_x,
                        mom_y,
                        mom_z,
                        vv_xx,
                        vv_yy,
                        vv_zz,
                        vv_xy,
                        vv_xz,
                        vv_yz,
                    )
                    data_arr.append(data)

    return data_arr


def read_MD_binary(filename: str, grid: int) -> list[MD_Data]:
    """
    Reads cell-averaged MD data from the binary file.

    Args:
        filename (str): File name of the binary file to read.
        grid (int): Number of Control Volumes along each axis in the file.
                    For example, if the grid is 5x5x5 cells, then grid = 5.

    Returns:
        list[MD_Data]: Array of MD_Data class objects.
    """

    # Reads the data from the binary file
    floats = np.fromfile(filename, dtype=np.float32)

    return generate_MD_data_list(floats, grid)


"""
Example.

Reads "output_3.dat" binary file and prints all data related to the cell (1,1,1).

To plot, for example, density vs time:

python3 read_traj.py > output_3.txt
gnuplot read_traj.plt

The plot will be saved to output.png.
"""
if __name__ == "__main__":

    # Read the data from file
    data_array = read_MD_binary("output_3.dat", grid=3)

    # Print data for the cell (i0, j0, k0)
    i0 = 1
    j0 = 1
    k0 = 1

    # Loop over the data set
    for data in data_array:
        if data.i == i0 and data.j == j0 and data.k == k0:
            print(
                data.time,
                data.cell_volume,
                data.dens,
                data.mom_x,
                data.mom_y,
                data.mom_z,
                data.vv_xx,
                data.vv_yy,
                data.vv_zz,
                data.vv_xy,
                data.vv_xz,
                data.vv_yz,
            )
