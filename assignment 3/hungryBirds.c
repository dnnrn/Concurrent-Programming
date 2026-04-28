
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


#define NUM_BABY_BIRDS 3 // Number of baby bird threads 
#define MAX_WORMS 10 //initial number of worms
#define MAX_WORMREFILL 10 // refill worms can be max 10 
#define MAX_REFILLS 3 // the parent bird will refill the plate 3 times but the code can be motified to run forever

int worms = MAX_WORMS;
//int counter = 0;
int theSignal = 0;

sem_t plate_lock; // semaphore mutual exclusion for baby bird accessing the plate
sem_t plate_empty; //signalling the parent when plate is empty 
sem_t plate_full; //Parent signaling the baby birds when plate is full again

void *baby_bird(void *arg){
int id = *(int *) arg; // gets baby bird ID 


while(1){
    sem_wait(&plate_lock); //the plate is locked for exclusive access
   
   /*if (counter >= MAX_REFILLS){ //stop after refilling the plate 4 times
    sem_post(&plate_lock);
    break;
   }*/

    if(worms > 0){
        worms--; 
        printf("baby bird %d ate a worm. %d worms left\n", id, worms);
        sem_post(&plate_lock); //release lock
        sleep(1);
    } 
    else{
    if (theSignal ==0){
        printf("Baby bird %d chirping, The plate is empty\n",id);
        sem_post(&plate_empty); //signal the parent bird that the food is out
        theSignal = 1; // setting the signal to 1 so another baby bird wouldn't alert the parent again
    }
        sem_post(&plate_lock); //release lock after waking up
        
        /// if(counter >= MAX_REFILLS) break; 
        sem_wait(&plate_full); //wait for parent to fill the plate
       
    }
  }
  return NULL;
}

void *parent_bird(void *arg){
    while(1){
        sem_wait(&plate_empty); // wait until baby birds signal the plate is empty
        sem_wait(&plate_lock); // locks the plate to refill worms

      /*  if(counter >= MAX_REFILLS){ // if the maximum refills have happened release lock
            sem_post(&plate_lock);
        

       for (int j = 0; j < NUM_BABY_BIRDS; j++) {
                sem_post(&plate_full); // wakes up all baby birds when plate is full
            }
            break;
        } */

        worms = (rand()% (MAX_WORMREFILL - 5)) + 5;
        ///counter++;
        printf("Parent birds gets %d worms.\n", worms);
        sem_post(&plate_lock);  //releases the lock after refilling the plate
        sleep(1);
        
        theSignal = 0; // the signal is reset to zero again

        for (int j = 0; j < NUM_BABY_BIRDS; j++) {
            sem_post(&plate_full);   //wakeup all baby birds
        }
    }
    return NULL;
}

int main(){
int i, bird_id[NUM_BABY_BIRDS];

pthread_t babyBirds[NUM_BABY_BIRDS]; // creates arrays ti store baby bird thread ID
pthread_t parentBird;

sem_init(&plate_lock, 0, 1);   //starts at 1 allows exclusive access
sem_init(&plate_empty, 0, 0); //signal parent bird when food is out
sem_init(&plate_full, 0, 0); //signal baby birds when food is refilled 

for(i = 0; i < NUM_BABY_BIRDS; i++){
    bird_id[i] = i;
    pthread_create(&babyBirds[i], NULL, baby_bird, (void*)&bird_id[i]);

}

pthread_create(&parentBird, NULL, parent_bird, NULL);


for(i = 0; i < NUM_BABY_BIRDS; i++){
    pthread_join(babyBirds[i], NULL);
}

pthread_join(parentBird, NULL);

return 0;
}