#!/bin/bash

# Tells bash to stop the script if there is an error
set -e

# Number of output files to wait (excluding the initial traj.000000.out file):
FILES=5

# Control Volumes (grids)
grids=("2" "3" "5" "7" "10" "15" "22")

# Path where the output files should be moved
EXTRENAL_PATH="test_output"

path_to_move=${EXTRENAL_PATH}/$1_BOX_$2

mkdir -p ${path_to_move}

# Color variables
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RESET='\033[0m' # Reset to default color

echo -e "${RED}DRIVER: Waiting for output files from 000000 to $(printf "%06d" "$FILES"):\n${RESET}"

# Loop over all output files (even if they don't exist yet)
for ((i=0; i<=$FILES; i++)); do

    # Convert the number to a string with leading zeros to make it 6 digits
    i_prefixed=$(printf "%06d" "$i")

    # File to wait for
    file_to_read="traj.${i_prefixed}.out"

    echo -e "${RED}Waiting for ${file_to_read} to be created...\n${RESET}"

    # Wait until the file is created
    while [ ! -f "${file_to_read}" ]; do
        sleep 1  # Wait for 1 second before checking again
    done

    # Call the reader
    echo -e "${RED}-- reading $file_to_read...${RESET}"
    ./read_traj.exe ${file_to_read}

    # Process the output files
    echo -e "${RED}-- moving output files to the external drive to ${path_to_move}...${RESET}"
    for grid in "${grids[@]}"; do
        mv output_${grid}.dat ${path_to_move}/output_${grid}.${i_prefixed}.dat
    done

    # Clean up
    echo -e "${RED}-- cleaning up...${RESET}"
    rm -v ${file_to_read}

    # Done
    echo

done

echo -e "${RED}\nDRIVER: Finished.\n${RESET}"
