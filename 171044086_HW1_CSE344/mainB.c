
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

void dftOperation(char *line,int fd2,struct flock lock2);
char * lineCheck(char *lineStart, char * lineEnd,char *line, int len);

int main(int argc, char *const* argv)
{

	/*Usage*/
	if(argc != 7){
		fprintf(stderr, "programB: Reads the characters from file since their ASCII equivalent then that will be convert to complex number before write the a common file. \n");
		fprintf(stderr, "Usage: %s -i inputPathB -o outputPathB -t time \n", argv[0]);
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
            		fprintf(stderr, "programB: Calculating the discrete Fourier transform \n");
				    fprintf(stderr, "Usage: %s -i inputPathB -o outputPathB -t time \n", argv[0]);
				    return -1;
            	}

                break;
            case 'o':  
                outputFile = optarg;  
                break;
            case ':':  
                fprintf(stderr, "programB: Calculating the discrete Fourier transform \n");
				fprintf(stderr, "Usage: %s -i inputPathB -o outputPathB -t time \n", argv[0]);
				return -1;
            case '?':  
                fprintf(stderr, "programB: Calculating the discrete Fourier transform \n");
				fprintf(stderr, "Usage: %s -i inputPathB -o outputPathB -t time \n", argv[0]);
				return -1;
        }  
    } 
    int fd,fd2;
    struct flock lock,lock2;
    memset(&lock,0,sizeof(lock));
    memset(&lock2,0,sizeof(lock2));   

    if ((fd = open(inputFile, O_RDWR|O_CREAT |O_EXCL| O_APPEND,0777)) == -1) {
		fd = open(inputFile, O_RDWR|O_EXCL| O_APPEND);
	}

	if ((fd2 = open(outputFile, O_WRONLY|O_CREAT |O_EXCL| O_APPEND,0777)) == -1) {
		fd2 = open(outputFile, O_WRONLY|O_EXCL| O_APPEND);
	}  
	char  buffer[1];
	char *start, *end;
	
	do{	
		lock.l_type = F_WRLCK;
	    fcntl(fd,F_SETLKW,&lock);
		int allbyte = lseek(fd,0,SEEK_END);
		char *allbuf = (char*)malloc(sizeof(char)*allbyte+1);
		lseek(fd,0,SEEK_SET);
		read(fd,allbuf,allbyte);
		allbuf[allbyte] = '\0';
		lseek(fd,0,SEEK_SET);
		
		int i = 0;
		char *line;
		do
		{
			read(fd,buffer,1);		
			i++;
		}while(*buffer !=' ' && *buffer != '\n' && *buffer != '\0');
		if (i > 2 )
		{	
			if (i == 6   )
			{	
				lseek(fd,0,SEEK_SET);
				start = allbuf;
				end = lineCheck(start, allbuf + allbyte, "finished", sizeof("finished"));
				ftruncate(fd, 0);
				lseek(fd, 0, 0);	
				write(fd, start, end - start);
				break;
			}

			if (i != 9)	{

				line = (char*)malloc(sizeof(char)*i+1);
				lseek(fd,-i,SEEK_CUR);
				read(fd,line,i);		
				line[i-1] = '\0';
				
			}else{
				i = 0;
				do{
					read(fd,buffer,1);		
					i++;
				}while(*buffer !=' ' && *buffer != '\n' && *buffer != '\0');
				int tempIndex;
				int tempIndex2 = i;

				if (i != 9)
				{
					free(line);
					line = (char*)malloc(sizeof(char)*i+1);
					tempIndex = lseek(fd,-i,SEEK_CUR);
					read(fd,line,i);		
					line[i-1] = '\0';
				}
				else {
					lseek(fd,tempIndex,SEEK_SET);
					start = allbuf;
					end = lineCheck(start, allbuf + allbyte, "finished", sizeof("finished"));
					ftruncate(fd, 0);
					lseek(fd, 0, 0);	
					write(fd, start, end - start);

					write(fd, "bitti", 5);
					write(fd, "\n", 1);
					usleep(1000 * time);
					lock.l_type = F_UNLCK;
			    	fcntl(fd,F_SETLKW,&lock);
					break;
				}
				allbyte = lseek(fd,0,SEEK_END);
				allbuf = (char *) realloc(allbuf, allbyte+9);
	   			allbuf[allbyte] = 'f';
	   			allbuf[allbyte+1] = 'i';
	   			allbuf[allbyte+2] = 'n';
	   			allbuf[allbyte+3] = 'i';
	   			allbuf[allbyte+4] = 's';
	   			allbuf[allbyte+5] = 'h';
	   			allbuf[allbyte+6] = 'e';
	   			allbuf[allbyte+7] = 'd';
	   			allbuf[allbyte+8] = '\n';
	   			
	   			allbyte += 9; 
				int t = lseek(fd,tempIndex,SEEK_SET);
				start = allbuf;
				end = lineCheck(start, allbuf + allbyte, "finished", sizeof("finished"));
				ftruncate(fd, 0);
				lseek(fd, 0, 0);	
				write(fd, start, end - start);
				write(fd,(allbuf+t),allbyte-t);
				free(allbuf);
				allbuf = (char*)malloc(sizeof(char)*allbyte+1);
			    lseek(fd,0,SEEK_SET);
			    read(fd,allbuf,allbyte);
				lseek(fd,tempIndex2,SEEK_SET);

			}

			int t = lseek(fd,0,SEEK_CUR);
			start = allbuf;
			end = lineCheck(start, allbuf + allbyte, line, sizeof(line));
			
			ftruncate(fd, 0);
			lseek(fd, 0, 0);
			
			write(fd, start, end - start);
			write(fd,(allbuf+t),allbyte-t);

			free(allbuf);
			allbyte = lseek(fd,0,SEEK_END);
			lseek(fd,0,SEEK_SET);	
			allbuf = (char*)malloc(sizeof(char)*allbyte+1);
			usleep(1000 * time);
			lock.l_type = F_UNLCK;
	    	fcntl(fd,F_SETLKW,&lock);
	    	dftOperation(line,fd2,lock2);
	    	free(line);	

		}else{
			lock.l_type = F_UNLCK;
	    	fcntl(fd,F_SETLKW,&lock);
		}

	}while(1);
	write(1,"Process B is Finished\n",22);

	close(fd);
	close(fd2);
    return 0;
}

