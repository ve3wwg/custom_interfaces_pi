set title "Diode 1N914 Plot"
set xlabel "Voltage across D1"
set ylabel "Current through D1"
set autoscale
set yrange [0:0.0008]
plot "diode.dat" using 1:2 with lines
