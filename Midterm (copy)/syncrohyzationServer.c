/**
* Written by Kasım Süzen at 14/04/2015
* This is midterm project server for CSE 244 class at GTU
*/

int operation(char * fifoName,int operationNumber);
int Operation1(char * fifoName);
int Operation2(char * fifoName);
int Operation3(char * fifoName);
int Operation4(char * fifoName);

int listener(char * nameOfFifo);

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>  
#include <sys/stat.h>

int main(int argc,char ** argv){
	listener(argv[1]);
	return 0;
}

int listener(char * nameOfFifo){
	int mainFifo,i=0,flag=0,operationNumber,fifofds[12],pipes[12][2],pids=0,status;
	char message[120],newFifoName[25];
	struct stat st;
	pid_t childpid[12],pid;

	if(nameOfFifo[0] == '-'){
		for(i=0;i < strlen(nameOfFifo) - 1;++i)
			nameOfFifo[i]=nameOfFifo[i+1];
	}
	nameOfFifo[i]='\0';

	if (stat(nameOfFifo, &st) != 0)
        mkfifo(nameOfFifo, 0777);
   
   fprintf(stderr,"debug1 %s\n",nameOfFifo);

   	mainFifo = open(nameOfFifo, O_RDONLY);

   	i=0;
    while(1){
    	fprintf(stderr,"debug2\n");
		while (flag == 0)
		{
    		fprintf(stderr,"debug3\n");
		    if (read(mainFifo, &message[i] ,1) != 0)
		    {
    			fprintf(stderr,"debug4 %c\n",message[i]);

		        if(message[i] == '\n'){
		    	/*Checks the messages for is this for server*/
    			
    			fprintf(stderr,"debug5 \n");

		        if(strncmp(message,"Server ",strlen("Server ")) == 0){
		        	flag = 1;
		        	message[i+1] ='\0';
    				fprintf(stderr,"debug5 %s\n",message);

		        	break;
		        }

		        /*If this message not for server writes messages back to fifo*/
		        else{
		        	close(mainFifo);
		        	mainFifo = open(nameOfFifo,O_WRONLY);
		        	message[i+1] ='\0';

		        	write(mainFifo,message,strlen(message));

		        	close(mainFifo);
		        	mainFifo = open(nameOfFifo,O_RDONLY);
		        }
		        
		        i = -1;
		    	
		    	}
		    }
		    else{
		    	fprintf(stderr,"else den çıktı\n");
		    	sleep(5);
		        break;
		    }
		    ++i;
		    fprintf(stderr,"%d\n",i);
		}
		/*If the message coming from aclient waiting for math server*/
		if(flag == 1){
			operationNumber = atoi(&message[strlen("Server ")]);
			strcpy(newFifoName,&message[strlen("Server ")+2]);
			newFifoName[strlen(newFifoName)-1]='\0';
			strcat(newFifoName,"_fifo\n");

			fprintf(stderr,"fifo name%sg%d\n",newFifoName,operationNumber);
			sleep(1);

			close(mainFifo);
			mainFifo = open(nameOfFifo,O_WRONLY);

			write(mainFifo,newFifoName,strlen(newFifoName));
			newFifoName[strlen(newFifoName)-1]='\0';			

			if((childpid[pids] = fork()) == -1)
		    {
		        perror("fork");
		        exit(1);
		    }

			if(childpid[pids] == 0)
		    {
		    	operation(newFifoName,operationNumber);
		        exit(0);
		    }
		   	else
		   		++pids;

			memset(message,'\0',50);
			memset(newFifoName,'\0',25);

		    close(mainFifo);
		    mainFifo = open(nameOfFifo,O_RDONLY);
		}
		flag =0;
		sleep(1);
		i=0;
		memset(message,'\0',120);

	}
	for(i=0; i < pids; ++i)
		pid = wait(&status);
	unlink(nameOfFifo);
}

/**
* This function choose which operation will be run and return is they killed by signal or user
* return -1 on error, -2 on killing by signal, 1 on kill by client
*/
int operation(char * fifoName,int operationNumber){
	int status;

	fprintf(stderr,"operat %s %d\n",fifoName,operationNumber);

	if(operationNumber == 1)
		status=Operation1(fifoName);
	else if(operationNumber == 2)
		status=Operation2(fifoName);
	else if(operationNumber == 3)
		status=Operation3(fifoName);
	else if(operationNumber == 4)
		status=Operation4(fifoName);
	else{
		fprintf(stderr,"Wrong operation number %d\n",operationNumber);
	
	return -1;		
	}

	return status;
}

