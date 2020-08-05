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
#include <sys/socket.h> 
#include <netdb.h> 
#include <arpa/inet.h>
#include <sys/time.h>

struct Nodes{
    int sourceNode;
    int destinationNode;
};

struct vector {
    int * edges;
    int size;
    int count;
    int front;
    int thereIs;
};

struct cache {
    struct vector datas;
    struct cache * next; 
};

struct workerThread{
    int id;
    int  socketFd;
    struct Nodes * node;
    pthread_cond_t conditionalV;
    pthread_mutex_t mutex;
    int full;
    int * logFd;
    struct vector * v;
    int nodeNumber;
    int * currentWorkThread;
    struct workerThread * next;
    struct cache * c;
    int r;
};

struct extendThread{
    int * s;
    int x;
    int r;
    int * currentWorkThread;
    pthread_cond_t conditionalV;
    pthread_mutex_t mutex;
    int * logFd;
    struct threadLinkList *threadPool;
    struct workerThread * infoThread;
    struct vector ** v;
    struct cache ** c;
    int nodeNumber;
};

struct threadLinkList{
    pthread_t thread;
    struct threadLinkList * next;
};

volatile sig_atomic_t terminate = 0;
volatile sig_atomic_t entitySize = 0;
volatile sig_atomic_t readers=0,writers=0;
pthread_mutex_t commonMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t currentMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t currentCV  = PTHREAD_COND_INITIALIZER;
pthread_cond_t noThreadCV  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t roomEmpty = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t readerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t readSwitch = PTHREAD_MUTEX_INITIALIZER;


static void usageError() {

    write(2,"server: calculate with the path from i1 to i2\n",46);
    write(2,"Example Program Execute : ./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24 -r [0||1||2]\n",98);
    exit(0); 
}

void signalHandler(int sig){
    terminate = 1;
}

void daemonProcess(int logFd);
void loadGraph(int readFd,struct vector ** v,int logFd);
int nodeFind(int readFd);
void pushBack(struct vector * v,int destination);
void findPathBFS(struct cache **c,struct vector **v,int socket,int source,int destination,int nodeNumber,int logFd,int id,int r);
int cacheCheck(struct cache **c,int socket,int source,int destination,int nodeNumber,int logFd,int id,int r);

void *threadFunc(void* in);
void *extendFunc();

