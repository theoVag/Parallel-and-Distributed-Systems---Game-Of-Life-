#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "mpi.h"
#include "omp.h"
#include <game-of-life.h>

/* set everthing to zero */

void initialize_board (unsigned char *board, int N,int M) {
  int   i, j;
  
  for (i=0; i<N; i++)
    for (j=0; j<M; j++) 
      Board(i,j) = 0;
}

/* generate random table */

void generate_table (unsigned char *board, int N,int M, float threshold) {

  int   i, j,pid;
  int counter = 0;
MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  srand(time(NULL)*(pid+1));//change seed of rand to use it for a number of different proccesses
for (i=0; i<N; i++) {
  for (j=0; j<M; j++) {

      Board(i,j) = ( (float)rand() / (float)RAND_MAX ) < threshold;
      counter += Board(i,j);
    }
  }
}

