#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>
#include "Shared_mem.h"
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>

pid_t Santa;
int fd;

void Create_Santa(int NR);
void Create_Elves(int NE, pid_t *Elves, int TE);
void Create_Reindeers(int NR, pid_t *Reindeers, int TR);
void SemInit();

shared_data *DATA;

int main(int argc, char *argv[]){
    
    //Check stdin
    if(argc != 5){
        fprintf(stderr, "ERROR: Invalid arguments or bad syntax!\n");
        exit(1);
    }

    //Get parameters from stdin
    int NE = atoi(argv[1]); //Num of elves
    int NR = atoi(argv[2]); //Num of Reindeer
    int TE = atoi(argv[3]); //Max. time of an elve working
    int TR = atoi(argv[4]); //Max. time of a reindeer being on holiday
    //------------------------------------------------------------------------ 

    //Check if the parameters are within the bounds
    if(NE < 1 || NE > 999){
        fprintf(stderr, "Invalid NE parameter 0 < NE < 1000\n");
        exit(1);
    }

    if(NR < 1 || NR > 19){
        fprintf(stderr, "Invalid NR parameter 0 < NR < 20\n");
        exit(1);
    }

    if(TE < 0 || TE > 1000){
        fprintf(stderr, "Invalid TE parameter 0 <= TE <= 1000\n");
        exit(1);
    }

    if(TR < 0 || TR > 1000){
        fprintf(stderr, "Invalid TR parameter 0 <= TR <= 1000\n");
        exit(1);
    }
    //-------------------------------------------------------------------------

    pid_t Elves[NE];
    pid_t ReinDeers[NR];
    int status;

    //Connect Shared memmory---------------------------------------------------
    key_t SHM_KEY = SHMEM_KEY;
    int SHM_FLAG = IPC_CREAT | S_IWUSR | S_IRUSR;

    int SHM_ID=shmget(SHM_KEY, SHMEM_SIZE, SHM_FLAG);
    if(SHM_ID == -1){
        fprintf(stderr, "ERROR: Main failed to connect to shared memory!\n");
        perror("shmget");
        exit(1);
    }

    DATA = (shared_data *) shmat( SHM_ID, NULL, 0);
    if(DATA == (shared_data *) -1){
        fprintf(stderr, "ERROR: Main failed to atttach shared memory block!\n");
        exit(1);
    }
    //Remove last proj2.out if it exists
    remove("proj2.out");

    //Open proj2.out
    fd = open("proj2.out", O_WRONLY | O_CREAT, 0777);
    if(fd == -1){
        fprintf(stderr, "File could not be opened or created!");
        exit(1);
    }
    dup2(fd, STDOUT_FILENO);

    //Initialize semaphores
    SemInit();

    //Create Processes
    Create_Santa(NR);
    sem_wait(&DATA -> Make_Santa);
    Create_Elves(NE, Elves, TE);
    Create_Reindeers(NR, ReinDeers, TR);

    // Wait for the processes to finish 
	for (int i = 0; i < NR; i++)
		waitpid(ReinDeers[i], &status, 0);

	for (int i = 0; i < NE; i++)
		waitpid(Elves[i], &status, 0);

	//Kill the santa process
	kill(Santa, SIGKILL);
    
	//Destroy the shared memory 
	shmctl(SHM_ID, IPC_RMID, NULL);

    close(fd);
    exit(0);
}

//Create Santa Process---------------------------------------------
void Create_Santa(int NR){
    char NumDeer[50];

    Santa = fork();
    if(Santa < 0){
        fprintf(stderr,"ERROR: Forking Santa failed!\n");
        exit(1);
    }
    else if(Santa == 0)
	{
        sprintf(NumDeer, "%d", NR);
		if( execl("./Santa","Santa", &NumDeer, NULL) < 0)
		{
			fprintf(stderr,"Making process Santa failed\n");
			exit(1);
		}
	}
}
//-----------------------------------------------------------------

//Create Elve processes--------------------------------------------
void Create_Elves(int NE, pid_t *Elves, int TE){
    char id[50];
    char max_time[50];

    for (int i = 0; i < NE; i++){
        Elves[i] = fork();

        if(Elves[i] < 0){
            fprintf(stderr, "ERROR: Forking Elve failed!\n");
            exit(1);
        }
        else if(Elves[i] == 0){
            sprintf(id, "%d", i + 1);
            sprintf(max_time, "%d", TE);
            if(execl("./Elve","Elves", &id, &max_time, (char*)0) < 0){
                fprintf(stderr,"Making process Elve failed\n");
			    exit(1);
            }
        }
    }
}
//------------------------------------------------------------------

//Create Reindeer processes-----------------------------------------
void Create_Reindeers(int NR, pid_t *Reindeers, int TR){
    char id[50];
    char max_time[50];
    char NumDeer[50];

    for(int i = 0; i < NR; i++){
        Reindeers[i] = fork();

        if(Reindeers[i] < 0){
            fprintf(stderr, "ERROR: Forking Reindeer failed!\n");
            exit(1);
        }
        else if(Reindeers[i] == 0){
            sprintf(id, "%d", i + 1);
            sprintf(max_time, "%d", TR);
            sprintf(NumDeer, "%d", NR);
            if(execl("./Reindeer","Reindeers", &id, &max_time, &NumDeer, (char*)0) < 0){
                fprintf(stderr,"Making process Reindeer failed\n");
			    exit(1);
            }
        }
    }
}
//-----------------------------------------------------------------

//Initialize shared memory-----------------------------------------
void SemInit(){
    sem_init(&DATA -> Make_Santa, 1, 0);
    sem_init(&DATA -> elfSem, 1, 3);
    sem_init(&DATA -> Santa_sleep, 1, 0);
    sem_init(&DATA -> ElfHelp, 1, 1);
    sem_init(&DATA -> Santa_helping, 1, 0);
    sem_init(&DATA -> Elf_wait_mutex, 1, 1);
    sem_init(&DATA -> Deer_CNT_Mutex, 1, 1);
    sem_init(&DATA -> Deer_Hitched, 1, 0);
    sem_init(&DATA->Santa_hitch_another, 1, 1);
    sem_init(&DATA->Print, 1, 1);

    DATA->Elf_Wait_CNT = 0;
    DATA->Elves_Get_Help = 0;
    DATA->workshop_STE = 0;
    DATA->lock = 0;
    DATA->Deer_CNT = 0;
    DATA->All_Deer_Back = 0;
    DATA->PrintCNT = 1;
}
//-----------------------------------------------------------------
