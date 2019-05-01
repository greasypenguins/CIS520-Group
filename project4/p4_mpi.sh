#!/bin/bash
#SBATCH --mem-per-cpu=2G
#SBATCH --time=0:01:00

#I think this is related to thread number with mpi
##SBATCH --ntasks-per-node=4
#SBATCH --nodes=1
#SBATCH	--cpus-per-task=4
#SBATCH --constraint=elves
#SBATCH --output=p4_mpi.out
#SBATCH -J SGTYES
srun p4_mpi
