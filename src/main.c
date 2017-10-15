/*
 * Game of Life implementation based on
 * http://www.cs.utexas.edu/users/djimenez/utsa/cs1713-3/c/life.txt
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "mpi.h"//mpi library
#include "sys/time.h"
#include <game-of-life.h>

int main (int argc, char *argv[]) {
  int i,nproc,pid,M;
	unsigned char *board, *newboard;//change boards' type to save memory space
	struct timeval startwtime, endwtime,startTimeInit,stopTimeInit,startTimeGen,stopTimeGen,startTimePlay,stopTimePlay;//variables for time measurement
  if (argc< 5 || argc>=7) { // Check if the command line arguments are correct 
    printf("Usage: %s N thres disp\n"
	   "where\n"
	   "  N     : size of table (N x N)\n"
	   "  thres : propability of alive cell\n"
           "  t     : number of generations\n"
	   "  disp  : {1: display output, 0: hide output}\n"
	   "  M : second dimension(blank if array is square) \n"
           , argv[0]);
    return (1);
  }
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);//save number o proccesses to nproc
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);//save proccess id to pid
  MPI_Barrier(MPI_COMM_WORLD);//barrier to synchronize proccesses for time measurement(start)
  if(pid==0) gettimeofday(&startwtime,NULL);//full timer start
  // Input command line arguments
  int N = atoi(argv[1]);        // Array size
  double thres = atof(argv[2]); // Propability of life cell
  int t = atoi(argv[3]);        // Number of generations 
  int disp = atoi(argv[4]);     // Display output?
  if(argc==6){//array NxM
	  
	M = atoi(argv[5]);        // Array size
  }
  else{ // else array is NxN
	M=N;
  }
  N=N/nproc;//each proccess takes a part of array equals to N/proccesses
  printf("Size %dx%d with propability: %0.1f%%\n", N, M, thres*100);

  board = NULL;
  newboard = NULL;
  
  board = (unsigned char *)malloc(N*M*sizeof(unsigned char));//allocate memory for N*M board

  if (board == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }

  /* second pointer for updated result */
  newboard = (unsigned char *)malloc(N*M*sizeof(unsigned char));//allocate memory for N*M newboard

  if (newboard == NULL){
    printf("\nERROR: Memory allocation did not complete successfully!\n");
    return (1);
  }
  if(pid==0) gettimeofday(&startTimeInit,NULL);//time measurement for initializing board in first proccess
  initialize_board (board, N,M);
  printf("Board initialized\n");
  if(pid==0) gettimeofday(&stopTimeInit,NULL);
  if(pid==0) gettimeofday(&startTimeGen,NULL);//time measurement for generating board in first proccess
  generate_table (board, N,M, thres);
  printf("Board generated\n");
  if(pid==0) gettimeofday(&stopTimeGen,NULL);

  /* play game of life 100 times */
  if(pid==0) gettimeofday(&startTimePlay,NULL);//time measurement for play in all generations for first proccess
  for (i=0; i<t; i++) {
    if (disp) display_table (board, N,M);
    play (board, newboard, N,M);    
  }
  if(pid==0) gettimeofday(&stopTimePlay,NULL);

  printf("Game finished after %d generations.\n", t);
  MPI_Barrier(MPI_COMM_WORLD);//barrier to synchronize proccesses for time measurement(stop)
  if(pid==0){
		gettimeofday(&endwtime,NULL);
		double response_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
		
		double Init_time = (double)((stopTimeInit.tv_usec - startTimeInit.tv_usec)
                                /1.0e6 + stopTimeInit.tv_sec - startTimeInit.tv_sec);
		double Gen_time = (double)((stopTimeGen.tv_usec - startTimeGen.tv_usec)
                                /1.0e6 + stopTimeGen.tv_sec - startTimeGen.tv_sec);
		double Play_time = (double)((stopTimePlay.tv_usec - startTimePlay.tv_usec)
                                /1.0e6 + stopTimePlay.tv_sec - startTimePlay.tv_sec);
		printf("\nFull response time=%lf",response_time);
		printf("\nInit response time=%lf",Init_time);
		printf("\nGen response time=%lf",Gen_time);
		printf("\nPlay response time=%lf",Play_time);
	}
		
   MPI_Finalize();// mpi end
   return 0;
}
