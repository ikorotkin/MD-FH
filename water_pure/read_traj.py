import numpy as np
from dataclasses import dataclass


filename = "output_3.dat"

grid = 3  # Number of Control Volumes along each axis written in the file

# Write data for the cell (i0, j0, k0)
i0 = 1
j0 = 1
k0 = 1


@dataclass
class Data:
    i: int
    j: int
    k: int
    time: float
    dens: float
    mom_x: float
    mom_y: float
    mom_z: float
    vv_xx: float
    vv_yy: float
    vv_zz: float
    vv_xy: float
    vv_xz: float
    vv_yz: float


'''
Reads data
'''
def read_data(data_bin, N) -> Data:
    data_arr = []

    ind = 0

    while(ind < len(data_bin)):
        time = data_bin[ind]
        ind += 1
        for k in range(N):
            for j in range(N):
                for i in range(N):
                    dens = data_bin[ind]
                    mom_x = data_bin[ind + 1]
                    mom_y = data_bin[ind + 2]
                    mom_z = data_bin[ind + 3]
                    vv_xx = data_bin[ind + 4]
                    vv_yy = data_bin[ind + 5]
                    vv_zz = data_bin[ind + 6]
                    vv_xy = data_bin[ind + 7]
                    vv_xz = data_bin[ind + 8]
                    vv_yz = data_bin[ind + 9]

                    ind += 10

                    data = Data(i, j, k, time, dens, mom_x, mom_y, mom_z, vv_xx, vv_yy, vv_zz, vv_xy, vv_xz, vv_yz)
                    data_arr.append(data)

    return data_arr


# Reads the data from the binary file
floats = np.fromfile(filename, dtype=np.float32)

data_array = read_data(floats, grid)

for data in data_array:
    if(data.i == 1 and data.j == 1 and data.k == 1):
        print(data.time, data.dens, data.mom_x, data.mom_y, data.mom_z, data.vv_xx, data.vv_yy, data.vv_zz, data.vv_xy, data.vv_xz, data.vv_yz)
