/**
* This code written by Kasım Süzen at 24 March 2015
* This is a unique word count program which is wrote for CSE 244's fifth homework
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
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define WORD_COUNT_LIMIT 100000
#define SEMAPHORE_KEY_CHAR 'E'
#define SHARED_KEY_FOR_FILE_LIST 'F'
#define SHARED_KEY_FOR_FILE_LIST_INDEX 'I'
#define SHARED_KEY_FOR_FOUNDED_DIR 'D'
#define SHARED_KEY_FOR_FOUNDED_INDEX 'H'

void usageError();
int crawler(char *rootDirectory);
void logger();

struct UniqueWords {
	char * word;
	int wordCount;
	struct UniqueWords * next;
}UniqueWords_t;

int globalCountOfFiles;
int globalCountOfSubdirectories;
int count;

int main(int argc,char ** argv){

	DIR *dp;
	int *bufferIndex,sem_id,shmIDIndex,shmIDBuffer;
	key_t sem_key,shmKeyIndex;
	struct sembuf sop;
	if(argc <= 1) {
		usageError();
	}

	dp = opendir(argv[1]);
	if(dp == NULL){
		fprintf(stderr,"%s named file could not opened either directory does not exist or this user does not have acess to directory \n",argv[1]);
		usageError();
	}

	closedir(dp);

	count =0;
	shmKeyIndex = ftok(".",SHARED_KEY_FOR_FILE_LIST_INDEX);

	if((shmIDIndex = shmget(shmKeyIndex, sizeof(int), IPC_CREAT|IPC_EXCL|0666)) == -1)
	{
		/* Segment probably already exists - try as a client */
		if((shmIDIndex = shmget(shmKeyIndex, sizeof(int), 0)) == -1)
		{
			perror("shmget");
			exit(1);
		}
	}

	/* Attach (map) the shared memory segment into the current process */
	if((bufferIndex = (int *)shmat(shmIDIndex, 0, 0)) == (int *)-1)
	{
		perror("shmat");
		exit(1);
	}

	*bufferIndex = 0;

	sem_key = ftok(".", SEMAPHORE_KEY_CHAR);
	sem_id = semget(sem_key, 1, IPC_CREAT | IPC_EXCL | 0600);
	if (sem_id < 0) {
		perror("Could not create main sem");
		exit(3);
	}

	if (semctl(sem_id, 0, SETVAL, 0) < 0) {
		perror("Could not set value of semaphore");
		exit(4);
	}

	sop.sem_num = 0;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	if (semop(sem_id, &sop, 1)) {
		perror("Could not increment semaphore");
		exit(5);
	}
	
	crawler(argv[1]);

	logger();

	fprintf(stderr,"There are %d unique word\n",count);
	fprintf(stderr,"There are %d subdirectories and %d files under %s\n",globalCountOfSubdirectories,globalCountOfFiles,argv[1]);
	if (semctl(sem_id, 0, IPC_RMID) < 0) {
		perror("Could not delete semaphore");
	}

	sleep(10);

	if(shmdt(bufferIndex) != 0)
		perror("Error at shmdt index buffer main");

	shmIDBuffer = shmget(ftok(".",SHARED_KEY_FOR_FILE_LIST), sizeof(char) * WORD_COUNT_LIMIT * 128, 0);

	if(shmctl(shmIDBuffer,IPC_RMID,NULL) != 0)
		perror("Error at shmctl index main");

	if(shmctl(shmIDIndex,IPC_RMID,NULL) != 0)
		perror("Error at shmctl at message main");

	return(0);
}

/**
* If there are any error while running program this function will be printed
*/
void usageError(){
	fprintf(stderr,"Wrong call, should be as:\n");
	fprintf(stderr,"./wordCount \"directoryName\" \"directoryName\" ... \n");
	exit(-1);
}