int main(int argc, char *const* argv)
{   
    char * pathToLogFile;
    int opt;
    if(argc != 13){
        usageError();
        return -1;
    }
    char * pathToFile;
    int s,x,r;
    int portNO;
    int sockfd,sockfd2;

    while((opt = getopt(argc, argv, "i:p:o:s:x:r:")) != -1)  
    {  
        switch(opt)  
        {    
            case 'i':  
                pathToFile = optarg;  
                break;
            case 'o':  
                pathToLogFile = optarg;
                break;    
            case 'p':  
                portNO = atoi(optarg);
                if (portNO<=0 || portNO>65536){
                    write(2,"PORT : [1,65535]\n",17);
                    usageError();
                    return -1;
                }
                break;
            case 's':  
                s = atoi(optarg);
                if (s<2){
                    write(2,"ThreadNumber : [2,...]\n",23);
                    usageError();
                    return -1;
                }
                break;
            case 'x':  
                x = atoi(optarg);
                if (x<2){
                    write(2,"MaxThreadNumber : [2,...]\n",26);
                    usageError();
                    return -1;
                }
                break;
            case 'r':  
                r = atoi(optarg);
                if (r>2||r<0){
                    write(2,"r -> [0||1||2]\n",26);
                    usageError();
                    return -1;
                }
                break;        
            case ':':  
                usageError();
                return -1;
            case '?':  
                usageError();
                return -1;
        }  
    }
    int logFd,readFd;
    if ((logFd = open(pathToLogFile, O_WRONLY|O_CREAT |O_EXCL| O_APPEND,0777)) == -1){
        logFd = open(pathToLogFile, O_WRONLY|O_EXCL| O_APPEND);
    }
    if (logFd == -1){
        write(2,"There is no such a file",23);
        write(2,pathToLogFile,strlen(pathToLogFile));
        write(2,"\n",1);
    }
    
    key_t key = ftok("noDoubleIns",65); 
    int shmid = shmget(key,1024,0666|IPC_CREAT); 
    char *noInstance = (char*) shmat(shmid,(void*)0,0);

    if (noInstance[0] == 'Y' && noInstance[1] == 'E')
    {
        write(2,"The server is already running. You can't run it again.\n",55);
        exit(0);
    }
    noInstance[0] = 'Y';
    noInstance[1] = 'E';
    
    daemonProcess(logFd); 

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGINT);
    sigaddset(&set,SIGTERM);
    struct sigaction sa;
    memset(&sa,0,sizeof(struct sigaction));
    sa.sa_handler =signalHandler;
    sigaction(SIGTERM,&sa,NULL);
    sigaction(SIGINT,&sa,NULL);

    char buf[100];
    int res;
    write(logFd,"Executing with parameters:\n",27);
    write(logFd,"-i ",3);
    write(logFd,pathToFile,strlen(pathToFile));
    write(logFd,"\n",1);
    write(logFd,"-p ",3);
    res = sprintf(buf, "%d",portNO);
    write(logFd,buf,res);
    write(logFd,"\n",1);
    write(logFd,"-o ",3);
    write(logFd,pathToLogFile,strlen(pathToLogFile));
    write(logFd,"\n",1);
    write(logFd,"-s ",3);
    res = sprintf(buf, "%d",s);
    write(logFd,buf,res);
    write(logFd,"\n",1);
    write(logFd,"-x ",3);
    res = sprintf(buf, "%d",x);
    write(logFd,buf,res);
    write(logFd,"\n",1);
    write(logFd,"-r ",3);
    res = sprintf(buf, "%d",r);
    write(logFd,buf,res);
    write(logFd,"\n",1);

    if ((readFd = open(pathToFile, O_RDONLY)) == -1) {
        write(logFd,"File is not opened\n",19);
        exit(1);
    }
    int nodeNumber = nodeFind(readFd);
    struct vector * v = malloc(nodeNumber*sizeof(struct vector));
    struct cache * c = malloc(sizeof(struct cache));

    loadGraph(readFd,&v,logFd);
    struct sockaddr_in servaddr;    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) { 
        write(logFd,"Socket creation failed\n",23);
        exit(0); 
    }
    int optvalue = 1;
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&optvalue,sizeof(int)) == -1)
    {
        write(logFd,"Socket setsockopt failed\n",25);
        exit(0);
    }

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(portNO);

    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
        write(logFd,"Socket bind failed\n",19);
        terminate = 1;
    } 

    if ((listen(sockfd, x)) != 0) { 
        write(logFd,"Listen failed\n",14);
        terminate = 1; 
    } 

    pthread_t threadPoolExtender;
    struct threadLinkList * headThread = malloc(sizeof(struct threadLinkList));
    struct threadLinkList * tempThread = headThread;
    for (int i = 1; i < s; ++i)
    {
        tempThread->next = malloc(sizeof(struct threadLinkList));
        tempThread = tempThread->next;
    }

    struct workerThread * headinfoThread = malloc(sizeof(struct workerThread));
    struct workerThread * tempInfo = headinfoThread;
    
    for (int i = 1; i < s; ++i)
    {
        tempInfo->next = malloc(sizeof(struct workerThread));
        tempInfo = tempInfo->next;
    }

    int currentWorkThread = 0;
    write(logFd,"A pool of ",10);
    res = sprintf(buf, "%d",s);
    write(logFd,buf,res);
    write(logFd," threads has been created\n",26);
    
    tempThread = headThread;
    tempInfo = headinfoThread;

    for (int i = 0; i < s; ++i)
    {
        tempInfo->id = i;
        tempInfo->full = 0;
        tempInfo->logFd = &logFd;
        tempInfo->v = v;
        tempInfo->c = c;
        tempInfo->nodeNumber = nodeNumber;
        tempInfo->currentWorkThread = &currentWorkThread;
        tempInfo->r = r;

        if (pthread_mutex_init(&tempInfo->mutex, NULL) != 0) { 
            write(logFd,"Mutex init has failed\n",22);
            exit(1); 
        } 
        if (pthread_cond_init(&tempInfo->conditionalV,NULL) != 0){ 
            write(logFd,"Cond init has failed\n",21);
            exit(1); 
        }
        if(pthread_create(&tempThread->thread,NULL,threadFunc,tempInfo) !=0){
            exit(1);
        }
        tempInfo = tempInfo->next;
        tempThread = tempThread->next;
    }

    struct extendThread * infoExtend = malloc(sizeof(struct extendThread));
    infoExtend->s = &s;
    infoExtend->x = x;
    infoExtend->currentWorkThread = &currentWorkThread;
    infoExtend->logFd = &logFd;
    infoExtend->threadPool = headThread;
    infoExtend->infoThread = headinfoThread;
    infoExtend->v = &v;
    infoExtend->c = &c;
    infoExtend->nodeNumber = nodeNumber;
    infoExtend->r = r;

    if (pthread_mutex_init(&infoExtend->mutex, NULL) != 0) { 
            write(logFd,"Mutex init has failed\n",22);
            exit(1); 
    } 
    if (pthread_cond_init(&infoExtend->conditionalV,NULL) != 0){ 
        write(logFd,"Cond init has failed\n",21);
        exit(1); 
    }
    if(pthread_create(&threadPoolExtender,NULL,extendFunc,infoExtend) !=0){
        exit(1);
    }

    while(!terminate){
        sockfd2 = accept(sockfd, NULL,NULL); 
        tempThread = headThread;
        tempInfo = headinfoThread;  
              
        if (sockfd2 < 0) { 
            terminate = 1; 
        }else{

            pthread_mutex_lock(&currentMutex);
            pthread_cond_signal(&infoExtend->conditionalV);
            
            if (currentWorkThread == s)
                pthread_cond_wait(&currentCV,&currentMutex);

            if (currentWorkThread == s){
                pthread_mutex_lock(&commonMutex);
                write(logFd,"No thread is available! Waiting for one.\n",41);
                pthread_mutex_unlock(&commonMutex);
                pthread_cond_wait(&noThreadCV,&currentMutex);
            }
            pthread_mutex_unlock(&currentMutex);
            
            pthread_mutex_lock(&currentMutex);
            for (int i = 0; i < s; ++i){
                pthread_mutex_lock(&tempInfo->mutex);
                if (tempInfo->full == 0)
                {   
                    tempInfo->socketFd = sockfd2;
                    tempInfo->full = 1;
                    pthread_mutex_lock(&commonMutex);
                    write(logFd,"A connection has been delegated to thread id #",46);
                    res = sprintf(buf, "%d",i);
                    write(logFd,buf,res);
                    write(logFd,", system load ",14);
                    res = sprintf(buf, "%0.1lf",(((double)currentWorkThread+1)/s)*100);
                    write(logFd,buf,res);
                    write(logFd,"%\n",2);
                    pthread_mutex_unlock(&commonMutex);
                    pthread_cond_signal(&tempInfo->conditionalV);
                    pthread_mutex_unlock(&tempInfo->mutex);
                    currentWorkThread++;
                    break;
                }else{
                    pthread_mutex_unlock(&tempInfo->mutex);
                }
                tempInfo = tempInfo->next;
            }
            pthread_mutex_unlock(&currentMutex);
        }
    }
    noInstance[0] = 'N';
    noInstance[1] = 'O';
    write(logFd,"Termination signal received, waiting for ongoing threads to complete.\n",70);
    
    tempThread = headThread;
    tempInfo = headinfoThread;
    terminate = 1; 
    for (int i = 1; i < s+1; ++i){
        pthread_mutex_lock(&tempInfo->mutex);
        pthread_cond_signal(&tempInfo->conditionalV);

        pthread_mutex_unlock(&tempInfo->mutex);

        pthread_join(tempThread->thread,NULL);
        pthread_mutex_destroy(&tempInfo->mutex);
        pthread_cond_destroy(&tempInfo->conditionalV);  
        
        tempInfo = tempInfo->next;
        tempThread = tempThread->next;
    }

    struct threadLinkList * tempThread2 = headThread;
    struct workerThread * tempInfo2 = headinfoThread;
    tempThread = headThread;
    tempThread2 = headThread;
    tempInfo = headinfoThread;
    tempInfo2 = headinfoThread;
    for (int i = 1; i < s+1; ++i){
        tempThread2 = tempThread;
        tempThread = tempThread->next;
        free(tempThread2);
        tempInfo2 = tempInfo;
        tempInfo = tempInfo->next;
        free(tempInfo2);
    }
    pthread_cond_signal(&infoExtend->conditionalV);
    pthread_join(threadPoolExtender,NULL);
    pthread_mutex_destroy(&infoExtend->mutex);
    pthread_cond_destroy(&infoExtend->conditionalV);
    free(infoExtend);

    for (int i = 0; i <nodeNumber ; ++i)
        free(v[i].edges);

    free(v);

    struct cache * tempCache = c;
    struct cache * tempCache2 = c;
    for (int i = 0; i <entitySize ; ++i){
        free(tempCache->datas.edges);
        tempCache = tempCache->next;
        free(tempCache2);
        tempCache2 = tempCache;
    }
    if (entitySize == 0)
        free(c);
    write(logFd,"All threads have terminated, server shutting down.\n",51);

    close(sockfd);
    close(logFd);
    close(readFd);
    shmdt(noInstance); 
    return 0;
}

