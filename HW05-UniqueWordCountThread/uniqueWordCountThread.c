/**
* This code written by Kasım Süzen at 24 March 2015
* This is a wc(word count) like program which is wrote for CSE 244's second homework 
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <pthread.h>
#include <errno.h>


void usageError();
int isAlpha(char key);
void * counter(void * filePath);
void * crawler(void *rootDirectoryName);
void logger();

struct UniqueWords {
	char * word;
	int wordCount;
	struct UniqueWords * next;
}UniqueWords_t;

int flagForWrite;
int globalCountOfFiles;
int globalCountOfSubdirectories;
int count;
int bufferIndex;
int bufferSize;
char ** buffer;

pthread_t mainThreadID;

pthread_mutex_t resultMutex;

int main(int argc,char ** argv){

	DIR *dp;
	int i;
	if(argc != 2)
		usageError();

	dp = opendir(argv[1]);
	if(dp == NULL){
		fprintf(stderr,"%s named file could not opened either directory does not exist or this user does not have acess to directory \n",argv[1]);
		usageError();
	}

	closedir(dp);

	count =0;
	bufferIndex =0;
	bufferSize = 100;
	flagForWrite = 0;
	buffer = malloc(sizeof(char*)* bufferSize);

	for(i =0; i < bufferSize; ++i )
		buffer[i] = malloc(sizeof(char)*256);

	mainThreadID = pthread_self();

	crawler((void*)argv[1]);

	logger();

	fprintf(stderr,"There are %d unique word\n",count);

	return(0);
}

/**
* If there are any error while running program this function will be printed
*/
void usageError(){
	fprintf(stderr,"Wrong call, should be as:\n");
	fprintf(stderr,"./wordCount -directoryName\n");
	exit(-1);
}

/**
* This function will crawl through and into directories and their subdirectories
* @param: name of the root directory which search will begin
*/
void * crawler(void *rootDirectoryName){
	pthread_t * thread;
	int  i,threadCount=0,count=0,numberOfSubdirectories=0,numberOfFile=0,control;
	DIR *dp;
	struct dirent *ep;
	char ** dirList,**fileList,rootDirectory[PATH_MAX];

	strcpy(rootDirectory,(char*)rootDirectoryName);

	dp = opendir(rootDirectory);
	/* open directory and count all elements inside directory include parent('..') and current directory('.') symbol */
	if (dp != NULL)
	{
		for (count=0;ep = readdir (dp);++count){

			if(ep == NULL)
				perror("Error at readdir ");

			if(ep->d_type == DT_DIR && strcmp(".",ep->d_name) != 0 && strcmp("..",ep->d_name) != 0)
				++numberOfSubdirectories;

			if(ep->d_type == DT_REG)
				++numberOfFile;
		}

		(void) closedir(dp);
	}
	else
		perror ("Couldn't open the directory");

	/* Allocations for file and directory list */
	if(0 != numberOfFile){
		fileList = malloc(numberOfFile * sizeof(char*));

		for(i=0 ;i < numberOfFile;++i)
			fileList[i]= malloc(PATH_MAX*sizeof(char));
	}

	if(numberOfSubdirectories != 0){
		dirList = malloc(numberOfSubdirectories * sizeof(char*));

		for(i=0 ;i < numberOfSubdirectories;++i)
			dirList[i]= malloc(PATH_MAX*sizeof(char));
	}

	/* allocate pid numbers for each directory and file -2 here for . and .. (current and parent directory symbols) */
	thread = malloc((count - 2)*sizeof(pthread_t));
	dp = opendir(rootDirectory);


	if(dp != NULL){

		/* start from beginning of the directory and search till the end of directory pointer */
		for (count=0;ep = readdir (dp);++count){

			/* if readed element is directory this function will call itself with this directory */
			if(ep->d_type == DT_DIR && strcmp(".",ep->d_name) != 0 && strcmp("..",ep->d_name) != 0){

				/*editing directory name for calling crawler function */
				strcpy(dirList[threadCount],rootDirectory);
				strcat(dirList[threadCount],"/");
				strcat(dirList[threadCount],ep->d_name);

				control = pthread_create(&thread[threadCount], NULL, crawler, (void *)dirList[threadCount]);
				if (control) {
					printf("ERROR; return code from pthread_create() is %d\n", control);
					exit(-1);
				}
				++threadCount;
			}

			/* if founded element is not directory it can only be file so name will edit */
			if(ep->d_type == DT_REG && strcmp(ep->d_name,".") != 0 && strcmp(ep->d_name,"..") != 0){
				strcpy(fileList[count - threadCount],rootDirectory);
				strcat(fileList[count - threadCount],"/");
				strcat(fileList[count - threadCount],ep->d_name);
			}

			/* Count variable will increase when . .. readed but should not increade because for this there will be thread creation */
			if(strcmp(ep->d_name,".") == 0 || strcmp(ep->d_name,"..") == 0){
				--count;
			}
		}
		(void)closedir(dp);
	}
	else
		perror ("Couldn't open the directory");

	/* Start new process for founded files. */
	count = 0;
	for (i = threadCount ; i <  threadCount + numberOfFile ; ++i)
	{

		if(strcmp(fileList[count],".") != 0 && strcmp(fileList[count],"..") != 0)
		{
			control = pthread_create(&thread[i], NULL, counter, (void *)(fileList[count]));
			if (control) {
				printf("ERROR; return code from pthread_create() is %d\n", control);
				exit(-1);
			}
		}

		++count;
	}

	pthread_mutex_lock(&resultMutex);

	globalCountOfFiles = globalCountOfFiles + numberOfFile;
	globalCountOfSubdirectories = globalCountOfSubdirectories + numberOfSubdirectories;

	pthread_mutex_unlock(&resultMutex);

	/* Wait for children to exit. */
	for(i=0;numberOfFile + numberOfSubdirectories > i;++i)
	{
		control = pthread_join(thread[i], NULL);
		if (control) {
			printf("ERROR; return code from pthread_join() is %d\n", control);
			exit(-1);
		}
	}

	free(thread);

	if(numberOfSubdirectories != 0){
		for(i=0; i < numberOfSubdirectories;++i)
			free(dirList[i]);

		free(dirList);
	}

	if(numberOfFile != 0){
		for(i=0; i < numberOfFile;++i)
			free(fileList[i]);

		free(fileList);
	}

	pthread_mutex_lock(&resultMutex);

	if(pthread_equal(pthread_self(),mainThreadID))
		flagForWrite = -1;

	pthread_mutex_unlock(&resultMutex);

}

