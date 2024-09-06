#!/gnuplot
set samples 1000

# Write to PNG
set terminal pngcairo fontscale 3 linewidth 3 font 'Verdana' size 1600,1200
set output 'output.png'

# Legend
set key box opaque top left maxrows 3 font 'Verdana,10' width -0.5 height 0.5
# unset key # Switch legend off

# Automatic legend titles taken from the data file
# set key autotitle columnhead
# set datafile separator '\t'

# Major ticks, grid, and border
set border linewidth 1.25
set tics scale 0.75
set style line 101 lc rgb 'grey' lt 1 lw 0.5
set grid xtics ytics ls 101

# Minor ticks
set mxtics
set mytics
set my2tics # For 2nd y-axis

# Title and axis labels
# set title 'Plot title' font 'Verdana,14'
set xlabel 'time (ps)'  # '{/:Italic x} ({/Symbol m}m)'
set ylabel 'Value vs time' # '{/:Italic y}'

# Log scale
# set logscale x 10
# set format x '10^{%T}'

# Ranges
# set xrange [1e-3:1e-1]
# set yrange [0:1]

# Second y-axis
# set ytics nomirror autofreq
# set y2tics nomirror autofreq tc lt 4
# set y2label 'Error' tc lt 4
# set y2range [0:2]
# set arrow nohead front from graph 1,1 to graph 1,0 lt 4 lw 1.5

# Color codes
color_blue = '#0060ad'
color_red = '#dd181f'
color_grey = '#777777'

# 6-color palette (from blue to red)
# color_dark_blue = '#073b4c'
# color_light_blue = '#118ab2'
# color_turquoise = '#06d6a0'
# color_yellow = '#ffd166'
# color_orange = '#f78c6b'
# color_light_red = '#ef476f'

# Line styles
set style line 1 lt 1 dashtype  1  lw 1.5 pointtype 7 pointsize 2.5 lc rgb color_blue
set style line 2 lt 2 dashtype '-' lw 2 pointtype 5 pointsize 2.5 lc rgb color_red
set style line 3 lt 4 dashtype '.' lw 2 pointtype 2 pointsize 2.5

# Labels
# set label '±5%' textcolor rgb color_grey font 'Verdana,10' at 0.0012,0.5 front
# set label '{/:Italic x} = 3·10^{-2}' textcolor lt 7 font 'Verdana,10' at 0.0095,0.76 front right

# Arrows
# set arrow head front from 0.01,0.75 to 0.028,0.67 lt 7 lw 1.5

# Functions
# f(x) = 3*GPVAL_pi*x - 1e-3

# Fitting
# f(x) = a*x + b
# fit f(x) 'data.dat' u 1:3 via a,b

# Data file names
file_name = 'output_3.txt'

# Plotting
plot \
    file_name u 1:3 w lines lw 1.5 title 'Density [a.m.u./nm^3]', \

    # '+' using ($1):(0.45):(0.55) lc rgb '#dddddd' notitle with filledcurves, \
    # file_name u 1:2 w points ls 1 title '{/:Italic a} = 1', \
    # file_name u 1:($3*1.2) w linespoints ls 2, \
    # f(x) w lines ls 3 notitle axis x1y2


# EXTRAS:

# Join two files together and use smoothing:
# plot "< join a.dat b.dat" u 2:3 smooth bezier w l

# Histogram
# plot file_name u 1:2 w boxes lc rgb color_blue

# List of built-in colors:
# gnuplot -e "show colornames"

# lt:
# 0 - black dots
# 1 - purple line (default)
# 2 - green line
# 3 - light blue
# 4 - orange
# 5 - yellow
# 6 - blue
# 7 - red
# 8 - black