void *threadFunc(void* in){
    struct  workerThread * info = (struct  workerThread*)in;
    int err;
    struct Nodes nodes;
    char buf[100];
    int res;
    while(!terminate){
        
        pthread_mutex_lock(&commonMutex);

        write(*(info->logFd),"Thread #",8);
        res = sprintf(buf, "%d",info->id);
        write(*(info->logFd),buf,res);
        write(*(info->logFd),": waiting for connection\n",25);
        pthread_mutex_unlock(&commonMutex);
        
        err = pthread_mutex_lock(&info->mutex);
        if (err != 0)
        {
            write(*(info->logFd),"Mutex Lock Error\n",17);
            terminate = 1;
        }
        info->full = 0;
        if (terminate == 1)
        {   
            pthread_mutex_unlock(&info->mutex);
            break;
        }

        if(info->full == 0){ 
            pthread_cond_signal(&noThreadCV);    
            pthread_cond_wait(&info->conditionalV,&info->mutex);
        }
        if (terminate == 1)
        {   
            pthread_mutex_unlock(&info->mutex);
            break;
        }
        pthread_mutex_unlock(&info->mutex);
        
        int * socket = &info->socketFd; 
        read(*socket, &nodes, sizeof(nodes));
        int isEntity = cacheCheck(&info->c,*socket,nodes.sourceNode,nodes.destinationNode,info->nodeNumber,*(info->logFd),info->id,info->r);
        if (isEntity == 0)
            findPathBFS(&info->c,&info->v,*socket,nodes.sourceNode,nodes.destinationNode,info->nodeNumber,*(info->logFd),info->id,info->r);        
            
        close(*socket);
        pthread_mutex_lock(&currentMutex);
        *info->currentWorkThread = *info->currentWorkThread - 1;
        pthread_mutex_unlock(&currentMutex);
    }

    return NULL;
}

