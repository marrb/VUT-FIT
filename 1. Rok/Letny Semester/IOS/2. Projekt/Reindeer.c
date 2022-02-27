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
    //Get parameters
    (void)argc;
    int DeerID = atoi(argv[1]);
    int TR = atoi(argv[2]);
    int NR = atoi(argv[3]);

    //Connect shared memory-------------------------------------------------------
    key_t SHM_KEY = SHMEM_KEY;
    int SHM_FLAG = IPC_CREAT | S_IWUSR | S_IRUSR;

    int SHM_ID=shmget(SHM_KEY, SHMEM_SIZE, SHM_FLAG);
    if(SHM_ID == -1){
        fprintf(stderr, "ERROR: Reindeer failed to connect to shared memory!\n");
        exit(1);
    }

    DATA = (shared_data *) shmat( SHM_ID, NULL, 0);
    if(DATA == (shared_data *) -1){
        fprintf(stderr, "ERROR: Reindeer failed to atttach shared memory block!\n");
        exit(1);
    }
    //----------------------------------------------------------------------------

    //Generate random holiday time <TR/2,TR>
    srand(time(NULL) ^ getpid());
    int holiday_time = (rand() % (TR - (TR/2) + 1) + (TR/2));

    sem_wait(&DATA->Print);
    printf("%d: A: RD %d: started\n",DATA->PrintCNT, DeerID);
    fflush(stdout);
    DATA->PrintCNT++;
    sem_post(&DATA->Print);

    //holiday.....
    usleep(holiday_time);
    //.........

    //Reindeer coming home....
    sem_wait(&DATA->Deer_CNT_Mutex);
    DATA->Deer_CNT++;
    sem_wait(&DATA->Print); 
    printf("%d: A: RD %d: return home\n", DATA->PrintCNT, DeerID);
    fflush(stdout);
    DATA->PrintCNT++;
    sem_post(&DATA->Print);
    sem_post(&DATA->Deer_CNT_Mutex);

    //If last reindeer came he wakes up Santa
    if(DATA->Deer_CNT == NR){
        DATA->All_Deer_Back = 1;
        sem_post(&DATA->Santa_sleep);
    }

    //Santa hitching reindeers
    sem_wait(&DATA->Deer_Hitched);
    sem_wait(&DATA->Print);
    printf("%d: A: RD %d: get hitched\n",DATA->PrintCNT, DeerID);
    fflush(stdout);     
    DATA->PrintCNT++;
    sem_post(&DATA->Print);
    sem_post(&DATA->Santa_hitch_another);

    exit(0);
}