/**
* This function make predefined operation as ((a^2 +b^2)^1/2)/abs(c)
* Return: returns 0 incase success -1 for divide by zero
*/
int Operation1(char * fifoName){
	double result;
	int variable1,variable2,variable3,fifoToClient,waitTime,i=0,space,flag=0;
	char message[120];

	fprintf(stderr,"q%sq\n",fifoName);

    fifoToClient = open(fifoName,O_RDONLY);

    if(fifoToClient == -1)
    	perror("Error at opening Operation1: ");

    while(flag == 0){
		while(1){

		    if (read(fifoToClient, &message[i] ,1) != 0)
		    {
		        printf(" %c",message[i]);
		    }
		    else
		        break;

		    if(message[i] == '\n'){
		    	/*Checks the messages for is this for server*/
		        if(strncmp(message,"Math ",strlen("Math ")) == 0){
		        	message[i] ='\0';
		        	fprintf(stderr,"doğru geldi -%s-\n",message );
		        	if(strncmp(message,"kill",strlen("kill")) == 0){
		        		fprintf(stderr,"%s\n",message );
		        		sleep(2);
		        		flag = 1;
		        	}
		        	sleep(2);
		        	break;
		        }

		        /*If this message not for Math server writes messages back to fifo*/
		        else{
		        	close(fifoToClient);
		        	fifoToClient = open(fifoName,O_WRONLY);
		        	message[i+1] ='\0';

		        	write(fifoToClient,message,strlen(message));

		        	close(fifoToClient);
		        	fifoToClient = open(fifoName,O_RDONLY);
		        }
		        i = -1;
		    }
		    ++i;
		}
		
		if(flag == 0){
			/*If the message coming from a client waiting result from math server*/
			waitTime = atoi(&message[strlen("Math -")]);
			fprintf(stderr,"wait time %d\n",waitTime);
			
			for(i=0; message[i] != '\0';++i){
				if(message[i] == ' ')
					++space;
				if(space == 1)
					variable1 = atoi(&message[i+2]);
				if(space == 2)
					variable2 = atoi(&message[i+2]);
				if(space == 3)
					variable3 = atoi(&message[i+2]);
			}

			fprintf(stderr,"-%d %d %d %d-\n",i,variable1,variable2,variable3);

			close(fifoToClient);
			fifoToClient = open(fifoName,O_WRONLY);

			if(variable3 == 0){
				write(fifoToClient,"Divison by zero\n",strlen("Division by zero\n"));
				continue;
			}

			fprintf(stderr,"wait öncesi \n");
			sleep(waitTime);
			result = sqrt(pow(variable1,2) + pow(variable2,2))/fabs(variable3);

			fprintf(stderr,"result %f\n",result);

			sprintf(message,"%lf %d\n",result,getpid());
			write(fifoToClient,message,strlen(message));

			sleep(1);

			close(fifoToClient);
			fifoToClient = open(fifoName, O_RDONLY);
			sleep(2);
		}
	}
	sleep(5);	
	close(fifoToClient);
	return 0;
}


