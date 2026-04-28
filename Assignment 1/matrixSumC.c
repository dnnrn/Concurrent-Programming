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

pthread_mutex_t lock;  /* mutex lock for the barrier */
int numWorkers;           /* number of workers */ 
double start_time, end_time; /* start and end times */
int size;  /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

int total_sum = 0;
int final_max, final_max_X, final_max_Y;
int final_min, final_min_X, final_min_Y;
int next_row = 0;


void *Worker(void *);

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

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex */
  pthread_mutex_init(&lock, NULL);


  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
 

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] = 1; // rand()%99; 
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

// initializing the final min/max with the first element of the matrix
final_max =matrix[0][0];
final_max_X = 0;
final_max_Y = 0;
final_min = matrix[0][0];
final_min_X = 0;
final_min_Y = 0;

 /* do the parallel work: create the workers */
 start_time = read_timer();
 for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  
 for (l = 0; l < numWorkers; l++)
        pthread_join(workerid[l], NULL);

/* get end time */
 end_time = read_timer();

 /* print results */
 printf("The total is %d\n", total_sum);
 printf("Maximum value is %d at position [%d][%d]\n", final_max, final_max_X, final_max_Y);
 printf("Minimum value is %d at position [%d][%d]\n", final_min, final_min_X, final_min_Y);
 printf("The execution time is %g sec\n", end_time - start_time);

 return 0;

}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg){
  long myid = (long) arg;
  int workerMax, workerMax_X, workerMax_Y;
  int workerMin, workerMin_X, workerMin_Y;
   int worker_sum = 0;
   int row, j;

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif
// loop for workers to continuously pick new tasks 
  while(1){
    pthread_mutex_lock(&lock);// one access to next_row at a time
    if(next_row >= size){ //exit when all task are done
        pthread_mutex_unlock(&lock);
        break;
    }
   // assigns available row to worker and increment counter
  row = next_row; 
  next_row++;
  pthread_mutex_unlock(&lock);// unlocks and let other workers pick their tasks

 // Initialize max and min values for the assigned row
 workerMax = matrix[row][0];
 workerMin = matrix[row][0];
 //initial position of max and min values in this row
 workerMax_X = row, workerMax_Y = 0;
 workerMin_X = row, workerMin_Y = 0;
 
  /* sum row */
    for (j = 0; j < size; j++) {
      worker_sum += matrix[row][j];

    /*Max value in my strip*/
    if(matrix[row][j] > workerMax){
      workerMax = matrix[row][j];
      workerMax_X = row;
      workerMax_Y = j;
      }

    /*min value in my strip*/
    if(matrix[row][j] < workerMin){
      workerMin = matrix[row][j];
      workerMin_X = row;
      workerMin_Y = j;
       }
    }
//the mutex lock ensures only one thread updates the global variables at a time
  pthread_mutex_lock(&lock);
  total_sum += worker_sum;
//update the global maximum value and its position 
    if (workerMax > final_max) {
        final_max = workerMax;
        final_max_X = workerMax_X;
        final_max_Y = workerMax_Y;  
    }
//update the global minimum value and its position 
    if (workerMin < final_min) {
        final_min = workerMin;
        final_min_X = workerMin_X;
        final_min_Y = workerMin_Y;
    }
  // Unlock the mutex to allow other threads to access the shared variables
    pthread_mutex_unlock(&lock);
    }
    return NULL;
  }

  