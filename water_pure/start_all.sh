#!/bin/bash

# Tells bash to stop the script if there is an error
set -e

# Number of threads for mdrun
threads=8

# Modified Gromacs path
gmx_path="/home/ik3g18/workspace/gromacs-2023.1/build"

# Array of box sizes (nm)
boxes=("7" "10" "15")
# boxes=("7" "10")

# Array of water model types (e.g., tip3p)
# models=("spce" "tip3p")
models=("spce")

# Force field (e.g., oplsaa)
forcefield="charmm27"

# Environment variables for Gromacs
export GMXBIN="${gmx_path}/bin"
export GMXDATA="${gmx_path}/share/gromacs"
export GMXLDLIB="${gmx_path}/lib"
export GMXMAN="${gmx_path}/share/man"

# Gromacs executables
gmx="${gmx_path}/bin/gmx"
mdrun="$gmx mdrun -nt $threads"

# Color variables
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RESET='\033[0m' # Reset to default color

# Build reader
echo -e "${YELLOW}\nBuilding reader...${RESET}"
./build_reader.sh

# Clean up
echo -e "${YELLOW}\nCleaning up...${RESET}"
if [ -n "$(find . -maxdepth 1 -name "*.out" -print -quit)" ]; then
    rm -v *.out
fi
if [ -n "$(find . -maxdepth 1 -name "traj.*" -print -quit)" ]; then
    rm -v traj.*
fi
if [ -n "$(find . -maxdepth 1 -name "output_*.dat" -print -quit)" ]; then
    rm -v output_*.dat
fi

# Loop over water model types
for model in "${models[@]}"; do

    # Loop over the box sizes
    for L in "${boxes[@]}"; do

        # Configuration and output file names
        top="${model}_topol_${L}.top"
        tpr="${model}_topol_${L}.tpr"
        conf="${model}_conf_${L}.gro"
        ener="${model}_ener_${L}.edr"
        traj="${model}_traj_${L}.xtc"
        traj_mol="${model}_traj_mol_${L}.xtc"
        trr="${model}_traj_${L}.trr"
        mdp="${model}_prd_${L}.mdp"
        log="${model}_md_${L}.log"

        # Prints box size and the model type
        echo -e "${YELLOW}\n======== Model: ${model^^} :: box size = $L x $L x $L nm ========${RESET}"

        # Start driver
        echo -e "${YELLOW}\nStarting driver...${RESET}"
        ./driver.sh ${model^^} $L &
        pid=$!
        echo -e "${YELLOW}The driver has PID: $pid${RESET}"

        # Generate initial topology
        ################################################################################
        echo -e "${GREEN}\n==== Generate initial ${model^^} topology ====\n${RESET}"

        echo -e "#include \"${forcefield}.ff/forcefield.itp\"" > $top
        echo -e "#include \"${forcefield}.ff/${model}.itp\"" >> $top
        echo -e "\n[ system ]" >> $top
        echo -e "${model^^} water :: $L x $L x $L nm" >> $top
        echo -e "\n[ molecules ]" >> $top

        echo "...done."

        # Solvate (add H2O) into box LxLxL
        ################################################################################
        echo -e "${GREEN}\n==== Solvate with ${model^^} water (add H2O) ====\n${RESET}"

        $gmx solvate -cs spc216 -o $conf -p $top -box $L $L $L

        # Minimisation
        ################################################################################
        echo -e "${GREEN}\n==== Minimisation ====\n${RESET}"

        $gmx grompp -f mdp/min.mdp -c $conf -p $top -o $tpr -po $mdp
        $mdrun -s $tpr -o $trr -x $traj -c $conf -e $ener -g min_$log

        # Equilibration (NPT)
        ################################################################################
        echo -e "${GREEN}\n==== Equilibration (NPT) ====\n${RESET}"

        $gmx grompp -f mdp/eql.mdp -c $conf -p $top -o $tpr -po $mdp
        $mdrun -s $tpr -o $trr -x $traj -c $conf -e $ener -g eql_$log -v

        # Production
        ################################################################################
        echo -e "${GREEN}\n==== Production ====\n${RESET}"

        $gmx grompp -f mdp/prd.mdp -c $conf -p $top -o $tpr -po $mdp
        $mdrun -s $tpr -o $trr -x $traj -c $conf -e $ener -g $log -v

        # Post-processing
        ################################################################################
        echo -e "${GREEN}\n==== Post-processing ====\n${RESET}"

        # Update molecules for VMD
        # echo 0 | $gmx trjconv -f $traj -s $tpr -pbc mol -o $traj_mol

        # Temperature, Density, Pressure, Volume
        # (echo 14
        # echo 22
        # echo 16
        # echo 21) | $gmx energy -f $ener -o ${ener}.xvg

        # Replace symbols @ with # for gnuplot
        # sed -i 's/@/#/g' ${ener}.xvg

        echo "...done."

    done
done

# Clean up
echo -e "\nClean up..."
if [ -n "$(find . -maxdepth 1 -name "step*.pdb" -print -quit)" ]; then
    rm step*.pdb
fi
rm \#*.*\#
rm *.cpt

# End
echo -e "${YELLOW}\nAll done.\n${RESET}"