/**
* This function will crawl through and into directories and their subdirectories
* @param: name of the root directory which search will begin
*/
int crawler(char *rootDirectory){
	int  i,count=0,itemCount=0,sem_id,shmIDIndex,shmIDBuffer,*bufferIndex;
	DIR *dp;
	struct dirent *ep;
	char ** itemList,*buffer;
	struct timeval start, end;
	long seconds, useconds;
	key_t sem_key;
	struct sembuf sop;
	key_t  shmKeyIndex,shmKeyBuffer;
	sem_key = ftok(".", SEMAPHORE_KEY_CHAR);
	// Create the semaphore
	sem_id = semget(sem_key, 0, 0);
	if (sem_id < 0) {
		perror("Could not obtain semaphore");
		exit(3);
	}

	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = SEM_UNDO;
	semop(sem_id, &sop, 1);

	shmKeyIndex = ftok(".",SHARED_KEY_FOR_FILE_LIST_INDEX);
	shmKeyBuffer = ftok(".",SHARED_KEY_FOR_FILE_LIST);

	if((shmIDIndex = shmget(shmKeyIndex, sizeof(int), IPC_CREAT|IPC_EXCL|0666)) == -1)
	{
		/* Segment probably already exists - try as a client */
		if((shmIDIndex = shmget(shmKeyIndex, sizeof(int), 0)) == -1)
		{
			perror("shmget");
			exit(1);
		}
	}
	/* Attach (map) the shared memory segment into the current process */
	if((bufferIndex = (int *)shmat(shmIDIndex, 0, 0)) == (int *)-1)
	{
		perror("shmat");
		exit(1);
	}

	if((shmIDBuffer = shmget(shmKeyBuffer, sizeof(char) * WORD_COUNT_LIMIT * 128, IPC_CREAT|IPC_EXCL|0666)) == -1)
	{
		/* Segment probably already exists - try as a client */
		if((shmIDBuffer = shmget(shmKeyBuffer, sizeof(char) * WORD_COUNT_LIMIT * 128, 0)) == -1)
		{
			perror("shmget");
			exit(1);
		}
	}

	/* Attach (map) the shared memory segment into the current process */
	if((buffer = (char *)shmat(shmIDBuffer, 0, 0)) == (char *)-1)
	{
		perror("shmat");
		exit(1);
	}

	sop.sem_op = 1;
	semop(sem_id, &sop, 1);

	gettimeofday(&start, NULL);

	dp = opendir(rootDirectory);
	/* open directory and count all elements inside directory include parent('..') and current directory('.') symbol */
	if (dp != NULL)
	{
		for (count=0;ep = readdir (dp);++count){

			if(ep == NULL)
				perror("Error at readdir ");

			if((ep->d_type == DT_DIR || ep->d_type == DT_REG) && strcmp(".",ep->d_name) != 0 && strcmp("..",ep->d_name) != 0)
				++itemCount;
		}

		(void) closedir(dp);
	}
	else
		perror ("Couldn't open the directory");

	/* Allocations for file and directory list */

	if(itemCount > 0){
		itemList = malloc(itemCount * sizeof(char*));

		for(i=0 ;i < itemCount;++i)
			itemList[i]= malloc(PATH_MAX*sizeof(char));
	}

	dp = opendir(rootDirectory);

	if(dp != NULL){

		/* start from beginning of the directory and search till the end of directory pointer */
		for (count=0;ep = readdir (dp);++count){

			/* if readed element is directory this function will call itself with this directory */
			if((ep->d_type == DT_DIR || ep->d_type == DT_REG) && strcmp(".",ep->d_name) != 0 && strcmp("..",ep->d_name) != 0) {

				/*editing directory name for calling crawler function*/
				strcpy(itemList[count], rootDirectory);
				strcat(itemList[count], "/");
				strcat(itemList[count], ep->d_name);
			}
			/* Count variable will increase when . .. readed but should not increase because for this there will be thread creation */
			if(strcmp(ep->d_name,".") == 0 || strcmp(ep->d_name,"..") == 0){
				--count;
			}
		}
		(void)closedir(dp);
	}
	else
		perror ("Couldn't open the directory");

	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = SEM_UNDO;
	semop(sem_id, &sop, 1);

	for(i=0; i < itemCount; ++i){
		strcpy(&buffer[*bufferIndex],itemList[i]);
		*bufferIndex = *bufferIndex + strlen(itemList[i]) +1;
	}

	sop.sem_op = 1;
	semop(sem_id, &sop, 1);

	gettimeofday(&end, NULL);
	seconds  = end.tv_sec  - start.tv_sec;
	if(seconds > 0){
		useconds = 1000000 - start.tv_usec + end.tv_usec;
	}

	if(seconds == 0)
		useconds = end.tv_usec - start.tv_usec;
	fprintf(stderr,"%s directory searched finished in %ld second and %ld microsecond\n",rootDirectory,seconds,useconds);

	if(itemCount > 0){
		for(i=0; i < itemCount;++i)
			free(itemList[i]);

		free(itemList);
	}

	shmdt(buffer);
	shmdt(bufferIndex);
	return itemCount;
}

