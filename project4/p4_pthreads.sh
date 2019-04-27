#!/bin/bash
#SBATCH --mem-per-cpu=2G
#SBATCH --time = 0:01:00
#SBATCH --nodes=1
#SBATCH --cpus-per-task=4
#SBATCH --constraint=elves
#SBATCH --output=p4_pth.out
#SBATCH -J SGTYES
srun p4_pth
