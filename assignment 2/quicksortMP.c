/* COMPILE USING: gcc -fopenmp -o quicksortMP.exe quicksortMP.c */

#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#define THREADS_MAX 8 //arbitrary, if a single core can handle 1 thread, the reasoning is that this should work well on all modern computers
#define SIZE_MAX 100000
#define ARRAY_SIZE_THRESHOLD 50 //arbitrary, reasoning is that multithreading is costly

void swap(int numberArray[], int i, int j) { //swap values at indexes i and j
        int temp = numberArray[i];
        numberArray[i] = numberArray[j];
        numberArray[j] = temp;
}

int partition(int numberArray[], int left, int right) { //Lomuto partition
    int pivot = numberArray[right];
    int i = left - 1;
    //goes through array until one index before right. continuously swaps numbers larger than pivot until loop stops,
    //then pivot is placed at the right index (the index int i has gotten to - the amount of elements pivot is larger than in the array)
    for (int j = left; j < right; j++) {
        if (numberArray[j] < pivot) {
            i++;
            swap(numberArray, i, j);
        }
    }
    swap(numberArray, i + 1, right);
    return i + 1; //returns the place where the pivot is now in the array
}

void quicksort(int numberArray[], int left, int right) {
    if (left < right) { //has more than one element
        int pivotIndex = partition(numberArray, left, right);

        if ((pivotIndex - left) > ARRAY_SIZE_THRESHOLD) { //starts a new task if we have less than maximum threads, and we have more than 50 elements in new array to sort (arbitrary number chosen)
            #pragma omp task
            quicksort(numberArray, left, pivotIndex - 1); //creates new task for quicksort()
        } else {
            quicksort(numberArray, left, pivotIndex - 1); //does quicksort() without creating a new task
        }

        if ((right - pivotIndex) > ARRAY_SIZE_THRESHOLD) {
            #pragma omp task
            quicksort(numberArray, pivotIndex + 1, right);
        } else {
            quicksort(numberArray, pivotIndex + 1, right);
        }
    }
}

int size, threads_max;

int main(int argc, char *argv[]) {
    srand(omp_get_wtime()); //otherwise random gets the same seed all the time
    double start_time, total_time; //variables to meassure time
    int n = (argc > 1)? atoi(argv[1]) : SIZE_MAX;
    if (n > SIZE_MAX) n = SIZE_MAX;

    threads_max = (argc > 2)? atoi(argv[2]) : THREADS_MAX;
    if (threads_max > THREADS_MAX) threads_max = THREADS_MAX;
    omp_set_num_threads(threads_max); //maximum threads

    //int n = rand()%99001 + 1000; //size of array, between 100 000 and 1000
    //int numberArray[n]; //create array
    int numberArray[n];

    for (int i = 0; i < n; i++) { //put random values at every index in array
        numberArray[i] = rand()%n + 1; //between 100 000 and 1
    }
    /*
    printf("Unsorted array: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", numberArray[i]);
    }
    printf("\n");*/

    start_time = omp_get_wtime();

    #pragma omp parallel //we make sure that it is multithreaded, we need it for when we enter quicksort()
    {
        #pragma omp single nowait //all the threads would be doing the same task unless we add single, and all the threads that are not single would idle if not for nowait
        quicksort(numberArray, 0, n - 1);
    }

    total_time = omp_get_wtime() - start_time;
    
    printf("Sorted array: ");
    for (int i = 0; i < n - 1; i++) {
        //printf("%d ", numberArray[i]);
        if (numberArray[i] > numberArray[i+1]){
           printf("unsorted");
        
           break;
        }
    }
    printf("\n");

    printf("Algorithm time: %f ms, n: %d, threads: %d\n", total_time*1000, n, threads_max); //omp wtime is in seconds, convert to ms

}