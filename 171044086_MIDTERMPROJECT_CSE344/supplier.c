#include "supplier.h"

void waitSemS(sem_t * sems,int index){
	int err;
	err = sem_wait(&sems[index]);
    if(err == -1){
        if(errno != EINTR)
            perror("sem_wait");
        exit(0);
	}
}
void postSemS(sem_t * sems,int index){
	int err;
	err = sem_post(&sems[index]);
    if(err == -1){
        if(errno != EINTR)
            perror("sem_post");
        exit(0);
	}
}
int fd;

void supplierhandler(){
	close(fd);
	exit(0);
}


void supplier(sem_t * sems, int LxM,int K,int * vars,char * filePath){
	signal(SIGINT, supplierhandler);
	int totalPlate = 3*LxM;
	int soupN = 0,mainN = 0,desertN = 0;
	char buffer[1];
	if ((fd = open(filePath, O_RDONLY)) == -1) {
		perror("open");
		exit(1);
	}   

	while((totalPlate)--){
		
		read(fd,buffer,1);    
	    switch(buffer[0]){
	    	
	    	case SOUP:
	    		if (soupN != LxM)
	    		{

	    			waitSemS(sems,7);
	    			goingDeliver(sems,"soup",4,vars[0],vars[1],vars[2],vars[3]);
					postSemS(sems,7);
		    		waitSemS(sems,0);
					waitSemS(sems,7);
					++vars[3];
					postSemS(sems,4);
					++vars[0];
				    postSemS(sems,1);
				    deliveredKitchen(sems,"soup",4,vars[0],vars[1],vars[2],vars[3]);
					postSemS(sems,7);
					soupN++;
	    		}else{
	    			totalPlate++;
	    		}
	    		break;
	    	case MAIN_COURSE:
				
				if (mainN != LxM)
				{

	    			waitSemS(sems,7);
	    			goingDeliver(sems,"main course",11,vars[0],vars[1],vars[2],vars[3]);
					postSemS(sems,7);
		    		waitSemS(sems,0);
		    	
					waitSemS(sems,7);
					++vars[3];
					postSemS(sems,4);
					++vars[1];
				    postSemS(sems,2);
				    deliveredKitchen(sems,"main course",11,vars[0],vars[1],vars[2],vars[3]);
					postSemS(sems,7);
					mainN++;	
				}else{
					totalPlate++;
				}

	    		break;
	    	case DESERT:

		    	if (desertN != LxM)
		    	{
		    		waitSemS(sems,7);
	    			goingDeliver(sems,"desert",6,vars[0],vars[1],vars[2],vars[3]);
					postSemS(sems,7);
		    		waitSemS(sems,0);
		    	
					waitSemS(sems,7);
					++vars[3];
					postSemS(sems,4);
					++vars[2];
				    postSemS(sems,3);
				    deliveredKitchen(sems,"desert",6,vars[0],vars[1],vars[2],vars[3]);
					postSemS(sems,7);
					desertN++;
		    	}else{
		    		totalPlate++;
		    	}
	    		break;
	    }
	}
	vars[8] = 0;
    waitSemS(sems,16);
    write(1,"Supplier finished supplying – GOODBYE!\n",39);
	postSemS(sems,16);
	exit(0);

}


void goingDeliver(sem_t* sems,char * item,int itemLen,int P,int C,int D,int total){
   
   char str[10];
   int temp;
   waitSemS(sems,16);
   write(1,"The supplier is going to the kitchen to deliver ",48);
   write(1,item,itemLen);
   write(1," : kitchen items P:",19);
   temp = sprintf(str, "%d", P);
   write(1,str,temp);
   write(1,",C:",3);
   temp = sprintf(str, "%d", C);
   write(1,str,temp);
   write(1,",D:",3);
   temp = sprintf(str, "%d", D);
   write(1,str,temp);
   write(1,"=",1);
   temp = sprintf(str, "%d", total);   
   write(1,str,temp);
   write(1,"\n",1);
   postSemS(sems,16);

}


void deliveredKitchen(sem_t* sems,char * item,int itemLen,int P,int C,int D,int total){
   
   char str[10];
   int temp;
   waitSemS(sems,16);
   write(1,"The supplier delivered ",23);
   write(1,item,itemLen);
   write(1," - after delivery: kitchen items P:",35);
   temp = sprintf(str, "%d", P);
   write(1,str,temp);
   write(1,",C:",3);
   temp = sprintf(str, "%d", C);
   write(1,str,temp);
   write(1,",D:",3);
   temp = sprintf(str, "%d", D);
   write(1,str,temp);
   write(1,"=",1);
   temp = sprintf(str, "%d", total);   
   write(1,str,temp);
   write(1,"\n",1);
   postSemS(sems,16);

}