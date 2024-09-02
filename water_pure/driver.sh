#!/bin/bash

# Number of output files to wait (excluding the initial traj.000000.out file):
FILES=5

# Path where the output files should be moved
EXTRENAL_PATH="."

echo "Waiting for output files from 000000 to $(printf "%06d" "$FILES"):"
echo

# Loop over all output files (even if they don't exist yet)
for ((i=0; i<=$FILES; i++)); do

    # Convert the number to a string with leading zeros to make it 6 digits
    i_prefixed=$(printf "%06d" "$i")

    # File to wait for
    file_to_read="traj.${i_prefixed}.out"

    echo "Waiting for ${file_to_read} to be created..."

    # Wait until the file is created
    while [ ! -f "${file_to_read}" ]; do
        sleep 1  # Wait for 1 second before checking again
    done

    # Call the reader
    echo "-- reading $file_to_read..."
    # ./read_traj.exe ${file_to_read}

    # Process the output files
    echo "-- moving the output files to the external drive..."
    # mv output_3.dat ${EXTRENAL_PATH}/output_3.${i_prefixed}.dat
    # ...

    # Clean up
    echo "-- cleaning up..."
    # rm ${file_to_read}

    # Done
    echo

done
