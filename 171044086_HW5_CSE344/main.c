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


struct client{

    char * name;
    int X;
    int Y;
    char * requestFlower;
};

struct Queue
{
    int front;
    int rear;
    struct client * waitingClients;
};

struct saleStatis
{
  double totalTime;
  int numberOfSales; 
};

struct florist
{
  int X;
  int Y;
  double speed;
  char * name;
  char ** flowerType;
  int flowerTypeSize;
  struct Queue que;
  struct saleStatis sta;
  pthread_mutex_t mutexF;
  pthread_cond_t conditionalV;
  int closingVariable;
};

struct florist * florists = NULL;
int terminate = 1;

static void usageError() {

	write(2,"floristApp: is to create a program that will make gullac with 6 chefs and wholesaler \n",83);
	write(2,"./floristApp -i filePath\n",22);
	exit(0); 
}

void signalHandler(){
  terminate = 0;
}

void *flowerFunc(void *floristInfo);
int numberClients(char * filePath);
int distanceChebyshev(struct florist * f,struct client *c);
int flowerExistInFlorist( char ** flowerType, char * c,int flowerTypeSize);

pthread_mutex_t printMutex = PTHREAD_MUTEX_INITIALIZER;


void mainThread(struct florist * florists,char * filePath,int floristNumber){
    int err;
    signal(SIGINT, signalHandler);
    char buffer[1],temp[1];
    int fd;

    temp[0] = '-';
    if ((fd = open(filePath, O_RDONLY)) == -1) {
      write(2,"File doesn't exist.\n",20);
      exit(1);
    }

    while(1){
      read(fd,buffer,sizeof(buffer));
      if (buffer[0] == '\n')
      {
        if (temp[0] == '\n' && buffer[0] == '\n')
          break;
        temp[0] = buffer[0];
      }else{
        temp[0] = buffer[0];
      }
    }

    temp[0] = '-';
    char * line;
    int nameLength = -1;
    int beginLine = lseek(fd, 0, SEEK_CUR);
    int endLine = lseek(fd, 0, SEEK_CUR);
    int lineLength = 0;
    struct client c;
    struct florist * f;
    
    while(terminate){
      
      int k;
      do{
        nameLength++;
        k = read(fd,buffer,sizeof(buffer));

      }while(buffer[0] != ' ' && k != 0);
      lseek(fd,beginLine,SEEK_SET);
      c.name = (char*)malloc((nameLength+1)*sizeof(char));
      do{
        
        endLine++;
        lineLength++;
        k = read(fd,buffer,sizeof(buffer));
      }while(buffer[0] != '\n'&& k!=0);
      if (k == 0)
        lineLength++;
      line = (char*)malloc((lineLength)*sizeof(char));
      lseek(fd,-(endLine-beginLine) ,SEEK_CUR);
      read(fd,line,lineLength);
      line[lineLength-1] = '\0';
      if(lineLength<=2){
          free(line);
          break;
      }

      sscanf(line,"%s (%d,%d)",c.name,&c.X,&c.Y);
      c.name[nameLength] = '\0';
      int linePaserIndex = 0;
      while(line[linePaserIndex] != ':'){
        
        linePaserIndex++;
      }
            
      int flowerLen = 0;
      int index2 = 0;
      for (int i = linePaserIndex; i < lineLength; ++i)
      { 

        if (line[i] == ',' || lineLength - i == 1){
          c.requestFlower = (char*) malloc((flowerLen+1)*sizeof(char));
          flowerLen = 0;
          index2++;
        }
        
        else if (line[i] == ' ' || line[i] == ':')
        {
          continue;
        }else{
          flowerLen++;
        }
      }

      index2 = 0;
      flowerLen = 0;
      for (int i = linePaserIndex; i < lineLength; ++i)
      { 
        if (line[i] == ','){
          flowerLen = 0;
          index2++;
        }
        else if (line[i] == ' '|| line[i] == ':')
        {
          continue;
        }else{
          c.requestFlower[flowerLen] = line[i];
          flowerLen++;
        }
      }

      free(line);
      nameLength = -1;
      beginLine = endLine;
      lineLength = 0;
      f = NULL;
      for(int i=0;i<floristNumber;++i){

          if(flowerExistInFlorist(florists[i].flowerType,c.requestFlower,florists[i].flowerTypeSize) && 
            (f == NULL || 
              (distanceChebyshev(&florists[i],&c) < distanceChebyshev(f,&c)))){
              
              f =&florists[i];
          }
      }
      if (f != NULL)
      {
        err = pthread_mutex_lock(&f->mutexF);
        if (err != 0)
        {
        	write(2,"Main Thread Mutex Lock Error\n",29);
        	terminate = 0;
        }
        f->que.rear++;
        f->que.waitingClients[f->que.rear].name = c.name;
        f->que.waitingClients[f->que.rear].X = c.X;
        f->que.waitingClients[f->que.rear].Y = c.Y;
        f->que.waitingClients[f->que.rear].requestFlower = c.requestFlower;
        f->que.waitingClients[f->que.rear].requestFlower[--flowerLen] = '\0';
        err = pthread_cond_signal(&f->conditionalV);
        if (err != 0)
        {
        	write(2,"Main Thread Signal Error\n",25);
        	terminate = 0;
        }
        err = pthread_mutex_unlock(&f->mutexF);
      	if (err != 0)
        {
        	write(2,"Main Thread Mutex Unlock Error\n",31);
        	terminate = 0;
        }
      }
    }
    free(c.name);
    close(fd);
}

