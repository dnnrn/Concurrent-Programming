/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_cond_t go;        /* condition variable for leaving */
int numWorkers;           /* number of workers */ 
int numArrived = 0;       /* number who have arrived */

/*Global variables*/
double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
int sums[MAXWORKERS]; /* partial sums */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */
int maxValues[MAXWORKERS], minValues[MAXWORKERS];/*min/max values by each worker*/
int max_X[MAXWORKERS], max_Y[MAXWORKERS]; /*Position of the maximum value by each worker thread*/
int min_X[MAXWORKERS], min_Y[MAXWORKERS]; /*Position of the minimum value by each worker thread*/


/* a reusable counter barrier */
void Barrier() {
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numWorkers) {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  } else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
}

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}


void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&barrier, NULL);
  pthread_cond_init(&go, NULL);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%99; 
	  }
  }

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
#endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
 for (l = 0; l < numWorkers; l++)
  pthread_join(workerid[l], NULL);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int total, i, j, first, last;

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

int workerMax = matrix[first][0];/*Min/max found by each worker*/
int workerMin = matrix[first][0];
int workerMax_X = first, workerMax_Y = 0;/*position of the max value*/
int workerMin_X = first, workerMin_Y = 0;/*position of the min value*/
 
  /* sum values in my strip */
  total = 0;
  for (i = first; i <= last; i++){
    for (j = 0; j < size; j++){
      total += matrix[i][j];

    /*Max value in my strip*/
    if(matrix[i][j] > workerMax){
      workerMax = matrix[i][j];
      workerMax_X = i;
      workerMax_Y = j;
      }
    /*min value in my strip*/
    if(matrix[i][j] < workerMin){
      workerMin = matrix[i][j];
      workerMin_X = i;
      workerMin_Y = j;
       }
    }
  }

    /*worker partial results*/
  sums[myid] = total;
  maxValues[myid] = workerMax;
  minValues[myid] = workerMin;
  max_X[myid] = workerMax_X;
  max_Y[myid] = workerMax_Y;
  min_X[myid] = workerMin_X;
  min_Y[myid] = workerMin_Y;

//ensure all workers have finished computations before proceeding
  Barrier();
  if (myid == 0) {
    total = 0;

  // Initialize the final max/min values and their positions using the first workers result
    int finalMax = maxValues[0], finalMin = minValues[0];
    int finalMaxRow = max_X[0], finalMaxCol = max_Y[0];
    int finalMinRow = min_X[0], finalMinCol = min_Y[0];
  
  //compute the results of all workers
    for (i = 0; i < numWorkers; i++){
      total += sums[i];
//update final max and its position
    if (maxValues[i] > finalMax) {
        finalMax = maxValues[i];
        finalMaxRow = max_X[i];
        finalMaxCol = max_Y[i];  
    }
    //update final min and its position
    if (minValues[i] < finalMin) {
        finalMin = minValues[i];
        finalMinRow = min_X[i];
        finalMinCol = min_Y[i];
    }
    }
    /* get end time */
    end_time = read_timer();
    /* print results */
    printf("The total is %d\n", total);
   printf("Maximum value is %d at position [%d][%d]\n", finalMax, finalMaxRow, finalMaxCol);
    printf("Minimum value is %d at position [%d][%d]\n", finalMin, finalMinRow, finalMinCol);
    printf("The execution time is %g sec\n", end_time - start_time);
  }
  return NULL;
} 