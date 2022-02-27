#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>
#include "Shared_mem.h"
#include <sys/shm.h>
#include <sys/stat.h>

shared_data *DATA;

int main(int argc, char *argv[]){
    //Get arguments
    (void)argc;
    int NR = atoi(argv[1]);
    int Deer_Hitched = 0;

    //Connect shared memory-------------------------------------------------------
    key_t SHM_KEY = SHMEM_KEY;
    int SHM_FLAG = IPC_CREAT | S_IWUSR | S_IRUSR;

    int SHM_ID=shmget(SHM_KEY, SHMEM_SIZE, SHM_FLAG);
    if(SHM_ID == -1){
        fprintf(stderr, "ERROR: Santa failed to connect to shared memory!\n");
        exit(1);
    }

    DATA = (shared_data *) shmat( SHM_ID, NULL, 0);
    if(DATA == (shared_data *) -1){
        fprintf(stderr, "ERROR: Santa failed to atttach shared memory block!\n");
        exit(1);
    }
    //----------------------------------------------------------------------------

    sem_wait(&DATA->Print);
    printf("%d: A: Santa: going to sleep\n", DATA->PrintCNT);
    fflush(stdout);
    DATA->PrintCNT++;
    sem_post(&DATA->Print);
    sem_post(&DATA -> Make_Santa);

    while(true){
    //Waiting for Elves or Reindeers to wake up Santa
    sem_wait(&DATA -> Santa_sleep);
        //If all reindeers are back
        if(DATA->All_Deer_Back == 1){
            sem_wait(&DATA->Print);
            printf("%d: A: Santa: closing workshop\n", DATA->PrintCNT);
            fflush(stdout);
            DATA->PrintCNT++;
            sem_post(&DATA->Print);
            DATA->workshop_STE = 1;
            while(Deer_Hitched != NR+1){
                sem_wait(&DATA->Santa_hitch_another);
                sem_post(&DATA->Deer_Hitched);
                Deer_Hitched++;
            }
            sem_wait(&DATA->Print);
            printf("%d: A: Santa: Christmass started\n", DATA->PrintCNT);
            fflush(stdout);
            DATA->PrintCNT++;
            sem_post(&DATA->Print);
            exit(0);
        }
    //If Elves want helps
    sem_wait(&DATA->Print);
    printf("%d: A: Santa: helping elves\n", DATA->PrintCNT);  
    fflush(stdout);
    DATA->PrintCNT++;
    sem_post(&DATA->Print);

    //Santa finished helping elves
    sem_wait(&DATA -> Santa_helping);
    printf("%d: A: Santa: going to sleep\n", DATA->PrintCNT);    
    fflush(stdout);
    DATA->PrintCNT++;
    sem_post(&DATA->Print);
    }
}