void dftOperation(char *line,int fd2,struct flock lock2) {
	char *token;
	double inputReal[16],outputReal[16],
		   inputImag[16],outputImag[16];

    int index = 0;

    token = strtok(line, ",");
    while( token != NULL ) {
		char r[3],im[3];
		int rI = 0,iI=0;
		int check = 0;
		for (int i = 0; i < sizeof(token); ++i)
		{
			if (check == 0)
			{
				if (token[i] != '+')
					r[rI++] = token[i];  
				else
					check = 1;
			}else{
				
				if (token[i] != 'i')
					im[iI++] = token[i];
				else
					break;
			}
		}
		int a=0,b=0;
		int dig1=1,dig2 = 1;
		for (int i = rI-1; i >=0 ; --i){
			a = a + (r[i]-'0')*dig1;
			dig1 *= 10;
		}    	

		for (int i = iI-1; i >=0 ; --i){
			b = b + (im[i]-'0')*dig2;
			dig2 *= 10;
		}
		inputReal[index] = a;
		inputImag[index] = b;
    	token = strtok(NULL, ",");
    	index++;
    }
    
   for (int k = 0; k < index; k++) {  
        double sumreal = 0;
        double sumimag = 0;
        for (int t = 0; t < index; t++) {  
            double angle = 2 * M_PI * t * k / index;
            sumreal +=  inputReal[t] * cos(angle) + inputImag[t] * sin(angle);
            sumimag += -inputReal[t] * sin(angle) + inputImag[t] * cos(angle);
        }
        outputReal[k] = sumreal;
        outputImag[k] = sumimag;
    }

    lock2.l_type = F_WRLCK;
	fcntl(fd2,F_SETLKW,&lock2);
	int res;
	for (int i = 0; i < 16; ++i)
	{	char    buffer[11];
		res = sprintf(buffer, "%.3f", outputReal[i]);
		buffer[res] = '\0';
		write(fd2,buffer,res);
		if (outputImag[i] >= 0 )
			write(fd2,"+",1);
			
		char buffer2[11];
		res = sprintf(buffer2, "%.3f", outputImag[i]);
		buffer2[res] = '\0';
		
		write(fd2,buffer2,res);
		write(fd2,"i",1);
		if (i != 15)
			write(fd2,",",1);				
	}
	write(fd2,"\n",1);
	lock2.l_type = F_UNLCK;
	fcntl(fd2,F_SETLKW,&lock2);
}

char * lineCheck(char *lineStart,char * lineEnd,char *line, int len)
{
	char *start = lineStart;
	int i = 0;
	while (start + len < lineEnd) {
		for (i = 0; i < len; i++)
			if (start[i] != line[i]) 
				break;
		if (i == len) 
			return (char *)start;
		start++;
	}
	return 0;
}