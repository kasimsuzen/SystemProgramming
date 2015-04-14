/**
* Written by Kasım Süzen at 14/04/2015
* This is midterm project client for CSE 244 class at GTU
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>  
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

./clientX -<mainFifoName> -<waitingTime> -<operationName> -<parametre_1> ...-<parametre_k>;

int main(int argc,char ** argv){
	int waitTime,fifoSynch,i=0,mathFifo,operationNumber;
	char message[120],temp[15],fifoName[70];

	fifoSynch = open(argv[1],O_WRONLY);

	operationNumber = operationNumberFinder(argv[3]);

	sprintf(message,"Server %d %d",operationNumber,getpid());
	write(fifoSynch,message,strlen(message));

	close(fifoSynch);
	fifoSynch = open(argv[1],O_RDONLY);

	while(1){
		if (read(fifoSynch, &buf[i] ,1) != 0)
        {
            //perror("while ");
            printf("%c",buf[i]);
        }
        else
            break;

        if(buf[i] == '\n'){
        	sprintf(temp,"%d",getpid());
        	/*checks if message at fifo is for this client */
        	if(strncmp(temp,buf,strlen(temp)) == 0){
        		/*Take new fifoName for message */
        		strcpy(&fifoName[strlen(temp)+1]);
        		break;
        	}
        	else{
        		/*If message not meant for this client writes message back to fifo */
				close(fifoSynch);
				fifoSynch = open(argv[1],O_WRONLY);

				buf[i+1]='\0';
				write(fifoSynch,buf,strlen,(buf));

				close(fifoSynch);
				fifoSynch = open(argv[1],O_RDONLY);
        	}
            i = -1;
        }
        ++i;
	}

	mathFifo = open(fifoName,O_WRONLY);

	memset(message,'\0',100);

	strcpy(message,waitingTime);
	strcat(message," ");
	strcat(message,argv[4]);
	strcat(message," ");
	strcat(message,argv[5]);
	
	if(operationNumber == 1){
		strcat(message," ");
		strcat(message,argv[6]);
	}

	if(operationNumber == 3){
		strcat(message," ");
		strcat(message,argv[6]);
	}

	if(operationNumber == 4){
		strcat(message," ");
		strcat(message,argv[6]);
		strcat(message," ");
		strcat(message,argv[7]);
	}

	write(mathFifo,message,strlen(message));

	close(mathFifo);

	mathFifo = open(fifoName, O_RDONLY);
	i=0;

	while (1)
	{
	    if (read(mathFifo, &message[i] ,1) != 0)
	    {
	        //perror("while ");
	        printf("%c",message[i]);
	    }
	    else
	        break;

	    if(message[i] == '\n'){
	        i = -1;
	        break;
	    }
	    ++i;

	}

	fprintf(stderr,"readed value from server %s \n",message);
	waitTime = atoi(argv[2]);
	sleep(waitTime);
}

int operationNumberFinder(char * arg){
	if(strcmp(tolower(arg),"operation1") == 0 || strcmp(tolower(arg),"-operation1") == 0 ||strcmp(arg,"1") == 0 )
		return 1;

	if(strcmp(tolower(arg),"operation2") == 0 || strcmp(tolower(arg),"-operation2") == 0 ||strcmp(arg,"2") == 0 )
		return 2;
	
	if(strcmp(tolower(arg),"operation3") == 0 || strcmp(tolower(arg),"-operation3") == 0 ||strcmp(arg,"3") == 0 )
		return 3;

	if(strcmp(tolower(arg),"operation4") == 0 || strcmp(tolower(arg),"-operation4") == 0 ||strcmp(arg,"4") == 0 )
		return 4;
}