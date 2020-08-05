
#ifndef COOKS
#define COOKS

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


void cooks(int id,sem_t * sems, int LxM,int * vars,int * soupQ,int * mainQ,int * desertQ);
void waitSemC(sem_t * sems,int index);
void postSemC(sem_t * sems,int index);
void insertQ(int * Queue,int * vars,int varsIndex);
void deleteQ(int * Queue,int * vars,int varsIndex);
void goingToKitchen(sem_t * sems,int id,int P,int C,int D,int total);
void goingToCounter(sem_t *sems,int id,char * item,int itemLen,int P,int C,int D,int total);
void deliveredCounter(sem_t *sems,int id,char * item,int itemLen,int P,int C,int D,int total);
void goodByeCooker(sem_t * sems,int id,int total);

#endif