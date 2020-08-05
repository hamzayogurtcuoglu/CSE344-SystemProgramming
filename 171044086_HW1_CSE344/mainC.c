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

void inPlaceMergeSort ( double x[],int newLineNumber,int fd,int lineLeght[],char *inputFile);
void swapInFile(int locationLines[],double x[],int lineLeght[],int fd,int newLineNumber,char *inputFile);

int main(int argc, char *const* argv) { 
   

   /*Usage*/
	if(argc != 3){
		fprintf(stderr, "programC: Run as a single instance, and sort the lines in ascending order of the file denoted by inputPathC using mergesort. \n");
		fprintf(stderr, "Usage: %s -i inputPathC \n", argv[0]);
		return -1;
	}

	int opt;	
	char *inputFile ;

    while((opt = getopt(argc, argv, "i:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'i':  
                inputFile = optarg;  
                break;    
            case ':':  
                fprintf(stderr, "programC: Run as a single instance, and sort the lines in ascending order of the file denoted by inputPathC using mergesort. \n");
				fprintf(stderr, "Usage: %s -i inputPathC \n", argv[0]);
				return -1;
            case '?':  
                fprintf(stderr, "programC: Run as a single instance, and sort the lines in ascending order of the file denoted by inputPathC using mergesort. \n");
				fprintf(stderr, "Usage: %s -i inputPathC \n", argv[0]);
				return -1;
        }  
    }

    int fd;

	if ((fd = open(inputFile, O_RDWR,0777)) == -1) {
		perror("open");
		exit(1);
	}

	char  buffer[1];
	int newLineNumber = 0;
	while(read(fd,buffer,sizeof(buffer)) != 0)
		if (buffer[0] == '\n')
			newLineNumber++;

	lseek(fd,0,SEEK_SET);

	int lineLeght[newLineNumber];
	int indexLength = 0;
	int indexL = 0;
	while(read(fd,buffer,sizeof(buffer)) != 0){
		
		if (buffer[0] == '\n'){

			lineLeght[indexL] = indexLength;
			indexL++;
			indexLength = 0;
			continue;
		}
		indexLength++;
	}
	
	lseek(fd,0,SEEK_SET);
	
	double * lineMagnitude = (double *) malloc(sizeof(double)*newLineNumber);
	
	for (int i = 0; i < newLineNumber; ++i)
		lineMagnitude[0] = 0.0;

	for (int i = 0; i < newLineNumber ; ++i)
	{	
		char r[9],im[9];
		int rI = 0,iI=0;
		int check = 0;

	    for (int j = 0; j < lineLeght[i] ; ++j)
	    {
	    	read(fd,buffer,sizeof(buffer));
	    	if (buffer[0] == 'i')
	    	{

				double dig1=0.001,dig2 = 0.001;
				double a = 0,b = 0;
	    		for (int z = rI-1; z >=0 ; --z){
					a = a + (r[z]-'0')*dig1;
					dig1 *= 10;
				}    	

				for (int z = iI-1; z >=0 ; --z){
					b = b + (im[z]-'0')*dig2;
					dig2 *= 10;
				}
				a *= a;
				b *=b;
				lineMagnitude [i] += sqrt(a + b);
	    		rI = 0;
	    		iI = 0;
	    		check = 0;
	    		
	    	}
	    	else if (rI == 0 && buffer[0] == '-' && check == 0)
	    	{
	    	}else if (buffer[0] == '-' || buffer[0] == '+'){
	    		check = 1;
	    	}
	    	else if (check == 0 && buffer[0] != '.' && buffer[0] != ',' ){

	    		r[rI++] = buffer[0];
	    	}
	    	else if (check == 1 && buffer[0] != '.'){

	    		im[iI++] = buffer[0];
	    	}

	    }
	    read(fd,buffer,sizeof(buffer));
	}

    inPlaceMergeSort(lineMagnitude,newLineNumber,fd,lineLeght,inputFile);
	
	close(fd);
	free(lineMagnitude);
  	return 0;
}


void swapInFile(int locationLines[],double x[],int lineLeght[],int fd,int newLineNumber,char *inputFile){

	int index = 0;
	int currentLine = 0;
	int tempLocationLines[newLineNumber];
	for (int i = 0; i < newLineNumber; ++i)
   		tempLocationLines[i] = i;
	
	char * swapLine1;
	int j;
	

	int tempfd;

	if ((tempfd = open("tempHamza", O_RDWR|O_CREAT |O_EXCL| O_APPEND,0777)) == -1) {
		tempfd = open("tempHamza", O_RDWR|O_EXCL| O_APPEND);
	}
	printf("In-place sorting is finished\n");
	while(index != newLineNumber){

		int l1=0;
		lseek(fd,0,SEEK_SET);
		int temp = locationLines[currentLine];
		
		for (j = 0; temp != tempLocationLines[j]; ++j){
			l1 += lineLeght[j]+1; 
		}
		j = j + 1;
		lseek(fd,l1,SEEK_SET);
		swapLine1 = (char*)malloc(lineLeght[j-1]);
		read(fd,swapLine1,lineLeght[j-1]);
		swapLine1[lineLeght[j-1]] ='\0';
	

		write(tempfd,swapLine1,lineLeght[j-1]);
		write(tempfd,"\n",1);


		free(swapLine1);
		currentLine++;
		index++;	
	}
	close(fd);
	remove(inputFile);

	rename("tempHamza", inputFile);

}

void inPlaceMergeSort ( double x[],int newLineNumber,int fd,int lineLeght[],char *inputFile)
{
   
   int leftList1 = 0;
   int arrayLength = 0;
   int i, j, rightList1, leftList2, rightList2;
   double tempArray[newLineNumber];
   int locationLines[newLineNumber];
   int tempLocationLines[newLineNumber];

   for (int i = 0; i < newLineNumber; ++i)
   		locationLines[i] = i;

   lseek(fd,0,SEEK_SET);
	    
    for(int index=1; index < newLineNumber; index=index*2 )
    {
        while( leftList1+index < newLineNumber )
        {
            rightList1=leftList1+index-1;
            leftList2=rightList1+1;
            rightList2=leftList2+index-1;
            if( rightList2>=newLineNumber )
                rightList2=newLineNumber-1;
            i=leftList1;
            j=leftList2;
            
            while(i<=rightList1 && j<=rightList2 )
            {
                if( x[i] <= x[j] ){
                    
                    tempArray[arrayLength]=x[i];
                    tempLocationLines[arrayLength] = locationLines[i];
                    arrayLength++;
                    i++;
                }
                else{
                    
                    tempArray[arrayLength]=x[j];
                    tempLocationLines[arrayLength] = locationLines[j];
                    arrayLength++;
                    j++;
                }
                
            }
            while(i<=rightList1){
                
                tempArray[arrayLength]=x[i];
                tempLocationLines[arrayLength] = locationLines[i];
                arrayLength++;
                i++;}
            
            while(j<=rightList2){
                tempArray[arrayLength]=x[j];
                tempLocationLines[arrayLength] = locationLines[j];
                arrayLength++;
                j++;
            }            
            
            leftList1=rightList2+1; 
        }

        for(i=leftList1; arrayLength<newLineNumber; i++){ 
            tempArray[arrayLength]=x[i];
            tempLocationLines[arrayLength] = locationLines[i];
            arrayLength++;
        }

        for(i=0;i<newLineNumber;i++){
            x[i]=tempArray[i];   
        	locationLines[i] = tempLocationLines[i];
        }
        leftList1 = 0;
        arrayLength = 0;   
    }
    
    swapInFile(locationLines,x,lineLeght,fd,newLineNumber,inputFile);
}