void *extendFunc(void* in){
    struct  extendThread * info = (struct  extendThread*)in;
    char buf[100];
    int res;
    int dynamic;
    while(! terminate ){
        pthread_mutex_lock(&currentMutex);
        double rate = ((double)*info->currentWorkThread)/((double)*info->s);
        if (rate >= 0.75 && *info->currentWorkThread != info->x)
        {   
            dynamic = *info->s/4; 
            if (dynamic < 1)
                dynamic = 1;
            struct threadLinkList * tempThread = info->threadPool;
            struct workerThread * infoTemp = info->infoThread;
            for (int i = 0; i < *info->s-1 ; ++i)
                infoTemp = infoTemp->next;                 
            for (int i = 0; i < *info->s-1 ; ++i)
                tempThread = tempThread->next;
            int extend = 0;
            int tempS = *info->s;
            for (int i = tempS; (i <(tempS+dynamic))&&(i<info->x) ; ++i)
            {
                infoTemp->next = malloc(sizeof(struct workerThread));
                infoTemp->next->id = i;
                infoTemp->next->full = 0;
                infoTemp->next->logFd = info->logFd;                 
                infoTemp->next->v = *info->v;
                infoTemp->next->c = *info->c;
                infoTemp->next->r = info->r;
                infoTemp->next->nodeNumber = info->nodeNumber;
                infoTemp->next->currentWorkThread = info->currentWorkThread; 

                tempThread->next = malloc(sizeof(struct threadLinkList));
                if (pthread_mutex_init(&infoTemp->next->mutex, NULL) != 0) { 
                    write(*info->logFd,"Mutex init has failed\n",22);
                    exit(1); 
                } 
                if (pthread_cond_init(&infoTemp->next->conditionalV,NULL) != 0){ 
                    write(*info->logFd,"Cond init has failed\n",21);
                    exit(1); 
                }
                if(pthread_create(&tempThread->next->thread,NULL,threadFunc,infoTemp->next) !=0){
                    exit(1);
                }
                infoTemp = infoTemp->next;
                tempThread = tempThread->next;
                *info->s = *info->s + 1;
                extend = 1;
            }
            if (extend == 1)
            {
                pthread_mutex_lock(&commonMutex);
                write(*info->logFd,"System load ",12);
                res = sprintf(buf, "%0.1lf",rate*100);
                write(*info->logFd,buf,res);
                write(*info->logFd,"%, pool extended to ",20);
                res = sprintf(buf, "%d",*info->s);
                write(*info->logFd,buf,res);
                write(*info->logFd," threads\n",9);
                pthread_mutex_unlock(&commonMutex);
            }
        }

        pthread_cond_signal(&currentCV);    
        pthread_cond_wait(&info->conditionalV,&currentMutex);
        if (terminate == 1)
        {   
            pthread_mutex_unlock(&currentMutex);
            break;
        }
        pthread_mutex_unlock(&currentMutex); 
    }
    return NULL;

}