void *flowerFunc(void *floristI){
    struct florist *floristInfo =(struct florist*)floristI;
    double time = 0;
    int err;
    while(terminate){
        err = pthread_mutex_lock(&floristInfo->mutexF);
        if (err != 0)
        {
        	write(2,"Mutex Lock Error\n",17);
        	terminate = 0;
        }
        if(floristInfo->que.rear<(floristInfo->que.front+1)||!terminate){
            if(floristInfo->closingVariable||!terminate){
                err = pthread_mutex_unlock(&floristInfo->mutexF);
                if (err != 0)
		        {
		        	write(2,"Mutex Unlock Error\n",19);
		        	terminate = 0;
		        }
                pthread_exit(  &floristInfo->sta);
            }
            pthread_cond_wait(&floristInfo->conditionalV,&floristInfo->mutexF);
            if(floristInfo->que.rear<(floristInfo->que.front+1)){
                if(floristInfo->closingVariable){
                  err = pthread_mutex_unlock(&floristInfo->mutexF);
                  if (err != 0)
			      {
			        write(2,"Mutex Unlock Error\n",19);
			        terminate = 0;
			       }
                  pthread_exit( &floristInfo->sta);
            }
            pthread_cond_wait(&floristInfo->conditionalV,&floristInfo->mutexF);
            
            }    
        }
        floristInfo->que.front++;
        struct client * c;
        c = &floristInfo->que.waitingClients[floristInfo->que.front];
        pthread_mutex_unlock(&floristInfo->mutexF);
        time =rand()%250+1 ; 
        time += distanceChebyshev(floristInfo,c)/floristInfo->speed;
        usleep(1000*time);
        pthread_mutex_lock(&printMutex);
        write(1,"Florist ",8);
        write(1,floristInfo->name,strlen(floristInfo->name));
        write(1," has delivered a ",17);
        write(1,c->requestFlower,strlen(c->requestFlower));
        write(1," to ",4);
        write(1,c->name,strlen(c->name));
        write(1," in ",4);
        char timeChar[100];
        int res;
        res = sprintf(timeChar, "%.3lf",time);
        write(1,timeChar,res);
        write(1," ms \n",5);        
        pthread_mutex_unlock(&printMutex);
        floristInfo->sta.totalTime +=time;
        floristInfo->sta.numberOfSales++;
    }
    pthread_exit( &floristInfo->sta);
}


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
  int numberFlorists;
  fprintf(stdout, "Florist application initializing from file: %s\n",argv[2] );
  char buffer[1],temp[1]; 
  int fd;
  temp[0] = '-';
  if ((fd = open(filePath, O_RDONLY)) == -1) {
    write(2,"File doesn't exist.\n",20);
    exit(1);
  }
  int size = 0;    
  while(1){

    read(fd,buffer,sizeof(buffer));
    
    if (buffer[0] == '\n')
    {
      if (temp[0] == '\n' && buffer[0] == '\n')
        break;
      size++;
      temp[0] = buffer[0];
    }else{
      temp[0] = buffer[0];
    }
  }
  numberFlorists = size;
  lseek(fd,0,SEEK_SET);
  florists = malloc (sizeof(struct florist)*size);
  int index = 0;
  int nameLength = -1;
  int beginLine = 0,endLine = 0;
  int lineLength = 0;
  char * line = NULL;
  // Florist creation
  while(index != size){
      do{
        nameLength++;
        read(fd,buffer,sizeof(buffer));
      }while(buffer[0] != ' ');
      lseek(fd,beginLine,SEEK_SET);
      florists[index].name = (char*)malloc((nameLength+1)*sizeof(char));
      do{
        endLine++;
        lineLength++;
        read(fd,buffer,sizeof(buffer));
      }while(buffer[0] != '\n');
      line = (char*)malloc(lineLength*sizeof(char));
      lseek(fd,-(endLine-beginLine) ,SEEK_CUR);
      read(fd,line,lineLength);
      line[lineLength-1] = '\0';
      sscanf(line,"%s (%d,%d; %lf)",florists[index].name,&florists[index].X,&florists[index].Y,&florists[index].speed) ;
      florists[index].name[nameLength] = '\0';
      nameLength = -1;
      int linePaserIndex = 0;
      while(line[linePaserIndex] != ':'){
        linePaserIndex++;
      }
      int commaCounter = 1;
      for (int i = linePaserIndex; i < lineLength; ++i)
      {
        if (line[i] == ',')
          commaCounter++;        
      }
      florists[index].flowerTypeSize = commaCounter;
      florists[index].flowerType = (char**) malloc(commaCounter*sizeof(char*));
      int flowerLen = 0;
      int index2 = 0;
      for (int i = linePaserIndex; i < lineLength; ++i)
      { 
        if (line[i] == ',' || lineLength - i == 1){
          florists[index].flowerType[index2] = (char*) malloc((flowerLen+1)*sizeof(char));
          flowerLen = 0;
          index2++;
        }
        else if (line[i] == ' ' || line[i] == ':')
        {
          continue;
        }else{
          flowerLen++;
        }
      }
      index2 = 0;
      flowerLen = 0;
      for (int i = linePaserIndex; i < lineLength; ++i)
      { 
        if (line[i] == ','){
          florists[index].flowerType[index2][flowerLen] = '\0';
          flowerLen = 0;
          index2++;
        }
        else if (line[i] == ' '|| line[i] == ':')
        {
          continue;
        }else{
          florists[index].flowerType[index2][flowerLen] = line[i];
          flowerLen++;
        }
      }
      florists[index].flowerType[index2][--flowerLen] = '\0';
      florists[index].sta.totalTime = 0;
      florists[index].sta.numberOfSales = 0;
      florists[index].closingVariable = 0;
            
      pthread_mutex_init(&florists[index].mutexF,NULL);
      pthread_cond_init(&florists[index].conditionalV,NULL);

      free(line);
      beginLine = endLine;
      index++;
      lineLength = 0;
  }
  //Florist Creation End
    close(fd);
    fprintf(stdout, "%d florists have been created\n",numberFlorists );
    write(1,"Processing requests\n",20);
    pthread_t *floristThreads;
    floristThreads = malloc(numberFlorists*sizeof(pthread_t));
    int numberC = numberClients(filePath);
    struct saleStatis **stas;
    stas =(struct saleStatis**)malloc(sizeof(struct saleStatis*)*numberFlorists);

    for (int i = 0; i < numberFlorists; ++i)
    {
      florists[i].que.rear = -1;
      florists[i].que.front = -1;
      florists[i].que.waitingClients = malloc (sizeof(struct client)*numberC);       
    }

    for(int i=0;i<numberFlorists;++i){
        pthread_create(&floristThreads[i],NULL,flowerFunc,&florists[i]);
    }

    mainThread(florists,filePath,numberFlorists);

    for(int i=0;i<numberFlorists;++i){
        pthread_mutex_lock(&florists[i].mutexF);
        florists[i].closingVariable = 1;
        pthread_cond_signal(&florists[i].conditionalV);
        pthread_mutex_unlock(&florists[i].mutexF);
        pthread_join(floristThreads[i],(void**)&stas[i]);
        
    }
    write(1,"All requests processed\n",23);
    for (int i = 0; i < numberFlorists; ++i)
    {
      write(1,florists[i].name,strlen(florists[i].name));
      write(1," closing shop.\n",15);
    }
    write(1,"Sale statistics for today:\n",27); 
    write(1,"-------------------------------------------------\n",50);  
    write(1,"Florist          # of sales          Total time\n",48);
    write(1,"-------------------------------------------------\n",50);  
    
    for(int i=0;i<numberFlorists;++i){
    
        fprintf(stdout,"%-9s%9c%-12d%8c%.3lfms\n",florists[i].name,' ',stas[i]->numberOfSales,' ',stas[i]->totalTime);
    }

    for(int i=0;i<numberFlorists;++i){
      
        for (int j = 0; j < florists[i].que.rear+1; ++j)
        {
          free(florists[i].que.waitingClients[j].name);
          free(florists[i].que.waitingClients[j].requestFlower); 
        }
        free(florists[i].que.waitingClients);

        for (int j = 0; j < florists[i].flowerTypeSize; ++j)
        {
          free(florists[i].flowerType[j]);
        }
        free(florists[i].flowerType);
        free(florists[i].name);
        pthread_mutex_destroy(&florists[i].mutexF);
        pthread_cond_destroy(&florists[i].conditionalV);    
    }
    free(floristThreads);            
    free(stas);
    free(florists);
    
    pthread_mutex_destroy(&printMutex);
}