/**
* This function make predefined operation as sqrt(a+b)
* Return: returns 0 incase success -1 for sum of variables lesser than zero
*/
int Operation2(char * fifoName){
	double result;
	int variable1,variable2,fifoToClient,waitTime,i=0,space,flag=0;
	char message[120];

	fprintf(stderr,"q%sq\n",fifoName);

	sleep(1);
    fifoToClient = open(fifoName,O_RDONLY);

    if(fifoToClient == -1)
    	perror("Error at opening Operation1: ");

    while(flag == 0){
		while(1){

		    if (read(fifoToClient, &message[i] ,1) != 0)
		    {
		        printf(" %c",message[i]);
		    }
		    else
		        break;

		    if(message[i] == '\n'){
		  		fprintf(stderr,"Debug1 new line\n");

		    	/*Checks the messages for is this for server*/
		        if(strncmp(message,"Math ",strlen("Math ")) == 0){
		        	message[i] ='\0';
		        	fprintf(stderr,"doğru geldi -%s-\n",message );		        	
		        	break;
		        }
		        if(strncmp(message,"kill\n",strlen("kill\n")) == 0){
		        		fprintf(stderr,"Debug1 \n");
		        		flag = 1;
		        }

		        /*If this message not for Math server writes messages back to fifo*/
		        else{
		        	close(fifoToClient);
		        	fifoToClient = open(fifoName,O_WRONLY);
		        	message[i+1] ='\0';

		        	write(fifoToClient,message,strlen(message));

		        	close(fifoToClient);
		        	fifoToClient = open(fifoName,O_RDONLY);
		        }
		        i = -1;
		    }
		    ++i;
		}
		
		if(flag == 0){
			fprintf(stderr,"Debug2 \n");

			/*If the message coming from a client waiting result from math server*/
			waitTime = atoi(&message[strlen("Math -")]);
			fprintf(stderr,"wait time %d\n",waitTime);
			
			for(i=0; message[i] != '\0';++i){
				if(message[i] == ' ')
					++space;
				if(space == 1)
					variable1 = atoi(&message[i+3]);
				if(space == 2)
					variable2 = atoi(&message[i+3]);
			}

			fprintf(stderr,"-var %d %d %d-\n",i,variable1,variable2);

			close(fifoToClient);
			fifoToClient = open(fifoName,O_WRONLY);

			fprintf(stderr,"wait öncesi \n");
			sleep(waitTime);
			if(variable1 + variable2 < 0){
				write(fifoToClient,"sum of parameters is negative\n",strlen("sum of parameters is negative\n"));
				continue;
			}

			result = sqrt(variable1 + variable2);

			fprintf(stderr,"result %f\n",result);

			sprintf(message,"%lf\n",result);
			write(fifoToClient,message,strlen(message));

			sleep(1);

			close(fifoToClient);
			fifoToClient = open(fifoName, O_RDONLY);
		}
		
		fprintf(stderr,"Debug3 \n");
		i=0;
	}

	fprintf(stderr,"Debug4 \n");
	
	close(fifoToClient);
	return 0;
}

/**
* This function make predefined operation as roots of a*x^2 +b*x + c
* Return: returns 0 in case of success -1 for discriminant is lesser than 0
*/
int Operation3(char * fifoName){
	double result;
	int variable1,variable2,variable3,fifoToClient,waitTime,i=0,space,flag=0;
	char message[120];

	fprintf(stderr,"q%sq\n",fifoName);

    fifoToClient = open(fifoName,O_RDONLY);

    if(fifoToClient == -1)
    	perror("Error at opening Operation1: ");

    while(flag == 0){
		while(1){

		    if (read(fifoToClient, &message[i] ,1) != 0)
		    {
		        printf(" %c",message[i]);
		    }
		    else
		        break;

		    if(message[i] == '\n'){
		    	/*Checks the messages for is this for server*/
		        if(strncmp(message,"Math ",strlen("Math ")) == 0){
		        	message[i] ='\0';
		        	fprintf(stderr,"doğru geldi -%s-\n",message );
		        	if(strncmp(message,"kill",strlen("kill")) == 0)
		        		flag = 1;
		        	
		        	break;
		        }

		        /*If this message not for Math server writes messages back to fifo*/
		        else{
		        	close(fifoToClient);
		        	fifoToClient = open(fifoName,O_WRONLY);
		        	message[i+1] ='\0';

		        	write(fifoToClient,message,strlen(message));

		        	close(fifoToClient);
		        	fifoToClient = open(fifoName,O_RDONLY);
		        }
		        i = -1;
		    }
		    ++i;
		}
		
		if(flag == 0){
			/*If the message coming from a client waiting result from math server*/
			waitTime = atoi(&message[strlen("Math -")]);
			fprintf(stderr,"wait time %d\n",waitTime);
			
			for(i=0; message[i] != '\0';++i){
				if(message[i] == ' ')
					++space;
				if(space == 1)
					variable1 = atoi(&message[i+2]);
				if(space == 2)
					variable2 = atoi(&message[i+2]);
				if(space == 3)
					variable3 = atoi(&message[i+2]);
			}

			fprintf(stderr,"-%d %d %d %d-\n",i,variable1,variable2,variable3);

			close(fifoToClient);
			fifoToClient = open(fifoName,O_WRONLY);

			result = variable2 * variable2 - 4 * variable1 * variable3;

			if(result < 0){
				write(fifoToClient,"Delta lesser than zero\n",strlen("Delta lesser than zero\n"));
				continue;
			}

			fprintf(stderr,"wait öncesi \n");
			sleep(waitTime);
			
			fprintf(stderr,"result %f\n",result);

			sprintf(message,"%lf\n",result);
			write(fifoToClient,message,strlen(message));
			memset(message,'\0',50);

			sleep(1);

			close(fifoToClient);
			fifoToClient = open(fifoName, O_RDONLY);
		}
	}
	
	close(fifoToClient);
	return 0;
}