void daemonProcess(int logFd){
    int pid = fork();
    if(pid < 0){
        close(logFd);
        exit(0);
    }
    if(pid > 0){
        close(logFd);
        exit(0);
    }
    setsid();
    umask(027);
}

void loadGraph(int readFd,struct vector ** v,int logFd){
    
    int beginLine = 0,endLine = 0,lineLength = 0;
    char buffer[1];
    char * line = NULL;
    int nodesNumber = 0,edgesNumber = 0;
    int source,destination;
    struct timeval  tv1, tv2;
    gettimeofday(&tv1, NULL);
    write(logFd,"Loading graph...\n",17);
    int num1,num2;
    int check = 0;
    do{    
        if(line!=NULL)
            free(line);
        line = NULL;
        lineLength = 0;
        do{
            endLine++;
            lineLength++;
            check = read(readFd,buffer,sizeof(buffer));
            if (check == 0)
                break;
        }while(buffer[0] != '\n');
        lseek(readFd,-(endLine-beginLine) ,SEEK_CUR);

        line = (char*)malloc((lineLength+1)*sizeof(char));
        read(readFd,line,lineLength);
        line[lineLength] = '\0';
        beginLine = endLine;
        if (line[0] != '#' && line[0] != 13 && line[0] != '\n' ){
            sscanf(line,"%d %d",&num1,&num2);
            if (nodesNumber<num1)
                nodesNumber = num1;
            if (nodesNumber<num2)
                nodesNumber = num2; 
            edgesNumber++;
        }
    }while( line[0] != '\n' && lineLength >=2 && lineLength != 1 && line[0] != ' ' && lineLength != 0);
    if(line!=NULL)
        free(line);
    line = NULL;
    nodesNumber++;
    lseek(readFd,0 ,SEEK_SET);
    for (int i = 0; i < nodesNumber; ++i)
    {
        (*v)[i].edges = (int*)malloc(10*sizeof(int));
        (*v)[i].size = 10;
        (*v)[i].count = 0;
        (*v)[i].thereIs = 0;
    }
    lseek(readFd,0,SEEK_SET);

    do{    
        if (line != NULL)
            free(line);
        line = NULL;            
        lineLength = 0;
        do{
            endLine++;
            lineLength++;
            check = read(readFd,buffer,sizeof(buffer));
            if (check == 0)
                break;
        }while(buffer[0] != '\n'  );
        lseek(readFd,-(endLine-beginLine) ,SEEK_CUR);
        line = (char*)malloc((lineLength+1)*sizeof(char));
        read(readFd,line,lineLength);
        line[lineLength] = '\0';
        beginLine = endLine;
        if (line[0] != '#' && line[0] != 13 && line[0] != '\n' ){
            sscanf(line,"%d %d",&source,&destination);
            pushBack(&(*v)[source],destination);
            (*v)[source].thereIs = 1;
            (*v)[destination].thereIs = 1;
        }
    }while(line[0] != 13 && line[0] != '\n' && lineLength >=2 && lineLength != 1 && line[0] != ' ' && lineLength != 0);
    if (line != NULL)
        free(line);
    line = NULL;            
    
    int tempNodeNumber = 0;
    for (int i = 0; i < nodesNumber; ++i)
    {
        if ((*v)[i].thereIs == 1)
            tempNodeNumber++;
    }

    gettimeofday(&tv2, NULL);
    double takenTime = (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec-tv1.tv_sec);
    char buf[100];
    int res;
    write(logFd,"Graph loaded in ",16);
    res = sprintf(buf, "%0.1lf",takenTime);
    write(logFd,buf,res);
    write(logFd," seconds with ",14);
    res = sprintf(buf, "%d",tempNodeNumber);
    write(logFd,buf,res);
    write(logFd," nodes and ",11);
    res = sprintf(buf, "%d",edgesNumber);
    write(logFd,buf,res);
    write(logFd," edges.\n",8);    
}

