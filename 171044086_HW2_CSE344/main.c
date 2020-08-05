
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

volatile sig_atomic_t usr_interrupt = 1;
int termination = 1;

void synch_signal (int sig){
    usr_interrupt = 0;
}
void terminate_handler(int signum) {
    termination = 0;
}

double  MAECal(int *n,double x1,double b1);
double  MSECal(int *n,double x1,double b1);
double RMSECal(int *n,double x1,double b1);            
void handler(int sig) {}
void childMain(int fd, int fd3, char *outputFile,char * inputFile);
char * lineCheck(char *lineStart, char * lineEnd,char *line, int len);
void stophandler(int signum){}

int main(int argc, char *const* argv)
{

    char errorMessage[] = "Program contains two processes, the first process computes the least square method of"
                           "every 10 points. The second process calculates the mean absolute error (MAE), mean squared error (MSE)" 
                           "and root mean squared error (RMSE) between the coordinates and the estimated line.";

    /*Usage*/
    if(argc != 5){
        fprintf(stderr, "program: %s \n",errorMessage);
        fprintf(stderr, "Usage: %s -i inputPath -o outputPath \n", argv[0]);
        return -1;
    }

    int opt;
    char *inputFile;
    char *outputFile;      
    
    while((opt = getopt(argc, argv, "t:i:o:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'i':  
                inputFile = optarg;  
                break;
            case 'o':  
                outputFile = optarg;  
                break;
            case ':':  
                fprintf(stderr, "program: %s \n",errorMessage);
                fprintf(stderr, "Usage: %s -i inputPath -o outputPath \n", argv[0]);
                return -1;
            case '?':  
                fprintf(stderr, "program: %s \n",errorMessage);
                fprintf(stderr, "Usage: %s -i inputPath -o outputPath \n", argv[0]);
                return -1;
        }  
    }

    int fd,fd2;
    if ((fd2 = open(inputFile, O_RDONLY)) == -1) {
        perror("InputFile : ");
        exit(1);
    }

    char template[] = "tempfileXXXXXX";
    char fname[14];
    
    strcpy(fname, template);        /* Copy template */
    fd = mkstemp(fname);            /* Create and open temp file */
    struct sigaction usr_action;
    sigset_t block_mask;
    sigfillset (&block_mask);

    usr_action.sa_handler = synch_signal;
    usr_action.sa_mask = block_mask;
    usr_action.sa_flags = 0;

    if(sigaction (SIGUSR1, &usr_action, NULL) == -1){
        perror("Failed to install SIGUSR1 signal handler");
        return 1;
    }

    signal(SIGUSR2, handler);
    struct sigaction act;
    sigset_t old_mask;
    signal(SIGTSTP, SIG_IGN);
    memset (&act, 0, sizeof(act));
    act.sa_handler = terminate_handler;
 
    if (sigaction(SIGTERM, &act, 0)) {
        perror("Failed to install SIGTERM signal handler");
        return 1;
    }

    sigset_t base_mask, waiting_mask;
    sigemptyset (&base_mask);
    sigaddset (&base_mask, SIGINT);
    sigaddset (&base_mask, SIGTSTP);
    sigprocmask (SIG_BLOCK, &base_mask, &old_mask);

    pid_t child_id = fork();

    if (child_id == 0)
        childMain(fd,fd2,outputFile,inputFile);   
    
    char buffer[20]; 
    int byte32_read;
    double a,b;
    double xTotal=0,x2Total=0,yTotal=0,xyTotal=0; 
    
    struct flock lock;
    memset(&lock,0,sizeof(lock));

    int readByte = 0;    
    int estimatedLine = 0;


    do{
        byte32_read = read(fd2,buffer,sizeof(buffer));
        if (byte32_read == 20){
            
            xTotal=0,x2Total=0,yTotal=0,xyTotal=0;            
            
            lock.l_type = F_WRLCK;
            fcntl(fd,F_SETLKW,&lock);
            lseek(fd,0,SEEK_END);


            for (int i = 0; i < 20; ++i)
            {   
                write(fd, "(", 1);
                char str[3];
                int temp = sprintf(str, "%d",(unsigned char)buffer[i]);
                write(fd,str,temp);
                write(fd, ",", 1);
                temp = sprintf(str, "%d",(unsigned char)buffer[i+1]);
                write(fd,str,temp);
                write(fd, ")", 1);
                write(fd, ",", 1);

                xTotal=xTotal+(int)((unsigned char)buffer[i]);                        
                yTotal=yTotal+(int)((unsigned char)buffer[i+1]);                        
                x2Total=x2Total+pow((int)((unsigned char)buffer[i]),2);                
                xyTotal=xyTotal+((int)buffer[i])*((int) ((unsigned char)buffer[i+1]));                  
                i++;
            }
            a=(10*xyTotal-xTotal*yTotal)/(10*x2Total-xTotal*xTotal);           
            b=(x2Total*yTotal-xTotal*xyTotal)/(x2Total*10-xTotal*xTotal);            
            char buffer2[11];
            int res = sprintf(buffer2, "%.3f", a);
            buffer2[res] = '\0';
            write(fd,buffer2,res);
            write(fd, "x", 1);
            res = sprintf(buffer2, "%.3f", b);
            buffer2[res] = '\0';
            
            if (b>=0)
                write(fd, "+", 1);

            write(fd,buffer2,res);
            write(fd,"\n",1);
            lock.l_type = F_UNLCK;
            readByte += 20;
            estimatedLine++;
            fcntl(fd,F_SETLKW,&lock);
            kill(child_id,SIGUSR2);
                        
        }else{
            break;
        }

    }while(termination != 0);
    
    kill(child_id,SIGUSR2);

    if (termination == 0)
        kill(child_id,SIGTERM);
    
    kill (child_id, SIGUSR1);
    close(fd2);
    wait(NULL); //Waiting the child process

    close(fd);              /* Close file */
    unlink(fname);              /* Remove it */
    remove(fname);
    remove(inputFile);
    
    if (termination == 1)
    {
        char processTitle[] = "----------------- P1 -----------------\n";
        char str[3];
        int temp = sprintf(str, "%d", readByte);

        write(1,processTitle,strlen(processTitle));
        write(1,"Number of bytes read : ",23);
        write(1,str,temp);
        write(1,"\n",1);
        
        temp = sprintf(str, "%d", estimatedLine);
        write(1,"Estiminated line : ",19);
        write(1,str,temp);
        write(1,"\n",1);

        sigpending (&waiting_mask);
        write(1,"Signals where sent to P1 : ",27);
        int check = 0;
        if (sigismember (&waiting_mask, SIGINT) == 1) {
            write(1,"SIGINT ",8);
            check = 1;
        }
        if (sigismember (&waiting_mask, SIGTSTP) == 1) {
            write(1,"SIGSTOP \n",9);
            check = 1;
        }
        if (!check)
            write(1,"No Signal\n",10);
        write(1,"\n",1);
    } 

    if (termination == 0)
        write(1,"\nSIGTERM signal is caught.\n",27);

    write(1,"Closed open files, and removing the input and temporary files from disk. \n",75);

}

void childMain(int fd, int fd3, char *outputFile,char * inputFile){
    
    fprintf (stdout, "\nP1 process id = %d "
       "P2 process id = %d\n",
       (int) getppid(),(int) getpid()); 
    int fd2;
    if ((fd2 = open(outputFile, O_RDWR|O_CREAT |O_EXCL| O_APPEND,0777)) == -1) {
        fd2 = open(outputFile, O_RDWR|O_EXCL| O_APPEND);
    }  
    char *start, *end;
    
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    
    sigset_t mask;
    sigemptyset(&mask);
      

    sigset_t base_mask;
    sigemptyset (&base_mask);
    sigaddset (&base_mask, SIGINT);
    sigaddset (&base_mask, SIGSTOP);
    signal(SIGINT, stophandler);
    char  buffer[1];
    
    char *line;
    char *allbuf;

    int i = 0; 
    int k = 0;
    
    double meanMAE = 0.0;
    double meanMSE = 0.0;
    double meanRMSE = 0.0;
    int allbyte;
    do{

        lock.l_type = F_WRLCK;
        fcntl(fd,F_SETLKW,&lock);

        allbyte = lseek(fd,0,SEEK_END);
        allbuf = (char*)malloc(sizeof(char)*allbyte+1);
        lseek(fd,0,SEEK_SET);
        read(fd,allbuf,allbyte);
        allbuf[allbyte] = '\0';
        lseek(fd,0,SEEK_SET);

        i = 0;
        do
        {
            read(fd,buffer,1);      
            i++;
        }while(*buffer !=' ' && *buffer != '\n' && *buffer != '\0');


        if (i>2)
        {

            line = (char*)malloc(sizeof(char)*i+1);            
            
            lseek(fd,-i,SEEK_CUR);
            read(fd,line,i);        
            line[i-1] = '\0';        
            double x1,b1;
            int n[20];
            sscanf (line,"(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),%lfx%lf"
                ,&n[0],&n[1],&n[2],&n[3],&n[4],&n[5],&n[6],&n[7],&n[8],&n[9],&n[10],
                 &n[11],&n[12],&n[13],&n[14],&n[15],&n[16],&n[17],&n[18],&n[19],&x1,&b1);
            
            double MAE = MAECal(n,x1,b1);
            double MSE = MSECal(n,x1,b1);
            double RMSE = RMSECal(n,x1,b1);
            
            meanMAE += MAE;
            meanMSE += MSE;
            meanRMSE += RMSE;

            write(fd2,line,i-1);
            
            write(fd2,", ",2);
            char buffer2[11];
            int res = sprintf(buffer2, "%.3f", MAE);
            buffer2[res] = '\0';
            write(fd2,buffer2,res);
            write(fd2,", ",2);
            
            res = sprintf(buffer2, "%.3f", MSE);
            buffer2[res] = '\0';
            write(fd2,buffer2,res);
            write(fd2,", ",2);

            res = sprintf(buffer2, "%.3f", RMSE);
            buffer2[res] = '\0';
            write(fd2,buffer2,res);
            write(fd2,"\n",1);
                  
            int t = lseek(fd,0,SEEK_CUR);
            start = allbuf;
            end = lineCheck(start, allbuf + allbyte, line, sizeof(line));
            
            ftruncate(fd, 0);
            lseek(fd, 0, 0);
            
            write(fd, start, end - start);
            write(fd,(allbuf+t),allbyte-t);
            k++;
            free(line);
        }

        free(allbuf);

        lock.l_type = F_UNLCK;
        fcntl(fd,F_SETLKW,&lock);
             if (usr_interrupt == 1)
            sigsuspend(&mask);
    }while ((usr_interrupt || i > 1)&& termination == 1);
    
    if (termination == 0)
    {
        kill(getppid(),SIGTERM);
    }

    if (termination == 1)
    {   
        char processTitle[] = "\n----------------- P2 -----------------\n";
        write(1,processTitle,strlen(processTitle));

        meanMAE /= k;
        meanMSE /= k;
        meanRMSE /= k;

        char buffer2[11];
        int res = sprintf(buffer2, "%.3f", meanMAE);
        buffer2[res] = '\0';
        write(1,"Mean of MAE : ",14);
        write(1,buffer2,res);
        write(1,"\n",1);

        res = sprintf(buffer2, "%.3f", meanMSE);
        buffer2[res] = '\0';
        write(1,"Mean of MSE : ",14);
        write(1,buffer2,res);
        write(1,"\n",1);

        res = sprintf(buffer2, "%.3f", meanRMSE);
        buffer2[res] = '\0';
        write(1,"Mean of RMSE : ",14);
        write(1,buffer2,res);
        write(1,"\n",1);

        lseek(fd2,0,SEEK_SET);
        double sdMAE = 0.0,sdMSE = 0.0,sdRMSE = 0.0;
        int n[20];
        for (int u = 0; u < k; ++u)
        {
            i = 0;
            do
            {
                read(fd2,buffer,1);      
                i++;
            }while(*buffer != '\n' && *buffer != '\0');

            line = (char*)malloc(sizeof(char)*i+1);            
            
            lseek(fd2,-i,SEEK_CUR);
            read(fd2,line,i);
            line[i-1] = '\0';        

            double x1,b1,e1,e2,e3;
            
            sscanf (line,"(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),(%d,%d),%lfx%lf, %lf, %lf, %lf"
                ,&n[0],&n[1],&n[2],&n[3],&n[4],&n[5],&n[6],&n[7],&n[8],&n[9],&n[10],
                 &n[11],&n[12],&n[13],&n[14],&n[15],&n[16],&n[17],&n[18],&n[19],&x1,&b1,&e1,&e2,&e3);

            sdMAE += pow(e1-meanMAE,2);
            sdMSE += pow(e2-meanMSE,2);
            sdRMSE += pow(e3-meanRMSE,2);
            free(line);
        }
        sdMAE /= k;
        sdMSE /= k;
        sdRMSE /= k;

        sdMAE = sqrt(sdMAE);
        sdMSE = sqrt(sdMSE);
        sdRMSE = sqrt(sdRMSE);
        
        res = sprintf(buffer2, "%.3f", sdMAE);
        buffer2[res] = '\0';
        write(1,"Standard Deviation of MAE : ",28);
        write(1,buffer2,res);
        write(1,"\n",1);

        res = sprintf(buffer2, "%.3f", sdMSE);
        buffer2[res] = '\0';
        write(1,"Standard Deviation of MSE : ",28);
        write(1,buffer2,res);
        write(1,"\n",1);

        res = sprintf(buffer2, "%.3f", sdRMSE);
        buffer2[res] = '\0';
        write(1,"Standard Deviation of RMSE : ",29);
        write(1,buffer2,res);
        write(1,"\n",1);
    }

    unlink(inputFile);
    remove(inputFile);
    close(fd3);
    close(fd2);
    close(fd);
    exit(0);
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

double  MAECal(int *n,double x1,double b1){
    double Total = 0.0;
    for (int i = 0; i < 20; ++i)
    {
        double y2 = x1*n[i]+b1;
        ++i;
        Total += abs(n[i]-y2);
    }
    return Total/10;
}

double  MSECal(int *n,double x1,double b1){
    double Total = 0.0;
    for (int i = 0; i < 20; ++i)
    {
        double y2 = x1*n[i]+b1;
        ++i;
        Total += pow(n[i]-y2,2);
    }

    return Total/10;
}

double RMSECal(int *n,double x1,double b1){
    double Total = 0.0;
    for (int i = 0; i < 20; ++i)
    {
        double y2 = x1*n[i]+b1;
        ++i;
        Total += pow(n[i]-y2,2);
    }
    Total = sqrt(Total);

    return Total/10;
}