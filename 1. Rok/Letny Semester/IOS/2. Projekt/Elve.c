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
    //Get paramters
    (void)argc;
    int elfID = atoi(argv[1]);
    int TE = atoi(argv[2]);

    //Connect shared memory-------------------------------------------------------
    key_t SHM_KEY = SHMEM_KEY;
    int SHM_FLAG = IPC_CREAT | S_IWUSR | S_IRUSR;

    int SHM_ID=shmget(SHM_KEY, SHMEM_SIZE, SHM_FLAG);
    if(SHM_ID == -1){
        fprintf(stderr, "ERROR: Elve failed to connect to shared memory!\n");
        exit(1);
    }

    DATA = (shared_data *) shmat( SHM_ID, NULL, 0);
    if(DATA == (shared_data *) -1){
        fprintf(stderr, "ERROR: Elve failed to atttach shared memory block!\n");
        exit(1);
    }
    //----------------------------------------------------------------------------

    //Generate random work time <0,TE>
    srand(time(NULL) ^ getpid());
    int work_time = (rand() % TE + 1);
    
    sem_wait(&DATA->Print);
    printf("%d: A: Elf %d: started\n",DATA->PrintCNT, elfID);
    fflush(stdout);
    DATA->PrintCNT++;
    sem_post(&DATA->Print);

    //work.....
    usleep(work_time);
    //.........

    sem_wait(&DATA->Print);
    printf("%d: A: Elf %d: need help\n",DATA->PrintCNT, elfID);
    fflush(stdout);
    DATA->PrintCNT++;
    sem_post(&DATA->Print);

    sem_wait(&DATA -> elfSem);
    sem_wait(&DATA -> Elf_wait_mutex);
    DATA -> Elf_Wait_CNT++;
    sem_post(&DATA -> Elf_wait_mutex);

    while(DATA -> Elf_Wait_CNT != 3){
        //Wait until there are 3 elves waiting
        if(DATA -> workshop_STE == 1){
            //Workshop is closed
            sem_post(&DATA -> elfSem);
            DATA -> Elf_Wait_CNT--;
            sem_wait(&DATA->Print);
            printf("%d: A: Elf %d: taking holidays\n",DATA->PrintCNT, elfID);
            fflush(stdout);
            DATA->PrintCNT++;
            sem_post(&DATA->Print);
            exit(0);
        }
    }

    if(DATA -> workshop_STE == 0){
        //Workshop is open
        //3 elves enter, wake up Santa and get help
        sem_wait(&DATA -> ElfHelp);
        DATA -> Elves_Get_Help++;
        if(DATA -> lock == 0){
            sem_post(&DATA -> Santa_sleep);
            DATA -> lock = 1;
        }
        sem_wait(&DATA->Print);
        printf("%d: A: Elf %d: get help\n", DATA->PrintCNT, elfID);
        fflush(stdout);
        DATA->PrintCNT++;
        sem_post(&DATA->Print);
        sem_post(&DATA -> ElfHelp);
        if(DATA -> Elves_Get_Help == 3){
            sem_post(&DATA -> Santa_helping);
            DATA -> Elf_Wait_CNT -= 3;
            DATA -> lock = 0;
            DATA -> Elves_Get_Help = 0;
            sem_post(&DATA -> elfSem);
            sem_post(&DATA -> elfSem);
            sem_post(&DATA -> elfSem);
            exit(0);
        }   
        exit(0);
    } 
}