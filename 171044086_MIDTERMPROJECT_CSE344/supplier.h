
#ifndef SUPPLIER
#define SUPPLIER

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

enum foodPlates {
    SOUP='P', MAIN_COURSE='C',DESERT='D'
};
void supplier(sem_t * sems, int LxM,int K,int * vars,char * filePath);
void waitSemS(sem_t * sems,int index);
void postSemS(sem_t * sems,int index);
void goingDeliver(sem_t* sems,char * item,int itemLen,int P,int C,int D,int total);
void deliveredKitchen(sem_t* sems,char * item,int itemLen,int P,int C,int D,int total);
void supplierhandler();

#endif