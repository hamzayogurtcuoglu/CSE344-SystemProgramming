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
#include <pthread.h>

#define CHEFS_NUMBER 6 
#define SHM_DATA "data_171044086_key"
#define SHM_SEMO "semophore_171044086_key"

static void usageError() {

	write(2,"program: is to create a program that will make gullac with 6 chefs and wholesaler \n",83);
	write(2,"./program -i filePath\n",22);
	exit(0); 
}

enum Ingredients{
    MILK = 'M',
    FLOUR = 'F',
    WALNUTS = 'W',
    SUGAR = 'S',
    NOTHING = 'N'
};

enum CHEFS{
    WS,
    SF,
    FW,
    MF,
    MS,
    MW
};

struct chefInfo {
	enum Ingredients lackIng[2];
	enum Ingredients endlessIng[2];
	int no;
};

int fd;
int deliverIngFd;
enum Ingredients * deliveredData;
sem_t * sems;
struct chefInfo *chefinfo;
int terminate = 1;

void* chef(void* arg);
void wholesaler(char * filePath);
void fileCheck(char * filePath);
void initSemaphore(int index,int value);
void postSem(sem_t * sems,int index);
void waitSem(sem_t * sems,int index);
void printIngredient(enum Ingredients item);
void printWaitingChef(struct chefInfo * infoC);
void printDeliverIng(char firstChar,char secondChar);
void printTakenIngs(char firstChar,char secondChar,struct chefInfo * infoC);
void printDeliveredDessert(struct chefInfo * infoC);
void preparingDessertPrint(struct chefInfo * infoC);
void printEndlessSupply(struct chefInfo * infoC);
void closeAll();