/**
* This function make predefined operation as inverse function (a*x + b)/(c*x +d) as string
* Return: returns 0 incase success -1 for divide by zero
*/
int Operation4(char * fifoName){
	char result[100],temp[25];
	int variable1,variable2,variable3,variable4,fifoToClient,waitTime,i=0,space,flag=0;
	char message[120];

	fprintf(stderr,"q%sq\n",fifoName);

    fifoToClient = open(fifoName,O_RDONLY);

    if(fifoToClient == -1)
    	perror("Error at opening Operation1: ");

    while(flag == 0){
		while(1){

		    if (read(fifoToClient, &message[i] ,1) != 0)
		    {
		        printf(" %c",message[i]);
		    }
		    else
		        break;

		    if(message[i] == '\n'){
		    	/*Checks the messages for is this for server*/
		        if(strncmp(message,"Math ",strlen("Math ")) == 0){
		        	message[i] ='\0';
		        	fprintf(stderr,"doğru geldi -%s-\n",message );
		        	if(strncmp(message,"kill",strlen("kill")) == 0)
		        		flag = 1;
		        	
		        	break;
		        }

		        /*If this message not for Math server writes messages back to fifo*/
		        else{
		        	close(fifoToClient);
		        	fifoToClient = open(fifoName,O_WRONLY);
		        	message[i+1] ='\0';

		        	write(fifoToClient,message,strlen(message));

		        	close(fifoToClient);
		        	fifoToClient = open(fifoName,O_RDONLY);
		        }
		        i = -1;
		    }
		    ++i;
		}
		
		if(flag == 0){
			/*If the message coming from a client waiting result from math server*/
			waitTime = atoi(&message[strlen("Math -")]);
			fprintf(stderr,"wait time %d\n",waitTime);
			
			for(i=0; message[i] != '\0';++i){
				if(message[i] == ' ')
					++space;
				if(space == 1)
					variable1 = atoi(&message[i+2]);
				if(space == 2)
					variable2 = atoi(&message[i+2]);
				if(space == 3)
					variable3 = atoi(&message[i+2]);
				if(space == 4)
					variable4 = atoi(&message[i+2]);
			}

			fprintf(stderr,"-%d %d %d %d-\n",i,variable1,variable2,variable3);

			close(fifoToClient);
			fifoToClient = open(fifoName,O_WRONLY);

			strcpy(result,"(");
			sprintf(temp,"%d",-1 * variable4);
			strcat(result,temp);
			strcat(result,"*x + ");
			
			memset(temp,'\0',25);
			sprintf(temp,"%d",variable2);
			strcat(result,temp);

			strcat(result,") / (");
			sprintf(temp,"%d",variable3);
			strcat(result,temp);
			strcat(result,"*x + ");
			
			sprintf(temp,"%d",-1 * variable1);
			strcat(result,temp);
			strcat(result,")");

			if(variable1 == 0 && variable3 == 0){
				write(fifoToClient,"Divison by zero\n",strlen("Division by zero\n"));
				continue;
			}

			fprintf(stderr,"wait öncesi \n");
			sleep(waitTime);

			fprintf(stderr,"result %s\n",result);

			write(fifoToClient,result,strlen(result));

			sleep(1);

			close(fifoToClient);
			fifoToClient = open(fifoName, O_RDONLY);
		}
	}
	
	close(fifoToClient);
	return 0;
}