/**
* This function read from pipe and writes a log file about each words count
* @param: fileDescriptorWords reading side of pipe
*/
void logger(){

	FILE * logFile;
	struct UniqueWords founded,*position,*temp;
	int init=0,i,sem_id,shmIDIndex,shmIDBuffer,*bufferIndex;
	char *buffer;
	key_t sem_key,shmKeyIndex,shmKeyBuffer;
	struct sembuf sop;

	sem_key = ftok(".", SEMAPHORE_KEY_CHAR);
	// Create the semaphore
	sem_id = semget(sem_key, 0, 0);
	if (sem_id < 0) {
		perror("Could not obtain semaphore");
		exit(3);
	}

	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = SEM_UNDO;
	semop(sem_id, &sop, 1);

	shmKeyIndex = ftok(".",SHARED_KEY_FOR_FILE_LIST_INDEX);
	shmKeyBuffer = ftok(".",SHARED_KEY_FOR_FILE_LIST);

	if((shmIDIndex = shmget(shmKeyIndex, sizeof(int), IPC_CREAT|IPC_EXCL|0666)) == -1)
	{
		/* Segment probably already exists - try as a client */
		if((shmIDIndex = shmget(shmKeyIndex, sizeof(int), 0)) == -1)
		{
			perror("shmget");
			exit(1);
		}
	}
	/* Attach (map) the shared memory segment into the current process */
	if((bufferIndex = (int *)shmat(shmIDIndex, 0, 0)) == (int *)-1)
	{
		perror("shmat");
		exit(1);
	}

	if((shmIDBuffer = shmget(shmKeyBuffer, sizeof(char) * WORD_COUNT_LIMIT * 128, IPC_CREAT|IPC_EXCL|0666)) == -1)
	{
		/* Segment probably already exists - try as a client */
		if((shmIDBuffer = shmget(shmKeyBuffer, sizeof(char) * WORD_COUNT_LIMIT * 128, 0)) == -1)
		{
			perror("shmget");
			exit(1);
		}
	}

	/* Attach (map) the shared memory segment into the current process */
	if((buffer = (char *)shmat(shmIDBuffer, 0, 0)) == (char *)-1)
	{
		perror("shmat");
		exit(1);
	}

	sop.sem_op = 1;
	semop(sem_id, &sop, 1);

	founded.next = NULL;
	founded.wordCount = 0;
	position = &founded;

	logFile = fopen("logFile","w+");

	while(1){
		sop.sem_num = 0;
		sop.sem_op = -1;
		sop.sem_flg = SEM_UNDO;
		semop(sem_id, &sop, 1);

		if(*bufferIndex <= 0) {
			sop.sem_op = 1;
			semop(sem_id, &sop, 1);
			break;
		}

		if(*bufferIndex >= 0 && init == 0){

			founded.word = malloc(sizeof(char) * strlen(&buffer[*bufferIndex]));
			strcpy(founded.word,&buffer[*bufferIndex]);
			++founded.wordCount;
			++count;
		}

		if(*bufferIndex >= 0 && init == 1) {

			do{
				for(i = *bufferIndex-2;i >= 0 && buffer[i] != '\0';--i){
				}

				if(position->next == NULL)
					break;
				position=position->next;
			}while(strcmp(position->word,&buffer[i+1]));

			if(strcmp(position->word,&buffer[i+1]) == 0){
				++position->wordCount;
				*bufferIndex=i;
			}

			else if(position->next == NULL){

				position->next = malloc(sizeof(struct UniqueWords));
				position = position->next;

				position->wordCount = 1;
				position->next = NULL;

				position->word = malloc(sizeof(char)*strlen(&buffer[i+1]));
				strcpy(position->word,&buffer[i+1]);
				*bufferIndex = i;
				++count;
			}

		}
		sop.sem_op = 1;
		semop(sem_id, &sop, 1);

			position = &founded;
		init = 1;
	}

	position = &founded;

	while(position->next != NULL){
		fprintf(logFile,"%s %d\n",position->word,position->wordCount);
		position = position->next;
	}
	fprintf(logFile,"%s %d\n",position->word,position->wordCount);

	i=0;
	for(position = founded.next; position != NULL ; ){
		++i;
		temp = position;
		free(temp->word);
		if(position->next != NULL){
			free(position);
		}
		position = temp->next;
	}
	free(temp);
	free(founded.word);

	fclose(logFile);
}