int nodeFind(int readFd){
    int beginLine = 0,endLine = 0,lineLength = 0;
    char buffer[1];
    char * line = NULL;
    int nodesNumber = 0,edgesNumber = 0;
    int num1,num2;
    int check = 0;
    do{    
        if(line!=NULL)
            free(line);
        line = NULL;
        lineLength = 0;
        do{
            endLine++;
            lineLength++;
            check = read(readFd,buffer,sizeof(buffer));
            if (check == 0)
                break;
        }while(buffer[0] != '\n');
        lseek(readFd,-(endLine-beginLine) ,SEEK_CUR);

        line = (char*)malloc((lineLength+1)*sizeof(char));
        read(readFd,line,lineLength);
        line[lineLength] = '\0';
        beginLine = endLine;
        if (line[0] != '#' && line[0] != 13 && line[0] != '\n' ){
            sscanf(line,"%d %d",&num1,&num2);
            if (nodesNumber<num1)
                nodesNumber = num1;
            if (nodesNumber<num2)
                nodesNumber = num2; 
            edgesNumber++;
        }
    }while( line[0] != '\n' && lineLength >=2 && lineLength != 1 && line[0] != ' ' && lineLength != 0);

    if(line!=NULL)
        free(line);
    line = NULL;
    nodesNumber++;
    lseek(readFd,0,SEEK_SET);
    return nodesNumber;
}


void pushBack(struct vector * v,int destination){

    if (v->size == v->count)
    {
        v->edges = realloc(v->edges, 2 * v->size * sizeof(int));
        v->size = 2 * v->size;
    }
    v->edges[v->count] = destination;
    v->count++;
}

