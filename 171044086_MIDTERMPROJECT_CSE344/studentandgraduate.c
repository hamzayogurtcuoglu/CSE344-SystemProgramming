
#include "studentandgraduate.h"

void waitSemSG(sem_t * sems,int index){
	int err;
	err = sem_wait(&sems[index]);
    if(err == -1){
        if(errno != EINTR)
            perror("sem_wait");
        exit(0);
	}
}
void postSemSG(sem_t * sems,int index){
	int err;
	err = sem_post(&sems[index]);
    if(err == -1){
        if(errno != EINTR)
            perror("sem_post");
        exit(0);
	}
}

void studentAndGraduate(sem_t * sems,int L,int id,int * vars,int gradOrStudent,int * tableNum,int T){

	int round = 0;
	int table;
	while(L--){
		
		round++;

		waitSemSG(sems,12);
		if (gradOrStudent == 1)
		{
			waitSemSG(sems,14);
			goingSCounter(sems,gradOrStudent,id,round,++vars[17],vars[5],vars[6],vars[7],vars[5]+vars[6]+vars[7]);
			postSemSG(sems,14);
			
		}else{
			waitSemSG(sems,13);
			goingSCounter(sems,gradOrStudent,id,round,++vars[16],vars[5],vars[6],vars[7],vars[5]+vars[6]+vars[7]);
			postSemSG(sems,13);
			
		}	
		postSemSG(sems,12);
		
		if (gradOrStudent == 1)
		{
			waitSemSG(sems,15);
			
		}else{
			waitSemSG(sems,11);
			waitSemSG(sems,13);
			--vars[16];
			postSemSG(sems,13);
		}
		
		waitSemSG(sems,12);
		--vars[5];
		--vars[6];
		--vars[7];
		
		postSemSG(sems,5);
		postSemSG(sems,5);
		postSemSG(sems,5);
		postSemSG(sems,12);

		gotFood(sems,gradOrStudent,id,round,vars[9]);

//////////////////////////////////////
		//tablee begin
		waitSemSG(sems,9);
		waitSemSG(sems,10);
		--vars[9];
		for (int i = 0; i < T ; ++i)
		{
			if (tableNum[i] == 0)
			{
				table = i + 1;
				tableNum[i] = 1;
				break;
			}

		}
		satTable(sems,gradOrStudent,id,table,round,vars[9]);		
		postSemSG(sems,10);
		waitSemSG(sems,10);
		++vars[9];
		tableNum[table-1] = 0;
		leftTable(sems,gradOrStudent,id,round,vars[9]);
		postSemSG(sems,10);
		postSemSG(sems,9);
		//tablee end

	}

	goodByeS(sems,gradOrStudent,id,round);
	
}

void goingSCounter(sem_t * sems,int gradOrStudent,int id,int round,int atCounter,int P,int C,int D,int total){

   char str[10];
   int temp;
   waitSemSG(sems,16);
   if (gradOrStudent == 1)
   {
   	 write(1,"Graduate Student ",17);
   }else{
   	 write(1,"Student ",8);
   }

   temp = sprintf(str, "%d", id);
   write(1,str,temp);
   write(1," is going to the counter (round ",32);

   temp = sprintf(str, "%d", round);
   write(1,str,temp);

   if (gradOrStudent == 1)
   {
   		write(1,") - # of graduate students at counter: ",39); 
   }else{
   		write(1,") - # of students at counter: ",30);
   }
   
   temp = sprintf(str, "%d", atCounter);
   write(1,str,temp);
   write(1," and counter items P:",21);
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
   postSemSG(sems,16);
}


void gotFood(sem_t * sems,int gradOrStudent,int id,int round,int emptyTable){

   char str[10];
   int temp;
   waitSemSG(sems,16);
   if (gradOrStudent == 1)
   {
   	 write(1,"Graduate Student ",17);
   }else{
   	 write(1,"Student ",8);
   }

   temp = sprintf(str, "%d", id);
   write(1,str,temp);
   write(1," got food and is going to get a table (round ",45);

   temp = sprintf(str, "%d", round);
   write(1,str,temp);
   write(1,") - # of empty tables: ",23);
   temp = sprintf(str, "%d", emptyTable);
   write(1,str,temp);
   write(1,"\n",1);
   postSemSG(sems,16);
}

void satTable(sem_t * sems,int gradOrStudent,int id,int tableNo,int round,int emptyTable){
   char str[10];
   int temp;
   waitSemSG(sems,16);
   if (gradOrStudent == 1)
   {
   	 write(1,"Graduate Student ",17);
   }else{
   	 write(1,"Student ",8);
   }

   temp = sprintf(str, "%d", id);
   write(1,str,temp);
   
   write(1," sat at table ",14);
   temp = sprintf(str, "%d", tableNo);
   write(1,str,temp);

   write(1," to eat (round ",15);
   temp = sprintf(str, "%d", round);
   write(1,str,temp);
   write(1,") - empty tables:",17);
   temp = sprintf(str, "%d", emptyTable);
   write(1,str,temp);
   write(1,"\n",1);
   postSemSG(sems,16);
}

void leftTable(sem_t * sems,int gradOrStudent,int id,int round,int emptyTable){
   char str[10];
   int temp;
   waitSemSG(sems,16);
   if (gradOrStudent == 1)
   {
   	 write(1,"Graduate Student ",17);
   }else{
   	 write(1,"Student ",8);
   }

   temp = sprintf(str, "%d", id);
   write(1,str,temp);
   
   write(1," left (round ",13);
   temp = sprintf(str, "%d", round);
   write(1,str,temp);
   write(1,") - # of empty tables: ",23);
   temp = sprintf(str, "%d", emptyTable);
   write(1,str,temp);
   write(1,"\n",1);
   postSemSG(sems,16);
}

void goodByeS(sem_t * sems,int gradOrStudent,int id,int round){
   
   char str[10];
   int temp;
   waitSemSG(sems,16);
   if (gradOrStudent == 1)
   {
   	 write(1,"Graduate Student ",17);
   }else{
   	 write(1,"Student ",8);
   }

   temp = sprintf(str, "%d", id);
   write(1,str,temp);
   
   write(1," is done eating L=",18);
   temp = sprintf(str, "%d", round);
   write(1,str,temp);

   write(1," times - going home - GOODBYE!!!",32);
   write(1,"\n",1);
   postSemSG(sems,16);
}