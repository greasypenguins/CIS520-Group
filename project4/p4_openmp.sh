#!/bin/bash

#SBATCH --mem-per-cpu=2G

## Max time 1 minute
#SBATCH --time=0:01:00

## Use 1 node
## If commented: Use any number of nodes
##SBATCH --nodes=1

## Use 1 core total (across all nodes)
#SBATCH --ntasks=1

#SBATCH --constraint=elves

## Job title
#SBATCH -J SGTYES

## Send email for all updates
#SBATCH --mail-type=ALL

srun p4_omp




## A Sample sbatch script created by Kyle Hutson
##
## Note: Usually a '#" at the beginning of the line is ignored. However, in
## the case of sbatch, lines beginning with #SBATCH are commands for sbatch
## itself, so I have taken the convention here of starting *every* line with a
## '#', just Delete the first one if you want to use that line, and then modify
## it to your own purposes. The only exception here is the first line, which
## *must* be #!/bin/bash (or another valid shell).

## There is one strict rule for guaranteeing Slurm reads all of your options:
## Do not put *any* lines above your resource requests that aren't either:
##    1) blank. (no other characters)
##    2) comments (lines must begin with '#')

## Specify the amount of RAM needed _per_core_. Default is 1G
##SBATCH --mem-per-cpu=1G

## Specify the maximum runtime in DD-HH:MM:SS form. Default is 1 hour (1:00:00)
##SBATCH --time=1:00:00

## Require the use of infiniband. If you don't know what this is, you probably
## don't need it.
##SBATCH --gres=fabric:ib:1

## GPU directive. If You don't know what this is, you probably don't need it
##SBATCH --gres:gpu:1

## number of cores/nodes:
## quick note here. Jobs requesting 16 or fewer cores tend to get scheduled
## fairly quickly. If you need a job that requires more than that, you might
## benefit from emailing us at beocat@cs.ksu.edu to see how we can assist in
## getting your job scheduled in a reasonable amount of time. Default is
##SBATCH --cpus-per-task=1
##SBATCH --cpus-per-task=12
##SBATCH --nodes=2 --tasks-per-node=1
##SBATCH --tasks=20

## Constraints for this job. Maybe you need to run on the elves
##SBATCH --constraint=elves
## or perhaps you just need avx processor extensions
##SBATCH --constraint=avx

## Output file name. Default is slurm-%j.out where %j is the job id.
##SBATCH --output=MyJobTitle.o%j

## Split the errors into a seperate file. Default is the same as output
##SBATCH --error=MyJobTitle.e%j

## Name my job, to make it easier to find in the queue
##SBATCH -J MyJobTitle

## Send email when certain criteria are met.
## Valid type values are NONE, BEGIN, END, FAIL, REQUEUE, ALL (equivalent to
## BEGIN, END, FAIL, REQUEUE,  and  STAGE_OUT),  STAGE_OUT  (burst buffer stage
## out and teardown completed), TIME_LIMIT, TIME_LIMIT_90 (reached 90 percent
## of time limit), TIME_LIMIT_80 (reached 80 percent of time limit),
## TIME_LIMIT_50 (reached 50 percent of time limit) and ARRAY_TASKS (send
## emails for each array task). Multiple type values may be specified in a
## comma separated list. Unless the  ARRAY_TASKS  option  is specified, mail
## notifications on job BEGIN, END and FAIL apply to a job array as a whole
## rather than generating individual email messages for each task in the job
## array.
##SBATCH --mail-type=ALL

## Email address to send the email to based on the above line.
## Default is to send the mail to the e-mail address entered on the account
## request form.
##SBATCH --mail-user myemail@ksu.edu

## And finally, we run the job we came here to do.
## $HOME/ProgramDir/ProgramName ProgramArguments

## OR, for the case of MPI-capable jobs
## mpirun $HOME/path/MpiJobName