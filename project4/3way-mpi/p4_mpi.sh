#!/bin/bash

for i in 1 2 4 8 16 
do 
	sbatch --constraint=elves --time=06:00:00 --ntasks-per-node=$i --nodes=1 --mem-per-cpu=$((16/$i))G -J SGTYES10K p4_runmpi.sh
done
