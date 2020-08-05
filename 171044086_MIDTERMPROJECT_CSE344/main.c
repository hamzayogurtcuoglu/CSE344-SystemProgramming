
#include "supplier.h"
#include "cooks.h"
#include "studentandgraduate.h"

#define SHM_KEY "midterm_171044086_key"
#define SHM_KEY_VARIABLE "midterm_171044086_key_variable"
#define SHM_KEY_SOUPQUEUE "midterm_171044086_key_SOUP"
#define SHM_KEY_MAINQUEUE "midterm_171044086_key_MAIN"
#define SHM_KEY_DESERTQUEUE "midterm_171044086_key_DESERT"
#define SHM_KEY_TABLE "midterm_171044086_key_TABLE"

#define SEM_NUMBER 17
#define VAR_NUMBER 18

static void usageError() {

	write(2,"program: is to create a program that will simulate the student mess hall of a university.\n",90);
	write(2,"./program -N [N>2] -T [T>=1] -S [S>3] -L [L>=3] -U [U>G>=1] -G [U>G>=1] -F filePath\n",84);
	exit(0); 
}

sem_t * sems;
int * vars;
int * soupQ;
int * mainQ;
int * desertQ;
int * tableNum;
int T;
void mainhandler();
void signalhandler();
void supplierhandler();


void initSemaphore(int index,int value){
	int err;
	err = sem_init(&sems[index], 1, (unsigned int)value);
    if(err == -1){
        perror("sem_init");
        exit(0);
    }
}

void fileContentCheck(char * filename,int numML){

	int fd;
	int P=0;
	int C=0;
	int D=0;
    char buffer[1]; 
    int one;
	if ((fd = open(filename, O_RDONLY)) == -1) {
		perror("open");
		exit(1);
	}   

	for(int i = 0;i<numML*3;i++){
		one = read(fd,buffer,1);
		if (one == 1)
		{
			switch(buffer[0]){
				case 'P':
					P++;
					break;
				case 'C':
					C++;
					break;
				case 'D':
					D++;
					break;
				default:
					close(fd);
					exit(0);
					break;
			}
		}
	}
	close(fd);
	if (P != numML || C != numML || D != numML  )
	{
    	write(2,"File doesn't contain exactly 3LM characters(P,C and D must be equal)\n",69);
		exit(0);
	}
}