int main(int argc, char *const* argv)
{
	/*Usage*/
	srand(time(0)); 
    if(argc != 3){
        usageError();
        return -1;
    }
    int opt;
    char * filePath;
    int shm_fd;
    int err;
    while((opt = getopt(argc, argv, "i:")) != -1)  
    {  
        switch(opt)  
        {    
            case 'i':  
            	filePath = optarg;  
                break;
            case ':':  
        		usageError();
                return -1;
            case '?':  
        		usageError();
                return -1;
        }  
    }
    fileCheck(filePath);
    
    deliverIngFd =shm_open(SHM_DATA,O_RDWR | O_CREAT | O_TRUNC,S_IRUSR|S_IWUSR);
    if(deliverIngFd == -1){
        perror("Error ");
        exit (0);
    }

    shm_fd = shm_open(SHM_SEMO, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        perror("shm_open");
        return -1;
    }

    err = shm_unlink(SHM_DATA);

    if(err == -1){
        perror("shm_unlink");
        return -1;
    }

    err = shm_unlink(SHM_SEMO);

    if(err == -1){
        perror("shm_unlink");
        return -1;
    }

    err = ftruncate(deliverIngFd,2*sizeof(enum Ingredients));
    if (err == -1){
        perror("ftruncate");
        return -1;
    }

    err = ftruncate(shm_fd, sizeof(sem_t)  * 5);

    if (err == -1) {
        perror("ftruncate");
        return -1;
    }
 
    deliveredData =mmap(NULL,2*sizeof(enum Ingredients),PROT_READ | PROT_WRITE,MAP_SHARED,deliverIngFd,0);
    
    if(deliveredData == MAP_FAILED){
        perror("Error ");
        exit(0);
    }

    sems = mmap(NULL, sizeof(sem_t) * 5, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if(sems == MAP_FAILED){
        perror("mmap");
        return -1;
    }

    // ALL SEMAPHORES ARE COMMON FOR SYNCHRONIZATION
    //chefing waiting mutex
    initSemaphore(0,0);
    //wholesaler waiting mutex
    initSemaphore(1,0); 
    // wait delivering mutex
    initSemaphore(2,0);
    // write screen output mutex
    initSemaphore(3,1);     
    // wholesaler is left mutex
    initSemaphore(4,0); 


    pthread_t chefs[CHEFS_NUMBER];

    chefinfo = malloc(sizeof(struct chefInfo)*6);


    for (int i = 0; i < CHEFS_NUMBER; i++){ 
        switch(i){
        	case WS:
        		chefinfo[i].lackIng[0] = WALNUTS; chefinfo[i].lackIng[1] = SUGAR;
        		chefinfo[i].endlessIng[0] = FLOUR; chefinfo[i].endlessIng[1] = MILK;
        		chefinfo[i].no = i+1;
        		pthread_create(&chefs[i], NULL, chef, ((void*)&chefinfo[i]) ); 
        	 	break;
        	case SF:
        		chefinfo[i].lackIng[0] = SUGAR; chefinfo[i].lackIng[1] = FLOUR;
        		chefinfo[i].endlessIng[0] = WALNUTS; chefinfo[i].endlessIng[1] = MILK;
        		chefinfo[i].no = i+1;
        		pthread_create(&chefs[i], NULL, chef, ((void*)&chefinfo[i]) ); 
        	 	break;
        	case FW:
        		chefinfo[i].lackIng[0] = FLOUR; chefinfo[i].lackIng[1] = WALNUTS ;
        		chefinfo[i].endlessIng[0] = SUGAR; chefinfo[i].endlessIng[1] = MILK;
        		chefinfo[i].no = i+1;
        		pthread_create(&chefs[i], NULL, chef, ((void*)&chefinfo[i]) );  
        	 	break;
        	case MF:
        		chefinfo[i].lackIng[0] = MILK; chefinfo[i].lackIng[1] = FLOUR;
        		chefinfo[i].endlessIng[0] = WALNUTS; chefinfo[i].endlessIng[1] = SUGAR;
        		chefinfo[i].no = i+1;
        		pthread_create(&chefs[i], NULL, chef, ((void*)&chefinfo[i]) ); 
        	 	break;
        	case MS:
        		chefinfo[i].lackIng[0] = SUGAR; chefinfo[i].lackIng[1] = MILK;
        		chefinfo[i].endlessIng[0] = WALNUTS; chefinfo[i].endlessIng[1] = FLOUR;
        		chefinfo[i].no = i+1;
        		pthread_create(&chefs[i], NULL, chef, ((void*)&chefinfo[i]) ); 
        	 	break;
        	case MW:
        		chefinfo[i].lackIng[0] = WALNUTS; chefinfo[i].lackIng[1] = MILK;
        		chefinfo[i].endlessIng[0] = FLOUR; chefinfo[i].endlessIng[1] = SUGAR;
        		chefinfo[i].no = i+1;
        		pthread_create(&chefs[i], NULL, chef, ((void*)&chefinfo[i]) ); 
        	 	break;
        }
    } 

    wholesaler(filePath);

    for (int i = 0; i < CHEFS_NUMBER; i++){ 
        pthread_join(chefs[i], NULL); 
    }

	for(int i = 0 ; i < 5 ; ++i){
        err = sem_destroy(&sems[i]);
        if(err == -1){
            perror("sem_destroy");
        	exit(0);
        }
    }
    err = munmap(sems, sizeof(sem_t) * 5);
        if(err == -1){
        perror("munmap");
       	exit(0);
        
    }
    err = munmap(deliveredData, sizeof(enum Ingredients) * 2);

    if(err == -1){
        perror("munmap");
        exit(0);
    }
    free(chefinfo);
    pthread_exit(NULL);
	return 0;
}


void* chef(void* info) 
{ 
    
   	struct chefInfo * infoC = (struct chefInfo*)info;
 	printEndlessSupply(infoC);
 	printWaitingChef(infoC);

 	while(1){
 	 	waitSem(sems,0);
 	 	if (deliveredData[0] == 'N' || deliveredData[1] == 'N' )
			break; 	 		

 	 	if ((deliveredData[0] == infoC->lackIng[0] && deliveredData[1] == infoC->lackIng[1]) || 
 	 		(deliveredData[0] == infoC->lackIng[1] && deliveredData[1] == infoC->lackIng[0]))
 	 	{
 	 		
 	 		printTakenIngs(deliveredData[0],deliveredData[1],infoC);
 	 		preparingDessertPrint(infoC);
 	 		printDeliveredDessert(infoC);
 	 		postSem(sems,1);
 	 		waitSem(sems,2);
 	 		printWaitingChef(infoC); 	 
   			postSem(sems,4);
 	 		
 	 	}else{
 	 		postSem(sems,0);
 	 	}
 	 	
 	}
 	return NULL;
}

void wholesaler(char * filePath) 
{ 
	signal(SIGINT, closeAll);
	if ((fd = open(filePath, O_RDONLY)) == -1) {
		perror("open");
		exit(1);
	}
	char buffer[3]; 
	int byte2_read ;

   	while(terminate){

		byte2_read = read(fd,buffer,3);
		if (!(byte2_read == 3 || byte2_read == 2) )
			break;
		printDeliverIng(buffer[0],buffer[1]);
		deliveredData[0] = buffer[0];
		deliveredData[1] = buffer[1];
		postSem(sems,0);
		waitSem(sems,1);

		waitSem(sems,3);
		write(1,"the wholesaler has obtained the dessert and left to sell it\n",60);
   		postSem(sems,3);

		postSem(sems,2);
   		waitSem(sems,4);
   	}
   	deliveredData[0] = 'N';
	deliveredData[1] = 'N';

   	for (int i = 0; i < 6; ++i)
   	{
		postSem(sems,0);
   	}
   	close(fd);
}

void fileCheck(char * filePath){

	if ((fd = open(filePath, O_RDONLY)) == -1) {
		perror("open");
		exit(1);
	}
	int delivers = 0;
	char buffer[3]; 
	int byte2_read = read(fd,buffer,3);


   	while(byte2_read == 3 || byte2_read == 2){

   		delivers++;
		if (buffer[0] !=  'M' && buffer[0] !=  'W' && buffer[0] !=  'S' && buffer[0] !=  'F')
		{
			write(2,"Please write like MF, WS, SW ... etc. Do not write different characters (just, 'M', 'F', 'S', 'W').\n",100);
			close(fd);
   			exit(0);
		}
   		if (buffer[1] !=  'M' && buffer[1] !=  'W' && buffer[1] !=  'S' && buffer[1] !=  'F')
		{
			write(2,"Please write like MF, WS, SW ... etc. Do not write different characters (just, 'M', 'F', 'S', 'W').\n",100);
			close(fd);
   			exit(0);
		}
		if (buffer[0] == buffer[1] || buffer[2] != '\n')
		{
			write(2,"Please write like MF, WS, SW ... etc. Do not write different characters (just, 'M', 'F', 'S', 'W').\n",100);
			close(fd);
   			exit(0);
		}
		byte2_read = read(fd,buffer,3);
   		
   	}

   	if (delivers < 10)
   	{
   		write(2,"File must has valid content and at least 10 rows.\n",50);
   		close(fd);
   		exit(0);
   	}

   	close(fd);
}


void initSemaphore(int index,int value){
	int err;
	err = sem_init(&sems[index], 1, (unsigned int)value);
    if(err == -1){
        perror("sem_init");
        exit(0);
    }
}

void waitSem(sem_t * sems,int index){
	int err;
	err = sem_wait(&sems[index]);
    if(err == -1){
        if(errno != EINTR)
            perror("sem_wait");
        exit(0);
	}
}
void postSem(sem_t * sems,int index){
	int err;
	err = sem_post(&sems[index]);
    if(err == -1){
        if(errno != EINTR)
            perror("sem_post");
        exit(0);
	}
}

void printIngredient(enum Ingredients item){
    switch(item){
        case MILK :
            write(1,"milk",4);
            break;
        case FLOUR :
            write(1,"flour",5);
        	break;
        case WALNUTS :
            write(1,"walnuts",7);        
        	break;
        case SUGAR :
            write(1,"sugar",5);
            break;
        case NOTHING :
    		break;
    }
}

void printWaitingChef(struct chefInfo * infoC){

	char str[10];
    int temp;
	waitSem(sems,3);
 	write(1,"chef",4);
 	temp = sprintf(str, "%d",infoC->no);
   	write(1,str,temp);
   	write(1," is waiting for ",16);
   	printIngredient(infoC->lackIng[0]);
   	write(1," and ",5);
   	printIngredient(infoC->lackIng[1]);
 	write(1,"\n",1);
 	postSem(sems,3);

}

void printDeliverIng(char firstChar,char secondChar){

	waitSem(sems,3);
	write(1,"the wholesaler delivers ",24);
	if (firstChar == 'M')
		write(1,"milk",4);
	else if (firstChar == 'S')
		write(1,"sugar",5);
	else if (firstChar == 'F')
		write(1,"flour",5);
	else
		write(1,"walnuts",7);
	write(1," and ",5);

	if (secondChar == 'M')
		write(1,"milk",4);
	else if (secondChar == 'S')
		write(1,"sugar",5);
	else if (secondChar == 'F')
		write(1,"flour",5);
	else
		write(1,"walnuts",7);
 	write(1,"\n",1);
 	postSem(sems,3);
}

void printTakenIngs(char firstChar,char secondChar,struct chefInfo * infoC){

	char str[10];
    int temp;

	waitSem(sems,3);

	write(1,"chef",4);
 	temp = sprintf(str, "%d",infoC->no);
   	write(1,str,temp);
   	write(1," has taken the ",15);
   	printIngredient(firstChar);
 	write(1,"\n",1);

 	write(1,"chef",4);
 	temp = sprintf(str, "%d",infoC->no);
   	write(1,str,temp);
   	write(1," has taken the ",15);
   	printIngredient(secondChar);
 	write(1,"\n",1);

 	postSem(sems,3);	
}


void printDeliveredDessert(struct chefInfo * infoC){

	char str[10];
    int temp;

	waitSem(sems,3);

	write(1,"chef",4);
 	temp = sprintf(str, "%d",infoC->no);
   	write(1,str,temp);
   	write(1," has delivered the dessert to the wholesaler",44);
 	write(1,"\n",1);

 	postSem(sems,3);	
}


void preparingDessertPrint(struct chefInfo * infoC){

	char str[10];
    int temp;

	waitSem(sems,3);
	write(1,"chef",4);
 	temp = sprintf(str, "%d",infoC->no);
   	write(1,str,temp);
   	write(1," is preparing the dessert",25);
 	write(1,"\n",1);
 	postSem(sems,3);
 	
	sleep(rand()%5 + 1);
}

void printEndlessSupply(struct chefInfo * infoC){

	char str[10];
    int temp;

	waitSem(sems,3);
	write(1,"chef",4);
 	temp = sprintf(str, "%d",infoC->no);
   	write(1,str,temp);
   	write(1," has an endless supply of ",26);
   	printIngredient(infoC->endlessIng[0]);
   	write(1," and ",5);
   	printIngredient(infoC->endlessIng[1]);
   	write(1," but lacks ",11);
   	printIngredient(infoC->lackIng[0]);
   	write(1," and ",5);
   	printIngredient(infoC->lackIng[1]);
 	write(1,"\n",1);

 	postSem(sems,3);	

}

void closeAll(){

	terminate = 0;
}