int numberClients(char * filePath){

  char buffer[1],temp[1]; 
  int fd;
  temp[0] = '-';
  if ((fd = open(filePath, O_RDONLY)) == -1) {
    write(2,"File doesn't exist.\n",20);
    exit(1);
  }
  int size = 0;    
  while(1){
    read(fd,buffer,sizeof(buffer));
    if (buffer[0] == '\n')
    {
      if (temp[0] == '\n' && buffer[0] == '\n')
        break;
      temp[0] = buffer[0];
    }else{
      temp[0] = buffer[0];
    }
  }
  temp[0] = '-';
  int readChar;
  while(1){
    readChar = read(fd,buffer,sizeof(buffer));
    if (buffer[0] == '\n' || readChar == 0)
    {
      if ((temp[0] == '\n' && buffer[0] == '\n')|| readChar == 0)
        break;
      size++;
      temp[0] = buffer[0];
    }else{
      temp[0] = buffer[0];
    }
  }
  close(fd);
  return size;
}

int distanceChebyshev(struct florist * f,struct client *c){

    int x = abs((c->X)-(f->X));
    int y = abs((c->Y)-(f->Y));
    if (x>y)
      return x;
    else
      return y;
}

int flowerExistInFlorist( char ** flowerType,char * requestFlower,int flowerTypeSize){

    int j = 0;
    for(int i=0;i<flowerTypeSize;i++){
      j = 0;      
      while (1)
      { 
          if (flowerType[i][j] != '\0' && requestFlower[j] == '\0' )
              break;
          if (flowerType[i][j] == '\0' && requestFlower[j] != '\0' )
              break;
          if (flowerType[i][j] == '\0' && requestFlower[j] == '\0' )
              return 1;
          if (flowerType[i][j] != requestFlower[j])
              break;
          j++;
      }
    }
    return 0;
}