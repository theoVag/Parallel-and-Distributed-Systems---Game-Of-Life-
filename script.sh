#PBS -N game-of-life
#PBS -q pdlab
#PBS -j oe
#PBS -l nodes=4:ppn=8
module load mpi/mpich3-x86_64
Date=$(date "+%Y-%m-%d-%H:%M-%S")
cd $PBS_O_WORKDIR
echo "==== Run starts now ======= 'date' "

mpiexec -np $PBS_NUM_NODES -ppn 1 ./bin/game-of-life 80000 0.4 3 0 80000 &> $PBS_JOBNAME-$PBS_NP-$Date.log

echo "==== Run ends now ======= 'date' "

