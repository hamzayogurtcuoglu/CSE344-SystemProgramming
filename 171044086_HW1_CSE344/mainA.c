#include<stdio.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <memory.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <sys/file.h>
#include <errno.h>
#include <malloc.h>
//int a_as_int = (int)'a'; //ASCII CEVIRME

int  main(int argc, char *const* argv)
{
	/*Usage*/
	if(argc != 7){
		fprintf(stderr, "programA: Reads the characters from file since their ASCII equivalent then that will be convert to complex number before write the a common file. \n");
		fprintf(stderr, "Usage: %s -i inputPathA -o outputPathA -t time \n", argv[0]);
		return -1;
	}

	int opt;
	int time = 0;
	
	char *inputFile;
	char *outputFile;      
    
    while((opt = getopt(argc, argv, "t:i:o:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'i':  
                inputFile = optarg;  
                break;
            case 't':  
            	time = atoi(optarg);
            	if (time>50||time<1){
            		fprintf(stderr, "Time : Integer in [1,50] !\n");
            		fprintf(stderr, "programA: Reads the characters from file since their ASCII equivalent then that will be convert to complex number before write the a common file. \n");
				    fprintf(stderr, "Usage: %s -i inputPathA -o outputPathA -t time \n", argv[0]);
				    return -1;
            	}

                break;
            case 'o':  
                outputFile = optarg;  
                break;
            case ':':  
                fprintf(stderr, "programA: Reads the characters from file since their ASCII equivalent then that will be convert to complex number before write the a common file. \n");
				fprintf(stderr, "Usage: %s -i inputPathA -o outputPathA -t time \n", argv[0]);
				return -1;
            case '?':  
                fprintf(stderr, "programA: Reads the characters from file since their ASCII equivalent then that will be convert to complex number before write the a common file. \n");
				fprintf(stderr, "Usage: %s -i inputPathA -o outputPathA -t time \n", argv[0]);
				return -1;
        }  
    }     
    
    char buffer[32]; 
    int fd,fd2;

    int byte32_read;

	if ((fd = open(inputFile, O_RDONLY)) == -1) {
		perror("open");
		exit(1);
	}    

	struct flock lock;

	if ((fd2 = open(outputFile, O_WRONLY|O_CREAT |O_EXCL| O_APPEND,0777)) == -1) {
		fd2 = open(outputFile, O_WRONLY|O_EXCL| O_APPEND);
	}   

	memset(&lock,0,sizeof(lock));
	lock.l_type = F_WRLCK;

	do{

		byte32_read = read(fd,buffer,sizeof(buffer));
		if (byte32_read == 32){
			
			lock.l_type = F_WRLCK;
			fcntl(fd2,F_SETLKW,&lock);

			if ((fd2 = open(outputFile, O_WRONLY|O_CREAT |O_EXCL| O_APPEND,0777)) == -1) {
				fd2 = open(outputFile, O_WRONLY|O_EXCL| O_APPEND);
			}  		
			for (int i = 0; i < 32; ++i)
			{
				if (i%2==0){
					if (i!=0)
						write(fd2, ",", 1);
					
					int totalDigits = 0;
					int ascii = (int)buffer[i];
					int temp = ascii;
					while(temp!=0){
						temp = temp/10;
					    totalDigits ++;
					}
					char str[totalDigits];
					sprintf(str, "%d", ascii);
					write(fd2,str,sizeof(str));
					write(fd2, "+", 1);		

				}else{
					int totalDigits = 0;
					int ascii = (int)buffer[i];
					int temp = ascii;
					while(temp!=0){
						temp = temp/10;
					    totalDigits ++;
					}
					char str[totalDigits];
					sprintf(str, "%d", ascii);
					write(fd2,str,sizeof(str));
					write(fd2, "i", 1);
				}
			}
			write(fd2, "\n", 1);
			usleep(1000 * time);

			lock.l_type = F_UNLCK;
    		fcntl(fd2,F_SETLKW,&lock);
			
		}else{
			break;
		}	

	
	}while(1);

	if ((fd2 = open(outputFile, O_WRONLY|O_CREAT |O_EXCL| O_APPEND,0777)) == -1) {
				fd2 = open(outputFile, O_WRONLY|O_EXCL| O_APPEND);
			} 
	lock.l_type = F_WRLCK;
	fcntl(fd2,F_SETLKW,&lock);
	char a[8] = "finished";
	write(fd2, a, 8);
	write(fd2, "\n", 1);
	usleep(1000 * time);
	lock.l_type = F_UNLCK;
	fcntl(fd2,F_SETLKW,&lock);
	
	write(1,"Process A is Finished\n",22);
	close(fd);
	close(fd2);
	return 0;
}