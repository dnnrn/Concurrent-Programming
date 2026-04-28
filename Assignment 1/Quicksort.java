package quicksortpackage;

import java.util.concurrent.locks.ReentrantLock;
import java.util.Random;

class Quicksort {
    private static final int THREADS_MAX = 8;  //Make maximum threads 8, final value will not change
    private static int activeThreadsNumber = 0;
    private static ReentrantLock lock = new ReentrantLock(); //a mutex lock to prevent race condition

    public static void quicksort(int[] numberArray, int left, int right) {
        if (left < right) {
            int pivotIndex = partition(numberArray, left, right);

            Thread leftThread = null, rightThread = null;

            lock.lock();
            try {
                if (activeThreadsNumber < THREADS_MAX & pivotIndex - left > 50) { //starts a subthread if we have less than maximum threads, and we have more than 50 elements in new array to sort (arbitrary number chosen)
                    activeThreadsNumber++;
                    leftThread = new Thread(() -> quicksort(numberArray, left, pivotIndex - 1)); //quicksort() is basically the run method of the object through lambda function
                    leftThread.start(); //begins the quicksort algorithm for the partition on the left side of the pivot
                }
            } finally {
                lock.unlock();
            }

            lock.lock();
            try {
                if (activeThreadsNumber < THREADS_MAX & right - pivotIndex > 50) { //starts a subthread if we have less than maximum threads, and we have more than 50 elements in new array to sort (arbitrary number chosen)
                    activeThreadsNumber++;
                    rightThread = new Thread(() -> quicksort(numberArray, pivotIndex + 1, right)); //quicksort() is basically the run method of the object through lambda function
                    rightThread.start(); //begins the quicksort algorithm for the partition on the right side of the pivot
                }
            } finally {
                lock.unlock();
            }

            if (leftThread != null) { //checks if we have a leftThread running
                try {
                    leftThread.join(); //waits until leftThread is done running
                    lowerActiveThreadsNumber();
                } catch (InterruptedException e) {} //We need this catch statement, or else the program cannot run, even though no error appears.
            } else {
                quicksort(numberArray, left, pivotIndex - 1);
            }

            if (rightThread != null) { //checks if we have a rightThread running
                try {
                    rightThread.join(); //waits until rightThread is done running
                    lowerActiveThreadsNumber();
                } catch (InterruptedException e) {} //We need this catch statement, or else the program cannot run, even though no error appears.
            } else {
                quicksort(numberArray, pivotIndex + 1, right);
            }
        }
    }
//reduces the count of active threads after they finish their work
    private static void lowerActiveThreadsNumber() {
        lock.lock(); //shared variable activeThreadsNumber, needs to be mutually exclusive
        try {
            activeThreadsNumber--; //we remove one active thread from the count
        } finally {
            lock.unlock();
        }
    }

    private static int partition(int[] numberArray, int left, int right) { //Lomuto partition
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

    private static void swap(int[] numberArray, int i, int j) { //swap values at indexes i and j
        int temp = numberArray[i];
        numberArray[i] = numberArray[j];
        numberArray[j] = temp;
    }

    public static void main(String[] args) {
        Random rand = new Random();
        int n = rand.nextInt(21) + 10; //n is between 10 000 and 1 000 000, entries in array
        int[] numberArray = new int[n];
        for (int i = 0; i < n; i++) {
            numberArray[i] = 1 + rand.nextInt(1000); //int at i is between 1 and 1 000 000
        }

        System.out.print("Unsorted array: ");
        for (int number : numberArray) {
          System.out.print(number + " ");
      }
        System.out.println();
        
        long timeStart = System.currentTimeMillis(); //We meassure from when we enter the actual quicksort algorithm
        
        quicksort(numberArray, 0, numberArray.length - 1);

        long timeEnd = System.currentTimeMillis(); //We meassure the time when the quicksort algorithm finishes

        System.out.print("Sorted array: ");
        for (int number : numberArray) {
          System.out.print(number + " ");
        }
        System.out.println();

        long totalTime = timeEnd - timeStart;
        System.out.println("Algorithm time: " + totalTime + " ms, n: " + n); //Always below 1s in our set of n's, however printing the array takes much longer.
    }
}