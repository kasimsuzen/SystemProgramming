/**
* Written by Kasım Süzen at 14/04/2015
* This is midterm project client for CSE 244 class at GTU
* ./clientX -<mainFifoName> -<waitingTime> -<operationName> -<parametre_1> ...-<parametre_k>;
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>  
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

int operationNumberFinder(char * arg);

int main(int argc,char ** argv){
	int waitTime,fifoSynch,i=0,mathFifo,operationNumber;
	char message[120],temp[15],fifoName[70],mainFifo[80];
	struct stat st;

	FILE * log;
	strcpy(mainFifo,&argv[1][1]);

	fprintf(stderr,"%s\n",mainFifo);
	fifoSynch = open(mainFifo,O_WRONLY);

	operationNumber = operationNumberFinder(argv[3]);

	sprintf(message,"Server %d %d\n",operationNumber,getpid());
	sprintf(temp,"client%d",operationNumber);
	
	log=fopen(temp,"w+");

	write(fifoSynch,message,strlen(message));

	close(fifoSynch);
	fifoSynch = open(mainFifo,O_RDONLY);

	while(1){
		if (read(fifoSynch, &message[i] ,1) != 0)
        {
        	fprintf(stderr,"%c\n",message[i]);
            
            if(message[i] == '\n'){
            	sprintf(temp,"%d",getpid());
    			message[i+1]='\0';

    			fprintf(stderr,"message %s \n",message);
            	/*checks if message at fifo is for this client */
            	fprintf(stderr,"debug%s\n",message);
            	if(strncmp(temp,message,strlen(temp)) == 0){
            		fprintf(stderr,"debug2%sq\n",message);
            		/*Take new fifoName for message */
            		strcpy(fifoName,message);
            		fifoName[i]='\0';
            		break;
            	}
            	else{
            		/*If message not meant for this client writes message back to fifo */
    				close(fifoSynch);
    				fifoSynch = open(mainFifo,O_WRONLY);

    				message[i+1]='\0';
    				write(fifoSynch,message,strlen(message));

    				close(fifoSynch);
    				fifoSynch = open(mainFifo,O_RDONLY);
            	}
                i = -1;
            }
        }
        else
            break;

        ++i;
	}
	fprintf(stderr,"fifo opening q%sq\n",fifoName);
	if (stat(fifoName, &st) != 0){
        i=mkfifo(fifoName, 0777);
        if( i != 0)
        	perror("Client error at creation ");
	}else
		perror("stat hata ");

	fprintf(stderr,"fifo oluşturuldu \n");
	
	mathFifo = open(fifoName,O_WRONLY);
	if(mathFifo == -1)
		perror("Open error: ");
	fprintf(stderr,"fifo açıldı \n");

	memset(message,'\0',100);
	
	waitTime = atoi(argv[2]);
	fprintf(stderr,"wait%d\n",waitTime);

	sprintf(message,"Math %d ",waitTime);
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

	strcat(message,"\n");
	waitTime = atoi(&argv[2][1]);

	write(mathFifo,message,strlen(message));
	fprintf(stderr,"mesaj sonu --%s--\n",message);

	close(mathFifo);

	mathFifo = open(fifoName, O_RDONLY);
	i=0;

	memset(message,'\0',120);

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
	fprintf(log,"result is %s\n",message);
	sleep(waitTime);

	close(mathFifo);
	mathFifo = open(fifoName, O_WRONLY);

	write(mathFifo,"kill\n",strlen("kill\n"));

	sleep(1);
	close(mathFifo);

	fclose(log);
	unlink(fifoName);
	return 0;
}

int operationNumberFinder(char * arg){
	int i=1;

	if(arg[0] == '-'){
	for(i=0;i < strlen(arg) - 1;++i)
			arg[i]=arg[i+1];
	}
	arg[i]='\0';

	for(i=0; i < strlen(arg);++i)
		if(isalpha(arg[i]))
			arg[i]=tolower(arg[i]);
	
	arg[i] = '\0';

	fprintf(stderr,"oper %s\n",arg);

	if(strcmp(arg,"operation1") == 0 || strcmp(arg,"-operation1") == 0 ||strcmp(arg,"1") == 0 )
		return 1;

	else if(strcmp(arg,"operation2") == 0 || strcmp(arg,"-operation2") == 0 ||strcmp(arg,"2") == 0 )
		return 2;
	
	else if(strcmp(arg,"operation3") == 0 || strcmp(arg,"-operation3") == 0 ||strcmp(arg,"3") == 0 )
		return 3;

	else if(strcmp(arg,"operation4") == 0 || strcmp(arg,"-operation4") == 0 ||strcmp(arg,"4") == 0 )
		return 4;
	else{
		fprintf(stderr,"Operation number is faulty %s\n",arg);
		return -1;
	}
}