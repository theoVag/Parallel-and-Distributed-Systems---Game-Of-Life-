#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "omp.h"
#include "mpi.h"
#include <game-of-life.h>
#define num_th 8
void play (unsigned char *board, unsigned char *newboard, int N,int M) {
  /*
    (copied this from some web page, hence the English spellings...)

    1.STASIS : If, for a given cell, the number of on neighbours is 
    exactly two, the cell maintains its status quo into the next 
    generation. If the cell is on, it stays on, if it is off, it stays off.

    2.GROWTH : If the number of on neighbours is exactly three, the cell 
    will be on in the next generation. This is regardless of the cell's
    current state.

    3.DEATH : If the number of on neighbours is 0, 1, 4-8, the cell will 
    be off in the next generation.
  */
  int   i, j, a;
  int pid,nproc,prev,next;//pid=proccess unique id, nproc=number of proccesses(proccesses-1)
  /* for each cell, apply the rules of Life */
  unsigned char *buff1,*buff2,*buff3,*buff4;
  buff1=(unsigned char *)malloc(M*sizeof(unsigned char));//allocate memory for buffers 
  buff2=(unsigned char *)malloc(M*sizeof(unsigned char));//each buffer contains a "row"
  buff3=(unsigned char *)malloc(M*sizeof(unsigned char));
  buff4=(unsigned char *)malloc(M*sizeof(unsigned char));
  
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);//save proccess id(starting from 0) to pid
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);//save number of proccesses to nproc
  MPI_Status status[4];//array of status to synchronize communication
  MPI_Request requests[4];//array of requests to synchronize communication
  prev=pid-1;//variable points to previous proccess(previous part of board)
  next=pid+1;//variable points to next proccess(next part of board)
  if(pid==0) prev=nproc-1;//if pid==0 then previous part if the last one
  if(pid ==(nproc-1)) next=0;//if pid==nproc-1 then next part is the first one
  
  MPI_Irecv(buff3, M, MPI_UNSIGNED_CHAR, prev, 1,MPI_COMM_WORLD, &requests[0]);// receive last row of previous part
  MPI_Irecv(buff4, M, MPI_UNSIGNED_CHAR, next,2,MPI_COMM_WORLD, &requests[1]);//  receive first row of next part
  
  buff1=&Board(0,0);//buffer to send first row of current part
  buff2=&Board(N-1,0);	//buffer to send last row of current part
  MPI_Isend(buff1, M,MPI_UNSIGNED_CHAR, prev, 2, MPI_COMM_WORLD,&requests[2]);// send first row of current part
  MPI_Isend(buff2, M,MPI_UNSIGNED_CHAR, next, 1, MPI_COMM_WORLD,&requests[3]);// send last row of current part
		
//calculations until communications complete
   #pragma omp parallel for private(i,j,a) shared(board,newboard) num_threads(num_th)//parallel for with 8 threads with private variable for each thread i,j,a and shared(board,newboard)
  for (i=1; i<N-1; i++){//calculations in board without first and last row
		for (j=0; j<M; j++) {
		  a = adjacent_to (board, i, j, N,M);
		  if (a == 2) NewBoard(i,j) = Board(i,j);
		  if (a == 3) NewBoard(i,j) = 1;
		  if (a < 2) NewBoard(i,j) = 0;
		  if (a > 3) NewBoard(i,j) = 0;
		}
	}


  unsigned char *temp,*temp2;//temporary array 3xM to calculate boundary conditions
  temp=(unsigned char *)malloc(3*M*sizeof(unsigned char));//array includes last row of previous part , first and second row of current part(of board)
  temp2=(unsigned char *)malloc(3*M*sizeof(unsigned char));//array includes N-2 and N-1 row of current part and first row of next part(of board)
  int p;
    #pragma omp parallel for private(p) shared(temp,board) num_threads(num_th)
	for(p=0;p<M;p++){		
		temp2[0*M+p]=Board(N-2,p);//temp2 first row=board N-2 row
	}
	#pragma omp parallel for private(p) shared(temp,board) num_threads(num_th)
	for(p=0;p<M;p++){
		temp[1*M+p]=Board(0,p);//temp second row= board first row
		temp2[1*M+p]=Board(N-1,p);//temp2 second row=board last row
	}
	#pragma omp parallel for private(p) shared(temp,board) num_threads(num_th)
	for(p=0;p<M;p++){
		temp[2*M+p]=Board(1,p);//temp third row=board second row
	}
	
	 #pragma omp parallel for private(i,j,a) shared(board,newboard) num_threads(num_th)
	for (i=1; i<N-1; i++){//copy newboard to board

		for (j=0; j<M; j++) {
		  Board(i,j) = NewBoard(i,j);
		}
	}
	MPI_Waitall(4, requests, status);//wait all send/receive to complete
  

  /*for(p=0;p<M;p++){
		temp[p+0*M]=buff3[p];
		temp[p+1*M]=Board(0,p);
		temp[p+2*M]=Board(1,p);
		temp2[p+0*M]=Board(N-2,p);
		temp2[p+1*M]=Board(N-1,p);
		temp2[p+2*M]=buff4[p];
	}*/
	#pragma omp parallel for private(p) shared(temp) num_threads(num_th)//egine allagh
	for(p=0;p<M;p++){
		temp[0*M+p]=buff3[p];		//temp first row=received row
	}
	#pragma omp parallel for private(p) shared(temp) num_threads(num_th)//egine allagh
	for(p=0;p<M;p++){
		temp2[2*M+p]=buff4[p];//temp2 third row=received row
	}
	
	#pragma omp parallel for private(j,a) shared(board,newboard) num_threads(num_th)//egine allagh
  for (j=0; j<M; j++) {//calculate first row of current part by sending to function the three rows needed for calculations
      a = adjacent_to (temp, 0, j, 3,M);
      if (a == 2) NewBoard(0,j) = Board(0,j);
      if (a == 3) NewBoard(0,j) = 1;
      if (a < 2) NewBoard(0,j) = 0;
      if (a > 3) NewBoard(0,j) = 0;
    }
    #pragma omp parallel for private(j,a) shared(board,newboard) num_threads(num_th)//egine allagh
	for (j=0; j<M; j++) {//calculate last row of current part by sending to function the three rows needed for calculations
      a = adjacent_to (temp2, N-1, j, 3,M);
      if (a == 2) NewBoard(N-1,j) = Board(N-1,j);
      if (a == 3) NewBoard(N-1,j) = 1;
      if (a < 2) NewBoard(N-1,j) = 0;
      if (a > 3) NewBoard(N-1,j) = 0;
    }
    

  #pragma omp parallel for private(j) shared(board,newboard) num_threads(num_th)
    for (j=0; j<M; j++) {//copy newboard to board
      Board(0,j) = NewBoard(0,j);
      Board(N-1,j) = NewBoard(N-1,j);
    }

}
