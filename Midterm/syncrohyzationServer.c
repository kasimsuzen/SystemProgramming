/**
* Written by Kasım Süzen at 14/04/2015
* This is midterm project server for CSE 244 class at GTU
* TODO client math server fifo name
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
#include <unistd.h>
#include <fcntl.h>  
#include <sys/stat.h>

int main(int argc,char ** argv){
	listener(argv[1]);
	return 0;
}

int listener(char * nameOfFifo){
	int mainFifo,i=0,flag=0,operationNumber;
	char message[120],newFifoName[25];
	struct stat st;
	pid_t childpid;


	if (stat(nameOfFifo, &st) != 0)
        mkfifo(nameOfFifo, 0777);
   
   	mainFifo = open(nameOfFifo, O_RDONLY);

    while(1){
	
		while (1)
		{
		    if (read(mainFifo, &message[i] ,1) != 0)
		    {
		        printf("%c",message[i]);
		    }
		    else
		        break;

		    if(message[i] == '\n'){
		    	/*Checks the messages for is this for server*/
		        if(strncmp(message,"Server ",strlen("Server ")) == 0){
		        	flag = 1;
		        	message[i+1] ='\0';
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
		    ++i;
		}
		/*If the message coming from aclient waiting for math server*/
		if(flag == 1){
			operationNumber = atoi(&message[strlen("Server ")+1]);
			strcpy(newFifoName,&message[strlen("Server ")+3]);
			strcat(newFifoName,"_fifo");

			close(mainFifo);
			mainFifo = open(nameOfFifo,O_WRONLY);

			write(mainFifo,newFifoName,strlen(newFifoName));

			close(mainFifo);
			mainFifo = open(nameOfFifo,O_RDONLY);

			if((childpid = fork()) == -1)
		    {
		        perror("fork");
		        exit(1);
		    }

			if(childpid == 0)
		    {
		    	operation(newFifoName,operationNumber);
		        exit(0);
		    }

		}

	}
}

/**
* This function choose which operation will be run and return is they killed by signal or user
* return -1 on error, -2 on killing by signal, 1 on kill by client
*/
int operation(char * fifoName,int operationNumber){
	int status;

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
	double variable1,variable2,variable3,result;
	int fifoToClient,waitTime,i=0;
	char message[120],*position;
	struct stat st;

	if (stat(fifoName, &st) != 0)
        mkfifo(fifoName, 0777);

    fifoToClient = open(fifoName,O_RDONLY);

	while(1){
		while(1){

		    if (read(fifoToClient, &message[i] ,1) != 0)
		    {
		        printf("%c",message[i]);
		    }
		    else
		        break;

		    if(message[i] == '\n'){
		    	/*Checks the messages for is this for server*/
		        if(strncmp(message,"Math ",strlen("Math ")) == 0){
		        	message[i+1] ='\0';
		        	
		        	if(strncmp(message,"kill",strlen("kill")) == 0)
		        		return 0;
		        	
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
		/*If the message coming from a client waiting result from math server*/
		waitTime = atoi(message);

		position = strchr(message,' ');
		variable1 = atof(position - message + 1);
		
		position = strchr(position + 1,' ');
		variable2 = atof(position - message + 1);
		
		position = strchr(position + 1,' ');
		variable3 = atof(position - message + 1);

		if(variable3 == 0)
			return -1;

		sleep(waitTime);
		result = sqrt(pow(variable1,2) + pow(variable2,2))/fabs(variable3);

		close(fifoToClient);
		fifoToClient = open(fifoName,O_WRONLY);

		sprintf(message,"%lf\n",result);
		write(fifoToClient,message,strlen(message));

		close(fifoToClient);
		fifoToClient = open(fifoName, O_RDONLY);
	}

	return 0;
}


/**
* This function make predefined operation as sqrt(a+b)
* Return: returns 0 incase success -1 for sum of variables lesser than zero
*/
int Operation2(char * fifoName){
	double variable1,variable2,result;

	if(variable1 + variable2 < 0)
		return -1;

	result = sqrt(variable1 + variable2);

	return 0;
}

/**
* This function make predefined operation as roots of a*x^2 +b*x + c
* Return: returns 0 in case of success -1 for discriminant is lesser than 0
*/
int Operation3(char * fifoName){
	double variable1,variable2,variable3,result;

	result = variable2 * variable2 - 4 * variable1 * variable3;


	if(result < 0)
		return -1;

	return 0;
}

/**
* This function make predefined operation as inverse function (a*x + b)/(c*x +d) as string
* Return: returns 0 incase success -1 for divide by zero
*/
int Operation4(char * fifoName){
	char result[100],temp[25];
	double variable1,variable2,variable3;

	if( variable1 == 0 && variable3 == 0)
		return -1;

	strcpy(result,"(");
	sprintf(temp,"%lf",-1 * variable2);
	strcat(result,temp);
	strcat(result,"*x + ");

	memset(temp,'\0',25);

//	strcat(result,argument2);
	strcat(result,") / (");

//	strcat(result,argument3);
	strcat(result,"*x + ");
	
	sprintf(temp,"%lf",-1 * variable1);
	strcat(result,temp);
	strcat(result,")");

	printf("%s\n",result);
	return 0;
}