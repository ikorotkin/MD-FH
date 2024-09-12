gmx pdb2gmx -f 2jof.pdb -o conf_2jof.gro -p topol_2jof.top -water spce -ignh
# gmx editconf -f conf_2jof.gro -o conf_2jof_7.gro -box 7 7 7 -c
