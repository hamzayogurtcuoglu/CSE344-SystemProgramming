
#include "cooks.h"

void waitSemC(sem_t * sems,int index){
	int err;
	err = sem_wait(&sems[index]);
    if(err == -1){
        if(errno != EINTR)
            perror("sem_wait");
        exit(0);
	}
}
void postSemC(sem_t * sems,int index){
	int err;
	err = sem_post(&sems[index]);
    if(err == -1){
        if(errno != EINTR)
            perror("sem_post");
        exit(0);
	}
}

void insertQ(int * Queue,int * vars,int varsIndex)
{
   
    if (vars[varsIndex+1] == - 1){
        vars[varsIndex+1] = 0;
    }
    vars[varsIndex] = vars[varsIndex] + 1;
    Queue[vars[varsIndex]] = 1;

} 

void deleteQ(int * Queue,int * vars,int varsIndex)
{
    
	for (int i = 0; i < vars[varsIndex]; ++i)
	{
    	Queue[i] = Queue[i+1];
	}
    Queue[vars[varsIndex]] = 0;
    vars[varsIndex] = vars[varsIndex] - 1;


} 

void cooks(int id,sem_t * sems, int LxM,int * vars,int * soupQ,int * mainQ,int * desertQ){

	int totalCounter = 0;
	int totalKitchen = 0;
	do{
		waitSemC(sems,6);
		waitSemC(sems,8);
		postSemC(sems,6);

		if (vars[4] >= (3*LxM))
			break;

		waitSemC(sems,7);
		totalKitchen = vars[0] + vars[1] + vars[2];
		goingToKitchen(sems,id,vars[0],vars[1],vars[2],totalKitchen);
		postSemC(sems,0);
		postSemC(sems,7);

	    waitSemC(sems,12);
    	if ((vars[5]<=vars[6] && vars[5]<=vars[7]&&vars[8]) || 
    		((vars[5]<=vars[6] && vars[5]<=vars[7])&&((vars[0] + vars[1] + vars[2])>3)) 
    		||((vars[5]<=vars[6] && vars[5]<=vars[7])&&(vars[0] != 0)) )
    		
    	{
	    	postSemC(sems,12);
			waitSemC(sems,1);
			waitSemC(sems,4);

			waitSemC(sems,7);

			--vars[0];
			--vars[3];
			postSemC(sems,7);
			
			totalCounter = vars[5] +vars[6] + vars[7];
			goingToCounter(sems,id,"soup",4,vars[5],vars[6],vars[7],totalCounter);
			waitSemC(sems,5);

	    	waitSemC(sems,12);
			++vars[5];
			totalCounter = vars[5] +vars[6] + vars[7];
	    	postSemC(sems,12);

			
			insertQ(soupQ,vars,10);
			if (soupQ[0] == 1 &&mainQ[0] == 1&& desertQ[0] ==1)
			{

				deleteQ(soupQ,vars,10);
				deleteQ(mainQ,vars,12);
				deleteQ(desertQ,vars,14);
				
				waitSemC(sems,14);
				if (vars[17]>0)
				{
					postSemC(sems,15);
					--vars[17];
				}else{
					postSemC(sems,11);
				}
				postSemC(sems,14);

			}		
			deliveredCounter(sems,id,"soup",4,vars[5],vars[6],vars[7],totalCounter);
			++vars[4];


    	}else if((vars[6]<=vars[5] && vars[6]<=vars[7]&&vars[8])|| 
    			((vars[6]<=vars[5] && vars[6]<=vars[7]) &&((vars[0] + vars[1] + vars[2])>3))
    			|| ((vars[6]<=vars[5] && vars[6]<=vars[7])&&(vars[1] != 0)) 
    		){
	    	postSemC(sems,12);
			waitSemC(sems,2);	
			waitSemC(sems,4);
			waitSemC(sems,7);
    		
			--vars[1];
			--vars[3];
			postSemC(sems,7);
			

			totalCounter = vars[5] +vars[6] + vars[7];

			goingToCounter(sems,id,"main course",11,vars[5],vars[6],vars[7],totalCounter);

			waitSemC(sems,5);

	    	waitSemC(sems,12);
			++vars[6];
			totalCounter = vars[5] +vars[6] + vars[7];
	    	postSemC(sems,12);

		
			insertQ(mainQ,vars,12);
			if (soupQ[0] == 1 &&mainQ[0] == 1&& desertQ[0] ==1)
			{
				deleteQ(soupQ,vars,10);
				deleteQ(mainQ,vars,12);
				deleteQ(desertQ,vars,14);
				waitSemC(sems,14);
				if (vars[17]>0)
				{
					postSemC(sems,15);
					--vars[17];
				}else{
					postSemC(sems,11);
				}
				postSemC(sems,14);
			}

			deliveredCounter(sems,id,"main course",11,vars[5],vars[6],vars[7],totalCounter);
			++vars[4];

	    	

    	}else{
	    	
	    	postSemC(sems,12);
			waitSemC(sems,3);	
			waitSemC(sems,4);
			waitSemC(sems,7);

			--vars[2];
			--vars[3];
			postSemC(sems,7);
			
			totalCounter = vars[5] +vars[6] + vars[7];
			goingToCounter(sems,id,"desert",6,vars[5],vars[6],vars[7],totalCounter);
			waitSemC(sems,5);

			waitSemC(sems,12);
			++vars[7];
			totalCounter = vars[5] +vars[6] + vars[7];
	    	postSemC(sems,12);
	
			insertQ(desertQ,vars,14);
			if (soupQ[0] == 1 &&mainQ[0] == 1&& desertQ[0] ==1)
			{

				deleteQ(soupQ,vars,10);
				deleteQ(mainQ,vars,12);
				deleteQ(desertQ,vars,14);
				waitSemC(sems,14);
				if (vars[17]>0)
				{
					postSemC(sems,15);
					--vars[17];
				}else{
					postSemC(sems,11);
				}
				postSemC(sems,14);
			}		
			
			totalCounter = vars[5] +vars[6] + vars[7];
    		

			deliveredCounter(sems,id,"desert",6,vars[5],vars[6],vars[7],totalCounter);
			++vars[4];
    	}
	
		postSemC(sems,8);


	}while(vars[4] != (3*LxM) );
	
	postSemC(sems,6);
	postSemC(sems,8);
	goodByeCooker(sems ,id,(vars[4]-(3*LxM)));
	exit(0);
}