void findPathBFS(struct cache **c,struct vector **v,int socket,int source,int destination,int nodeNumber,int logFd,int id,int r){

    int * predecessor = (int*)malloc(nodeNumber*sizeof(int));
    int * distance = (int*)malloc(nodeNumber*sizeof(int));
    int * visit = (int*)malloc(nodeNumber*sizeof(int));
    struct vector bfsQueue;

    bfsQueue.edges = (int*)malloc(10*sizeof(int));
    bfsQueue.size = 10;
    bfsQueue.count = 0;
    bfsQueue.front = 0;

    for (int i = 0; i < nodeNumber; ++i)
    {
        visit[i] = 0;
        distance[i] = 2147483640;
        predecessor[i] = -2147483640;
    }
    visit[source] = 1; 
    distance[source] = 0;
    pushBack(&bfsQueue,source);
    int thereIsPath = 0;
    while(bfsQueue.front != bfsQueue.count){
        int front = bfsQueue.edges[bfsQueue.front];
        bfsQueue.front++;
        for (int i = 0; i < (*v)[front].count; ++i)
        {
            if (visit[(*v)[front].edges[i]] == 0)
            {
                predecessor[(*v)[front].edges[i]] = front;
                visit[(*v)[front].edges[i]] = 1;
                distance[(*v)[front].edges[i]] = distance[front] + 1;
                pushBack(&bfsQueue,(*v)[front].edges[i]);
                if ((*v)[front].edges[i] == destination)
                    thereIsPath = 1;
            }
        }
    }

    char buf[100];
    int res;    
    if(thereIsPath == 0){
        pthread_mutex_lock(&commonMutex);
        write(logFd,"Thread #",8);
        res = sprintf(buf, "%d",id);
        write(logFd,buf,res);
        write(logFd,": path not possible from node ",30);
        res = sprintf(buf, "%d",source);
        write(logFd,buf,res);
        write(logFd," to ",4);
        res = sprintf(buf, "%d",destination);
        write(logFd,buf,res);
        write(logFd,"\n",1);
        pthread_mutex_unlock(&commonMutex);
        free(bfsQueue.edges);
        free(visit);
        free(predecessor);
        free(distance);
        return;
    }

    struct vector nodePath;
    
    nodePath.edges = (int*)malloc(10*sizeof(int));
    nodePath.size = 10;
    nodePath.count = 0;
    nodePath.front = 0;

    pushBack(&nodePath,destination);
    while(predecessor[destination] != -2147483640){
        pushBack(&nodePath,predecessor[destination]);
        destination = predecessor[destination];
    }
    
    if (r == 0 || r == 2){
        pthread_mutex_lock(&roomEmpty);
    }else if(r == 1){
        pthread_mutex_lock(&writerMutex);
        writers++;
        if(writers==1)
        pthread_mutex_lock(&readSwitch);
        pthread_mutex_unlock(&writerMutex);
        pthread_mutex_lock(&roomEmpty);
    }

    struct cache * tempC = &(*c)[0]; 
    for (int i = 0; i < entitySize-1; ++i)
        tempC = tempC->next;
    if (entitySize != 0 )
    {
        tempC->next = malloc(sizeof(struct cache));
        tempC = tempC->next;
    }
    tempC->datas.edges = (int*)malloc(10*sizeof(int));
    tempC->datas.size = 10;
    tempC->datas.count = 0;
    tempC->datas.front = 0;
    entitySize++;
    int path[nodePath.count];
    for (int i = nodePath.count-1; i >= 0; --i)
    {
        path[nodePath.count-i-1] = nodePath.edges[i];
        pushBack(&tempC->datas,nodePath.edges[i]);
    }
    if (r == 0||r == 2){
        pthread_mutex_unlock(&roomEmpty);
    }else if(r == 1){
        pthread_mutex_unlock(&roomEmpty);
        pthread_mutex_lock(&writerMutex);
        writers--;
        if(writers==0)
        pthread_mutex_unlock(&readSwitch);
        pthread_mutex_unlock(&writerMutex);
    }

    pthread_mutex_lock(&commonMutex);
    if (nodePath.count != 0 )
    {
        write(logFd,"Thread #",8);
        res = sprintf(buf, "%d",id);
        write(logFd,buf,res);
        write(logFd,": path calculated: ",19);
        for (int i = 0; i < nodePath.count ; ++i)
        {
            res = sprintf(buf, "%d",path[i]);
            write(logFd,buf,res);
            if(nodePath.count-1 != i)
                write(logFd,"->",2);
        }
        write(logFd,"\n",1);

        write(logFd,"Thread #",8);
        res = sprintf(buf, "%d",id);
        write(logFd,buf,res);
        write(logFd,": responding to client and adding path to database",50);
        write(logFd,"\n",1);

    }else{
        write(logFd,"Thread #",8);
        res = sprintf(buf, "%d",id);
        write(logFd,buf,res);
        write(logFd,": path not possible from node ",30);
        res = sprintf(buf, "%d",source);
        write(logFd,buf,res);
        write(logFd," to ",4);
        res = sprintf(buf, "%d",destination);
        write(logFd,buf,res);
        write(logFd,"\n",1);
    }        
    pthread_mutex_unlock(&commonMutex);

    write(socket,&nodePath.count,sizeof(int));
    write(socket,path,sizeof(path));
    free(bfsQueue.edges);
    free(nodePath.edges);
    free(predecessor);
    free(distance);
    free(visit);
}


