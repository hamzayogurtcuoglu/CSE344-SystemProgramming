
#ifndef STUDENTGRADUATE
#define STUDENTGRADUATE

#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <memory.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <err.h>
#include <semaphore.h>
#include <sys/shm.h>


void studentAndGraduate(sem_t * sems,int L,int id,int * vars,int gradOrStudent,int * tableNum,int T);
void waitSemSG(sem_t * sems,int index);
void postSemSG(sem_t * sems,int index);
void goingSCounter(sem_t * sems,int gradOrStudent,int id,int round,int atCounter,int P,int C,int D,int total);
void gotFood(sem_t * sems,int gradOrStudent,int id,int round,int emptyTable);
void satTable(sem_t * sems,int gradOrStudent,int id,int tableNo,int round,int emptyTable);
void leftTable(sem_t * sems,int gradOrStudent,int id,int round,int emptyTable);
void goodByeS(sem_t * sems,int gradOrStudent,int id,int round);


#endif