void goingToKitchen(sem_t *sems,int id,int P,int C,int D,int total){
   
   char str[10];
   int temp;
   waitSemC(sems,16);
   write(1,"Cook ",5);
   temp = sprintf(str, "%d", id);
   write(1,str,temp);
   write(1," is going to the kitchen to wait for/get a plate - kitchen items P:",67);
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
   postSemC(sems,16);

}


void goingToCounter(sem_t *sems,int id,char * item,int itemLen,int P,int C,int D,int total){
   
   char str[10];
   int temp;
   waitSemC(sems,16);
   write(1,"Cook ",5);
   temp = sprintf(str, "%d", id);
   write(1,str,temp);
   write(1," is going to counter to deliver ",32);
   write(1,item,itemLen);
   write(1," - counter items P:",19);
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
   postSemC(sems,16);

}



void deliveredCounter(sem_t * sems,int id,char * item,int itemLen,int P,int C,int D,int total){
   
   char str[10];
   int temp;
   waitSemC(sems,16);
   write(1,"Cook ",5);
   temp = sprintf(str, "%d", id);
   write(1,str,temp);
   write(1," placed ",8);
   write(1,item,itemLen);
   write(1," on the counter - items on counter P:",37);
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
   postSemC(sems,16);

}

void goodByeCooker(sem_t *sems,int id,int total){
   char str[10];
   int temp;
   waitSemC(sems,16);
   write(1,"Cook ",5);
   temp = sprintf(str, "%d", id);
   write(1,str,temp);
   write(1," finished serving - items at kitchen: ",38);
   temp = sprintf(str, "%d", total);
   write(1,str,temp);
   write(1," – going home – GOODBYE!!!",30);
   write(1,"\n",1);
   postSemC(sems,16);
}

