import numpy as np

floats = np.fromfile("output_3.dat", dtype=np.float32)

i = 0

for val in floats:
    i += 1
    print(val, end=' ')
    if i == 1:
        print(":")
        continue
    if i % 27 == 0:
        print()
    if i == 1000:
        break
