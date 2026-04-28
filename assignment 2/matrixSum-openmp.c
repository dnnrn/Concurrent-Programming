/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c 
     ./matrixSum-openmp size numWorkers

*/

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>


double start_time, end_time;

#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 8   /* maximum number of workers */

int numWorkers;
int size; 
int matrix[MAXSIZE][MAXSIZE];
void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j, total=0;
  int minValue, maxValue;
  int maxX, maxY, minX, minY;

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  omp_set_num_threads(numWorkers);

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
    //  printf("[ ");
	  for (j = 0; j < size; j++) {
      matrix[i][j] = rand()%99;
      //	  printf(" %d", matrix[i][j]);
	  }
	  	//  printf(" ]\n");
  }


 maxValue = matrix[0][0];
 minValue = matrix[0][0];
 maxX = 0; maxY = 0;
 minX = 0; minY = 0;

  start_time = omp_get_wtime();

#pragma omp parallel 
{
  int workerMin, workerMax;
  int workerMinX, workerMinY, workerMaxX, workerMaxY;
  int workerTotal = 0;

  workerMin = matrix[0][0]; workerMax = matrix[0][0];
  workerMinX = 0;  workerMinY = 0; workerMaxX = 0; workerMaxY = 0;
 

    #pragma omp for nowait

  for (i = 0; i < size; i++){
    for (j = 0; j < size; j++){
       workerTotal += matrix[i][j];

        if(matrix[i][j] < workerMin){
        workerMin = matrix[i][j];
        workerMinX = i;
        workerMinY = j;
        }

      if(matrix[i][j] > workerMax){
        workerMax = matrix[i][j];
        workerMaxX = i;
        workerMaxY = j;
      }
    }
  }

      #pragma omp critical 
      {

         total += workerTotal;

        if(workerMin < minValue){
        minValue = workerMin;
        minX = workerMinX;
        minY = workerMinY;
        }

      if(workerMax > maxValue){
        maxValue = workerMax;
        maxX = workerMaxX;
        maxY = workerMaxY;
        }
      }
   }
// implicit barrier

  end_time = omp_get_wtime();

  printf("the total is %d\n", total);
  printf("it took %g seconds\n", end_time - start_time);
  printf("Minimum value is %d at index[%d][%d]\n", minValue, minX, minY);
  printf("Maximum value is %d at index[%d][%d]\n", maxValue, maxX, maxY);

return 0;
}

