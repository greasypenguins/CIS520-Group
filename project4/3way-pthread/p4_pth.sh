#!/bin/bash

for i in 1 2 4 8 16 
do
	sbatch --constraint=elves --time=04:00:00 --cpus-per-task=$i --nodes=1 --mem-per-cpu=$((16/$i))G -J SGTYES10K p4_runpth.sh
done