int cacheCheck(struct cache **c,int socket,int source,int destination,int nodeNumber,int logFd,int id,int r)
{
    char buf[100];
    int res;
    pthread_mutex_lock(&commonMutex);
    write(logFd,"Thread #",8);
    res = sprintf(buf, "%d",id);
    write(logFd,buf,res);
    write(logFd,": searching database for a path from node ",42);
    res = sprintf(buf, "%d",source);
    write(logFd,buf,res);
    write(logFd," to node ",9);
    res = sprintf(buf, "%d",destination);
    write(logFd,buf,res);
    write(logFd,"\n",1);
    pthread_mutex_unlock(&commonMutex);

    if (r == 0)
    {
        pthread_mutex_lock(&readerMutex);
        readers++;
        if (readers == 1)
            pthread_mutex_lock(&roomEmpty);
        pthread_mutex_unlock(&readerMutex);
    }else if(r == 1){
        pthread_mutex_lock(&readSwitch);
        pthread_mutex_lock(&readerMutex);
        readers++;
        if(readers==1)
        pthread_mutex_lock(&roomEmpty);
        pthread_mutex_unlock(&readerMutex);
        pthread_mutex_unlock(&readSwitch);
    }
    else if (r == 2){
        pthread_mutex_lock(&roomEmpty);
    }

    struct cache * tempC = &(*c)[0]; 
    for (int i = 0; i <entitySize ; ++i)
    {
        if (tempC->datas.edges[0] == source && tempC->datas.edges[tempC->datas.count-1] == destination)
        {

            int path[tempC->datas.count];
            for (int j = tempC->datas.count-1; j >= 0; --j)
            {
                path[j] = tempC->datas.edges[j];
            }

            write(socket,&tempC->datas.count,sizeof(int));
            write(socket,path,sizeof(path));
            
            if (r == 0)
            {
                pthread_mutex_lock(&readerMutex);
                readers--;
                if (readers == 0)
                    pthread_mutex_unlock(&roomEmpty);
                pthread_mutex_unlock(&readerMutex);
            }else if(r == 1){
                pthread_mutex_lock(&readerMutex);
                readers--;
                if(readers == 0)
                    pthread_mutex_unlock(&roomEmpty);
                pthread_mutex_unlock(&readerMutex);
            }else if (r == 2){
                pthread_mutex_unlock(&roomEmpty);
            }

            pthread_mutex_lock(&commonMutex);
            write(logFd,"Thread #",8);
            res = sprintf(buf, "%d",id);
            write(logFd,buf,res);
            write(logFd,": path found in database: ",26);

            for (int i = 0; i < tempC->datas.count; ++i)
            {
                res = sprintf(buf, "%d",path[i]);
                write(logFd,buf,res);
                if(tempC->datas.count-1 != i)
                    write(logFd,"->",2);
            }

            write(logFd,"\n",1);
            pthread_mutex_unlock(&commonMutex);

            return 1;
        }
        tempC = tempC->next;
    }

    if (r == 0||r == 1)
    {
        pthread_mutex_lock(&readerMutex);
        readers--;
        if (readers == 0)
            pthread_mutex_unlock(&roomEmpty);
        pthread_mutex_unlock(&readerMutex);
    }
    else if (r == 2){
        pthread_mutex_unlock(&roomEmpty);
    } 

    pthread_mutex_lock(&commonMutex);
    write(logFd,"Thread #",8);
    res = sprintf(buf, "%d",id);
    write(logFd,buf,res);
    write(logFd,": no path in database, calculating ",35);
    res = sprintf(buf, "%d",source);
    write(logFd,buf,res);
    write(logFd,"->",2);
    res = sprintf(buf, "%d",destination);
    write(logFd,buf,res);
    write(logFd,"\n",1);
    pthread_mutex_unlock(&commonMutex);
    return 0;
}

