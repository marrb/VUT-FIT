#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef struct{
    int Elf_Wait_CNT; //Number of elves waiting in front of the Santa's workshop
    int Elves_Get_Help; //Number of elves getting help
    int Deer_CNT; //Number of deers that returned home from holiday
    int lock;   //Mutex for waking up Santa
    int PrintCNT;
    bool workshop_STE; //1 if workshop closed, 0 if open
    bool All_Deer_Back; //All reindeers came back from holiday
    sem_t Make_Santa; //Makes sure Santa gets created first
    sem_t elfSem;   //Elve semaphore
    sem_t Santa_sleep;  //Used to get Santa process to wait for being woken up
    sem_t ElfHelp;  //Semaphore for elves getting help for Santa
    sem_t Santa_helping;    //Used for Santa going back to sleep after helping elves
    sem_t Elf_wait_mutex;   //Mutex for Elf_Wait_CNT
    sem_t Deer_CNT_Mutex;   //Mutex for Deer_CNT
    sem_t Deer_Hitched;     //Semaphore for deers getting hitched
    sem_t Santa_hitch_another; //Semaphore for Santa hithing Reindeer
    sem_t Print;
}shared_data;

#define SHMEM_SIZE (sizeof(shared_data))
#define SHMEM_KEY 4895  