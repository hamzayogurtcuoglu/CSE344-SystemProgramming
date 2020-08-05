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

static void usageError() {

	write(2,"client: client program that sends the nodes's id from socket\n",61);
	write(2,"Example Program Execute : ./client -a 127.0.0.1 -p PORT -s 768 -d 979\n",70);
	exit(0); 
}

int main(int argc, char *const* argv)
{
	if(argc != 9){
        usageError();
        return -1;
    }
    int opt;
    char * socketAdress;
    int sourceNode,destinationNode;
    int portNO;
    int sockfd;

    while((opt = getopt(argc, argv, "a:p:s:d:")) != -1)  
    {  
        switch(opt)  
        {    
            case 'a':  
            	socketAdress = optarg;  
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
            	sourceNode = atoi(optarg);
                if (sourceNode<0){
					write(2,"SourceNode : Positive integer  -> s\n",36);
					usageError();
                    return -1;
                }
                break;
            case 'd':  
            	destinationNode = atoi(optarg);
                if (destinationNode<0){
					write(2,"DestinationNode : Positive integer  -> d\n",41);
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

    printf("Client (%d) connecting to %s:%d\n",getpid(),socketAdress,portNO);
    struct sockaddr_in servaddr; 	
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) { 
        printf("Socket creation failed...\n"); 
        exit(0); 
    } 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(socketAdress); 
    servaddr.sin_port = htons(portNO);

	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
    struct timeval  tv1, tv2;
    gettimeofday(&tv1, NULL);
	printf("Client (%d) connected and requesting a path from node %d to %d\n",getpid(),sourceNode,destinationNode);
    struct Nodes nodes;
	nodes.sourceNode = sourceNode;
	nodes.destinationNode = destinationNode;
	write(sockfd, &nodes, sizeof(nodes)); 
    int len = 0;
    read(sockfd, &len,sizeof(len)); 
    int * result = (int*)malloc(len*sizeof(int));
    
    read(sockfd, (int *)result,len*sizeof(int));
    gettimeofday(&tv2, NULL);
    double takenTime = (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec-tv1.tv_sec);
	
    if (len != 0 )
    {
        printf("Server’s response to (%d): ",getpid());
        for (int i = 0; i < len; ++i)
        {
            printf("%d",result[i] );
            if (i != len-1)
                printf("->");                
        }
        printf(", arrived in %lfseconds.\n",takenTime);
    }else{
        printf("Server’s response (%d): NO PATH, arrived in %lfseconds, shutting down\n",getpid(),takenTime );
    }
    free(result);
    close(sockfd);
	return 0;
}