/**
* Counts word in the file which given only word counts are consist only alphabetic characters
* @param: input file name for file
*/
void * counter(void * filePath){
	FILE * input;
	int flag=1,flagForExtraSpace=0,position=0,i=0;
	char temp,temp_arr[200],fileName[PATH_MAX];

	strcpy(fileName,(char*)filePath);
	input = fopen(fileName,"r");

	while(!feof(input)){
		fscanf(input,"%c",&temp);

		if(!isAlpha(temp) && temp != ' ' && temp != '\n'){ /* checking character is alphabetic */
			flag = 0;
		}

		if(isAlpha(temp) && temp != ' ' && temp != '\n'){
			temp_arr[position] = temp;
			++position;
		}

		if(isAlpha(temp))
			flagForExtraSpace=0; /* after one alphabetic character read we know a word has ocurred so space flag turned to true */

		/*EOF checked because if there is no space or new line character between eof and last character of word */
		if(temp == ' ' || temp == '\n' || feof(input)){
			
			if(flag == 1 && flagForExtraSpace ==0){ /* if flag equals to 1 characters between previos space and current are alphabetic words so increase word founded */
				
				if(temp_arr[position-1] == '.' || temp_arr[position-1] == ',')
					--position;

				flagForExtraSpace = 1;
				temp_arr[position] = '\0';

				pthread_mutex_lock(&resultMutex);

				if(bufferIndex >= bufferSize){
					bufferSize = bufferSize * 2;
					buffer = realloc(buffer,sizeof(char*) * bufferSize);
					for(i =bufferSize / 2; i < bufferSize;++i)
						buffer[i]=malloc(sizeof(char)*256);

				}

				strcpy(buffer[bufferIndex],temp_arr);
				++bufferIndex;

				pthread_mutex_unlock(&resultMutex);

				memset(temp_arr,'\0',200);
				++i;
			}

			position=0;
			flag = 1; /* reset flag because of space or new line*/
		}
	}
	fclose(input);
}

/**
* Checks is character alphabetic return 1 for true 0 for not alphabetic
* @param:key character which test for alphabet
* return 1 for alphabtic 0 for non-alphabetic
*/
int isAlpha(char key){
	if((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || key =='.' || key == ',')
		return 1;
	else
		return 0;
}

/**
* This function read from pipe and writes a log file about each words count
* @param: fileDescriptorWords reading side of pipe
*/
void logger(){

	FILE * logFile;
	struct UniqueWords founded,*position;
	int init=0;
	founded.next = NULL;
	founded.wordCount = 0;
	position = &founded;

	logFile = fopen("logFile","w+");

	while(1){
		pthread_mutex_lock(&resultMutex);

		if(flagForWrite == -1 && bufferIndex == 0) {
			pthread_mutex_unlock(&resultMutex);
			break;
		}

		if(bufferIndex >= 0 && init == 0){
			founded.word = malloc(sizeof(char) * strlen(buffer[bufferIndex]));
			strcpy(founded.word,buffer[bufferIndex-1]);
			++founded.wordCount;
			--bufferIndex;
			++count;
		}

		if(bufferIndex >= 0 && init == 1) {
			while(strcmp(position->word,buffer[bufferIndex-1])){
				if(position->next == NULL)
					break;
				position=position->next;
			}

			if(strcmp(position->word,buffer[bufferIndex-1]) == 0){
				++position->wordCount;
				--bufferIndex;
				memset(buffer[bufferIndex],'\0',256);

			}

			else if(position->next == NULL){

				position->next = malloc(sizeof(struct UniqueWords));
				position = position->next;

				position->wordCount = 1;
				position->next = NULL;

				position->word = malloc(sizeof(char)*strlen(buffer[bufferIndex-1]));
				strcpy(position->word,buffer[bufferIndex-1]);
				--bufferIndex;
				memset(buffer[bufferIndex],'\0',256);
				++count;
			}

		}
		pthread_mutex_unlock(&resultMutex);

			position = &founded;
		init = 1;
	}

	position = &founded;

	while(position->next != NULL){
		fprintf(logFile,"%s %d\n",position->word,position->wordCount);
		position = position->next;
	}
	fprintf(logFile,"%s %d\n",position->word,position->wordCount);
	fclose(logFile);
}