int main(int argc, char *const* argv)
{
    srand(time(0));                      
    /*Usage*/
    if(argc != 15){
        usageError(argv[0]);
        return -1;
    }
    int opt;    
    int N,U,G,S,L,K,M;
    char * filePath;
    while((opt = getopt(argc, argv, "N:T:S:L:U:G:F:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'N':  
                N = atoi(optarg);  
                break;
            case 'U':  
            	U = atoi(optarg);
                break;
            case 'G':  
            	G = atoi(optarg);
                break;
            case 'T':  
            	T = atoi(optarg);  
                break;
            case 'S':  
            	S = atoi(optarg);  
                break;
            case 'L':  
            	L = atoi(optarg);  
                break;
            case 'F':  
            	filePath = optarg;  
                break;
            case ':':  
        		usageError(argv[0]);
                return -1;
            case '?':  
        		usageError(argv[0]);
                return -1;
        }  
    }

    M = U + G;
    if (!(N>2 && N<M))
    {
    	write(2,"ATTENTION : N > 2 && N < M(U+G)\n",32);
        usageError(argv[0]);    	
    }
    else if(!(S>3)){
    	write(2,"ATTENTION : S > 3 \n",19);
        usageError(argv[0]);    	
    }
    else if (!(T>=1 && T<M))
    {
    	write(2,"ATTENTION : T >= 1 && T < M(U+G) \n",34);
        usageError(argv[0]);    	
    }
    else if (!(L>=3) )
    {
    	write(2,"ATTENTION : L >= 3\n",19);
        usageError(argv[0]);    	
    }else if (!(G>=1 && U>G ))
    {
    	write(2,"ATTENTION : M=U+G > 3, U>G>=1 \n",31);
        usageError(argv[0]); 
    }else if (! ((U+G+N)<10000))
    {
    	write(2,"N+U+G result must be less than 10000. Of course, this may not be valid for other computers.\n",92);
        usageError(argv[0]); 
    }

    K = 2*L*M + 1;

    fileContentCheck(filePath,M*L); //function checks if there are enough characters in the file

    int shm_fd,shm_variables_fd,err,tablefd;
    int soupQfd,mainQfd,desertQfd;

    shm_fd = shm_open(SHM_KEY, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        perror("shm_open");
        return -1;
    }

    shm_variables_fd = shm_open(SHM_KEY_VARIABLE, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (shm_variables_fd == -1) {
        perror("shm_open");
        return -1;
    }


    soupQfd = shm_open(SHM_KEY_SOUPQUEUE, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        perror("shm_open");
        return -1;
    }

    mainQfd = shm_open(SHM_KEY_MAINQUEUE, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (shm_variables_fd == -1) {
        perror("shm_open");
        return -1;
    }

    desertQfd = shm_open(SHM_KEY_DESERTQUEUE, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (shm_variables_fd == -1) {
        perror("shm_open");
        return -1;
    }
    tablefd = shm_open(SHM_KEY_TABLE, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (tablefd == -1) {
        perror("shm_open");
        return -1;
    }

    err = shm_unlink(SHM_KEY);

    if(err == -1){
        perror("shm_unlink");
        return -1;
    }


    err = shm_unlink(SHM_KEY_VARIABLE);

    if(err == -1){
        perror("shm_unlink");
        return -1;
    }

    err = shm_unlink(SHM_KEY_SOUPQUEUE);

    if(err == -1){
        perror("shm_unlink");
        return -1;
    }

    err = shm_unlink(SHM_KEY_MAINQUEUE);

    if(err == -1){
        perror("shm_unlink");
        return -1;
    }

    err = shm_unlink(SHM_KEY_DESERTQUEUE);

    if(err == -1){
        perror("shm_unlink");
        return -1;
    }

    err = shm_unlink(SHM_KEY_TABLE);

    if(err == -1){
        perror("shm_unlink");
        return -1;
    }

    err = ftruncate(shm_fd, sizeof(sem_t)  * SEM_NUMBER);

    if (err == -1) {
        perror("ftruncate");
        return -1;
    }

    err = ftruncate(shm_variables_fd, sizeof(int)  * VAR_NUMBER);

    if (err == -1) {
        perror("ftruncate");
        return -1;
    }

    err = ftruncate(soupQfd, sizeof(sem_t)  * 10000);

    if (err == -1) {
        perror("ftruncate");
        return -1;
    }

    err = ftruncate(mainQfd, sizeof(int)  * 10000);

    if (err == -1) {
        perror("ftruncate");
        return -1;
    }

    err = ftruncate(desertQfd , sizeof(int)  * 10000);

    if (err == -1) {
        perror("ftruncate");
        return -1;
    }

    err = ftruncate(tablefd , sizeof(int)  * T);

    if (err == -1) {
        perror("ftruncate");
        return -1;
    }

    sems = mmap(NULL, sizeof(sem_t) * SEM_NUMBER, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if(sems == MAP_FAILED){
        perror("mmap");
        return -1;
    }

    vars = mmap(NULL, sizeof(int) * VAR_NUMBER, PROT_READ | PROT_WRITE, MAP_SHARED, shm_variables_fd, 0);

    if(vars == MAP_FAILED){
        perror("mmap");
        return -1;
    }

    soupQ = mmap(NULL, sizeof(int) * 10000, PROT_READ | PROT_WRITE, MAP_SHARED, soupQfd, 0);

    if(soupQ == MAP_FAILED){
        perror("mmap");
        return -1;
    }

    mainQ = mmap(NULL, sizeof(int) * 10000, PROT_READ | PROT_WRITE, MAP_SHARED, mainQfd, 0);

    if(mainQ == MAP_FAILED){
        perror("mmap");
        return -1;
    }

    desertQ = mmap(NULL, sizeof(int) * 10000, PROT_READ | PROT_WRITE, MAP_SHARED, desertQfd, 0);

    if(desertQ == MAP_FAILED){
        perror("mmap");
        return -1;
    }

    tableNum = mmap(NULL, sizeof(int) * T, PROT_READ | PROT_WRITE, MAP_SHARED, tablefd, 0);

    if(tableNum == MAP_FAILED){
        perror("mmap");
        return -1;
    }
    // All tables are assigned first empty(0) - full table(1)
    for (int i = 0; i < T; ++i)
    	tableNum[i] = 0;
 

    //plate of dish for supplier kitchen
    vars[0] = 0; //SOUP
    vars[1] = 0; //MAIN COURSE
    vars[2] = 0; //DESERT
    // Number of items in this kitchen
    vars[3] = 0;
    // All the cooks will be out when how many meals Cookers left on the counter when 3ML.
    vars[4] = 0;
    //the number of dishes on the counter
    vars[5] = 0; //SOUP
    vars[6] = 0; //MAIN COURSE
    vars[7] = 0; //DESERT
    //Supplier is over or not
    vars[8] = 1;
    //Number of empty tables
    vars[9] = T;
	// The back and front indexes of the queues needed to understand if there is a service for COOKERS
    vars[10] = -1; //soup rear
    vars[11] = -1; //soup front
    vars[12] = -1; //...
    vars[13] = -1;
    vars[14] = -1;
    vars[15] = -1;
	//number of students at counter
	vars[16] = 0;
	//number of graduated students at counter
	vars[17] = 0;

    //Kitchen is full or not semaphore
    initSemaphore(0,K);
    //plate of food for kitchen
    initSemaphore(1,0); //SOUP
    initSemaphore(2,0); //MAIN COURSE
    initSemaphore(3,0); //DESERT
    // Number of items in this kitchen
    initSemaphore(4,0);
  	//counter current size semaphore
    initSemaphore(5,S);
    //critical section mutex for cookers (thus there is no starvation)
    initSemaphore(6,1);
    //mutex for the number of items in the kitchen
    initSemaphore(7,1);
    //cooker critic section room 2
    initSemaphore(8,1);
	//table full status
    initSemaphore(9,T);
    //table variable mutex
    initSemaphore(10,1);
	//ready service number for student
    initSemaphore(11,0);
    //counter item mutex
    initSemaphore(12,1);
    //number of students at counter mutex
    initSemaphore(13,1);
    //mutex between graduate and cooker
    initSemaphore(14,1);
    //ready service number for graduated student
    initSemaphore(15,0);
    //print mutex
    initSemaphore(16,1);

    if (fork() == 0)
    {	
    	supplier(sems,L*M,K,vars,filePath);
    	exit(0);
    }

    for (int i = 0; i < N; ++i)
    {
    	if (fork() == 0)
	    {
	    	signal(SIGINT, signalhandler);
    		cooks(i,sems,L*M,vars,mainQ,soupQ,desertQ);
	    	exit(0);
	    }
    }

    for (int i = 0; i < U; ++i)
    {
    	if (fork() == 0)
	    {
	    	signal(SIGINT, signalhandler);
    		studentAndGraduate(sems,L,i,vars,0,tableNum,T);
	    	exit(0);
	    }
    }

    for (int i = 0; i < G; ++i)
    {
    	if (fork() == 0)
	    {
	    	signal(SIGINT, signalhandler);
    		studentAndGraduate(sems,L,i,vars,1,tableNum,T);
	    	exit(0);
	    }
    }

	signal(SIGINT, mainhandler);

    while(wait(NULL) > 0);
    
    for(int i = 0 ; i < SEM_NUMBER ; ++i){
        err = sem_destroy(&sems[i]);
        if(err == -1){
            perror("sem_destroy");
        }
    }

    err = munmap(sems, sizeof(sem_t) * SEM_NUMBER);

    if(err == -1){
        perror("munmap");
        return -1;
    }

    err = munmap(soupQ, sizeof(int) *10000 );
    if(err == -1){
        perror("munmap");
        return -1;
    }

    err = munmap(mainQ, sizeof(int) *10000 );
    if(err == -1){
        perror("munmap");
        return -1;
    }

    err = munmap(desertQ, sizeof(int) *10000 );
    if(err == -1){
        perror("munmap");
        return -1;
    }

    err = munmap(vars, sizeof(int) * VAR_NUMBER);

    if(err == -1){
        perror("munmap");
        return -1;
    }

    err = munmap(tableNum, sizeof(int) * T);

    if(err == -1){
        perror("munmap");
        return -1;
    }
	return 0;
}

void mainhandler(){
    while(wait(NULL) > 0){}
	int err;
	for(int i = 0 ; i < SEM_NUMBER ; ++i){
        err = sem_destroy(&sems[i]);
        if(err == -1){
            perror("sem_destroy");
        }
    }

    err = munmap(sems, sizeof(sem_t) * SEM_NUMBER);

    if(err == -1){
        perror("munmap");
        exit(1);
    }

    err = munmap(soupQ, sizeof(int) *10000 );
    if(err == -1){
        perror("munmap");
        exit(1);
    }

    err = munmap(mainQ, sizeof(int) *10000 );
    if(err == -1){
        perror("munmap");
        exit(1);
    }

    err = munmap(desertQ, sizeof(int) *10000 );
    if(err == -1){
        perror("munmap");
        exit(1);
    }

    err = munmap(vars, sizeof(int) * VAR_NUMBER);

    if(err == -1){
        perror("munmap");
        exit(1);
    }

    err = munmap(tableNum, sizeof(int) * T);

    if(err == -1){
        perror("munmap");
        exit(1);
    }
}
void signalhandler(){
	exit(0);
}
