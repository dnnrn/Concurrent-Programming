#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_BEES 16
#define MAX_HONEY 64

sem_t full, empty;
int honeyPot = 0;
int honeyPotFull;

void *bee(void *arg) {
    int id = *(int *) arg;
    while(1) {
        sem_wait(&empty); //wait for permisission to add honey
        honeyPot++;
        printf("Bee number #%d added 1 honey to honeypot: %d/%d\n", id, honeyPot, honeyPotFull);
      
        if (honeyPot == honeyPotFull) {
            printf("Bee number %d woke up the bear.\n", id);
            sem_post(&full); //signal the bear
           
        } else {
            sem_post(&empty); //let bees continue filling
            
        }
        sleep(1);
    }
    return NULL;
}

void *bear() {
    while(1) {
        sem_wait(&full); //bear waits until pot is full

        printf("Honeypot fill: %d\n", honeyPot);
        printf("The bear emptied the honeypot\n");
        honeyPot = 0; //reset honeypot

        sleep(2);
        sem_post(&empty); //let bees refill the pot
    }
     return NULL;
}

int main(int argc, char *argv[]) {
    int numBees, id;

    numBees = (argc > 1) ? atoi(argv[1]) : MAX_BEES;
    if (numBees > MAX_BEES) numBees = MAX_BEES;

    honeyPotFull = (argc > 2) ? atoi(argv[2]) : MAX_HONEY;
    if (honeyPotFull > MAX_HONEY) honeyPotFull = MAX_HONEY;

    pthread_t bearThread, bees[numBees];
    int beeIDs[numBees];

    sem_init(&full, 0, 0);
    sem_init(&empty, 0, 1);

    for (id = 0; id < numBees; id++) {
        beeIDs[id] = id;
        pthread_create(&bees[id], NULL, bee, &beeIDs[id]);  
    }

    pthread_create(&bearThread, NULL, bear, NULL);

    for(id = 0; id < numBees; id++) {
        pthread_join(bees[id], NULL);
    }
    pthread_join(bearThread, NULL);

    return 0;
}