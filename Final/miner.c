/**
* This code written by Kasım Süzen at 24 March 2015
* This is a unique word count program which is wrote for CSE 244's Final exam/project
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

#define WORD_SIZE 128
#define WORD_COUNT_LIMIT 100000
#define SEMAPHORE_KEY_CHAR 'E'
#define SHARED_KEY_FOR_FILE_LIST 'F'
#define SHARED_KEY_FOR_FILE_LIST_INDEX 'I'
#define SHARED_KEY_FOR_FOUNDED_WORD 'W'
#define SHARED_KEY_FOR_FOUNDED_INDEX 'H'
#define SHARED_KEY_FOR_GOLD 'G'
#define SHARED_KEY_FOR_GOLD_INDEX 'X'

void usageError();
int receiver(int numberOfFiles);
int isAlpha(char key);
void * counter(void * filePath);
void logger();

struct UniqueWords {
	char * word;
	int wordCount;
	struct UniqueWords * next;
}UniqueWords_t;

int globalCountOfFiles;
int globalCountOfSubdirectories;
int count;
char ** foundedWords;
int foundedWordsSize;
int foundedWordsIndex;
struct timeval start, end;

pthread_mutex_t countMutex;

int main(int argc,char ** argv){

	int sem_id,i;
	key_t sem_key;
	struct sembuf sop;
	if(argc != 2) {
		usageError();
	}

	count =0;
	foundedWordsIndex=0;
	foundedWordsSize=1000;

	foundedWords = malloc(sizeof(char*) * foundedWordsSize);

	for(i = 0; i < foundedWordsSize; ++i){
		foundedWords[i] = malloc(sizeof(char)*WORD_SIZE);
		memset(foundedWords[i],'\0',WORD_SIZE);
	}

	sem_key = ftok(".", SEMAPHORE_KEY_CHAR);
	sem_id = semget(sem_key, 0, 0);
	if (sem_id < 0) {
		perror("Could not create main semaphor");
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
	i=receiver(atoi(argv[1]));
	logger(i);

	fprintf(stderr,"There are %d subdirectories and %d files under %s\n",globalCountOfSubdirectories,globalCountOfFiles,argv[1]);

	return(0);
}

/**
* If there are any error while running program this function will be printed
*/
void usageError(){
	fprintf(stderr,"Wrong call, should be as:\n");
	fprintf(stderr,"./miner numberOfFileToProcess \n");
	exit(-1);
}

/**
* This function will crawl through and into directories and their subdirectories
* @param: name of the root directory which search will begin
* @return number of file that has been processed
*/
int receiver(int numberOfFiles){
	int  i,itemCount=0,sem_id,shmIDIndex,shmIDBuffer,*bufferIndex,j;
	DIR *dp;
	FILE *fp;
	char ** itemList,*buffer,temp[PATH_MAX];
	key_t sem_key;
	struct sembuf sop;
	pthread_t * thread;
	struct dirent *ep;

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

	if((shmIDBuffer = shmget(shmKeyBuffer, sizeof(char) * WORD_COUNT_LIMIT * WORD_SIZE, IPC_CREAT|IPC_EXCL|0666)) == -1)
	{
		/* Segment probably already exists - try as a client */
		if((shmIDBuffer = shmget(shmKeyBuffer, sizeof(char) * WORD_COUNT_LIMIT * WORD_SIZE, 0)) == -1)
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

	/* Allocations for file and directory list */
	if(numberOfFiles > 0){
		itemList = malloc(numberOfFiles * sizeof(char*));

		for(i=0 ;i < numberOfFiles;++i) {
			itemList[i]= malloc(PATH_MAX*sizeof(char));
		}
	}

	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = SEM_UNDO;
	semop(sem_id, &sop, 1);

	for(i=0; i < numberOfFiles || *bufferIndex != 0; ++i){
		if(*bufferIndex > 2) {
			for (j = *bufferIndex - 2; j >= 0; --j) {
				if (buffer[j] == '\0') {
					break;
				}
			}
			++j;
		}
		else if(*bufferIndex <= 2 && *bufferIndex >= 0)
			j = 0;
		strcpy(itemList[i],&buffer[j]);
		if(j <= 0) {
			*bufferIndex=j;
			++i;
			break;
		}
		*bufferIndex = j;
	}
	itemCount = i;

	for(i=0;i < itemCount; ++i) {
		dp = opendir(itemList[i]);
		if (dp != NULL) {
			/* start from beginning of the directory and search till the end of directory pointer */
			for (j=0;ep = readdir (dp);++j){

				/* if readed element is directory this function will call itself with this directory */
				if((ep->d_type == DT_DIR || ep->d_type == DT_REG) && strcmp(".",ep->d_name) != 0 && strcmp("..",ep->d_name) != 0) {

					/*editing directory name for writing back to shared memory*/
					sprintf(temp,"%s/%s",itemList[i],ep->d_name);
					strcpy(&buffer[*bufferIndex],temp);
					*bufferIndex = *bufferIndex + strlen(temp) +1;
				}
			}
			for(j=i;j <= itemCount - 1; ++j){
				strcpy(itemList[j],itemList[j+1]);
			}
			--itemCount;
		}
		if(dp == NULL){
		}
		closedir(dp);
	}
	sop.sem_op = 1;
	semop(sem_id, &sop, 1);
	thread = malloc(sizeof(pthread_t) * itemCount);
	for(i=0; i < itemCount ; ++i){
		if((fp = fopen(itemList[i],"r")) != NULL){
			fclose(fp);
			pthread_create(&thread[i],NULL,counter,(void*)(itemList[i]));
		} else{
			fprintf(stderr,"fclose error %s %d\n",itemList[i],i);
			j=fclose(fp);
			if(j != 0){
				fprintf(stderr,"fclose error %s %d\n",itemList[i],i);
				perror("Error fclose");
			}
			fprintf(stderr,"%s named file couldnt open sometihng is wrong\n",itemList[i]);
			perror(" ");
		}
	}

	for(i = 0; i < itemCount; ++i) {
		pthread_join(thread[i], NULL);
	}

	if(numberOfFiles > 0){
		for(i=0; i < numberOfFiles;++i)
			free(itemList[i]);

		free(itemList);
	}

	free(thread);
	shmdt(buffer);
	shmdt(bufferIndex);
	return itemCount;
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

				pthread_mutex_lock(&countMutex);

				if(foundedWordsIndex >= foundedWordsSize){
					foundedWordsSize = foundedWordsSize * 2;
					foundedWords = realloc(foundedWords,sizeof(char*) * foundedWordsSize);
					for(i =foundedWordsSize / 2; i < foundedWordsSize;++i)
						foundedWords[i]=malloc(sizeof(char)*WORD_SIZE);

				}

				strcpy(foundedWords[foundedWordsIndex],temp_arr);
				++foundedWordsIndex;
				pthread_mutex_unlock(&countMutex);

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
void logger(int numberOfFile){

	FILE * logFile;
	struct UniqueWords founded,*position,*temp;
	int init=0,i,sem_id,shmIDIndex,shmIDBuffer,*bufferIndex,*goldMessageIndex,shmIDGold,shmIDGoldIndex;
	char *buffer,message[4096],*goldMessage;
	key_t sem_key,shmKeyIndex,shmKeyBuffer,shmKeyGoldMessage,shmKeyGoldMessageIndex;
	struct sembuf sop;
	long int seconds,useconds;

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

	shmKeyIndex = ftok(".",SHARED_KEY_FOR_FOUNDED_INDEX);
	shmKeyBuffer = ftok(".",SHARED_KEY_FOR_FOUNDED_WORD);
	shmKeyGoldMessage = ftok(".",SHARED_KEY_FOR_GOLD);
	shmKeyGoldMessageIndex = ftok(".",SHARED_KEY_FOR_GOLD_INDEX);

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

	if((shmIDGoldIndex = shmget(shmKeyGoldMessageIndex, sizeof(int), IPC_CREAT|IPC_EXCL|0666)) == -1)
	{
		/* Segment probably already exists - try as a client */
		if((shmIDGoldIndex = shmget(shmKeyGoldMessageIndex, sizeof(int) *1, 0)) == -1)
		{
			perror("shmget");
			exit(1);
		}
	}
	/* Attach (map) the shared memory segment into the current process */
	if((goldMessageIndex = (int *)shmat(shmIDGoldIndex, 0, 0)) == (int *)-1)
	{
		perror("shmat");
		exit(1);
	}

	if((shmIDGold = shmget(shmKeyGoldMessage, sizeof(char) * WORD_COUNT_LIMIT * WORD_SIZE, IPC_CREAT|IPC_EXCL|0666)) == -1)
	{
		/* Segment probably already exists - try as a client */
		if((shmIDGold = shmget(shmKeyGoldMessage, sizeof(char) * WORD_COUNT_LIMIT * WORD_SIZE, 0)) == -1)
		{
			perror("shmget");
			exit(1);
		}
	}

	/* Attach (map) the shared memory segment into the current process */
	if((goldMessage = (char *)shmat(shmIDGold, 0, 0)) == (char *)-1)
	{
		perror("shmat");
		exit(1);
	}

	sop.sem_op = 1;
	semop(sem_id, &sop, 1);

	founded.next = NULL;
	founded.wordCount = 0;
	position = &founded;

	sprintf(message,"logFileClient_%d",getpid());
	logFile = fopen(message,"w+");

	while(1){
		pthread_mutex_lock(&countMutex);

		if(foundedWordsIndex <= 0) {
			pthread_mutex_unlock(&countMutex);
			break;
		}

		if(foundedWordsIndex >= 0 && init == 0){
			founded.word = (char*)malloc(sizeof(char) * (int)strlen(foundedWords[foundedWordsIndex-1]));
			strcpy(founded.word,foundedWords[foundedWordsIndex-1]);
			++founded.wordCount;
			++count;
			--foundedWordsIndex;
		}

		if(foundedWordsIndex >= 0 && init == 1) {
			while(strcmp(position->word,foundedWords[foundedWordsIndex-1])){
				if(position->next == NULL)
					break;
				position=position->next;
			}

			if(strcmp(position->word,foundedWords[foundedWordsIndex-1]) == 0){
				++position->wordCount;
				--foundedWordsIndex;
				memset(foundedWords[foundedWordsIndex],'\0',WORD_SIZE);
			}

			else if(position->next == NULL){
				position->next = malloc(sizeof(struct UniqueWords));
				position = position->next;

				position->wordCount = 1;
				position->next = NULL;

				position->word = malloc(sizeof(char)*strlen(foundedWords[foundedWordsIndex - 1]));
				strcpy(position->word,foundedWords[foundedWordsIndex - 1]);
				--foundedWordsIndex;
				//++count;
			}

		}
		pthread_mutex_unlock(&countMutex);
		position = &founded;
		init = 1;
	}

	for(i=0; i < foundedWordsSize ; ++i){
		free(foundedWords[i]);
	}

	free(foundedWords);

	position = &founded;
	gettimeofday(&end, NULL);

	seconds  = end.tv_sec  - start.tv_sec;
	if(seconds > 0){
		useconds = 1000000 - start.tv_usec + end.tv_usec;
	}

	if(seconds == 0)
		useconds = end.tv_usec - start.tv_usec;
	fprintf(logFile,"Start time of miner from epoch is %ld second  %ld nanosecond\n",start.tv_sec,start.tv_usec);

	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = SEM_UNDO;
	semop(sem_id, &sop, 1);

	sprintf(message,"! %d %ld %ld",getpid(),seconds,useconds);
	strcpy(&buffer[*bufferIndex],message);
	*bufferIndex = *bufferIndex + strlen(message) +1;
	while(position->next != NULL){
		sprintf(message,"%s %d",position->word,position->wordCount);
		fprintf(logFile,"%s %d\n",position->word,position->wordCount);
		strcpy(&buffer[*bufferIndex],message);
		*bufferIndex = *bufferIndex + strlen(message) +1;
		position = position->next;
	}
	sprintf(message,"%s %d",position->word,position->wordCount);
	fprintf(logFile,"%s %d\n",position->word,position->wordCount);
	strcpy(&buffer[*bufferIndex],message);
	*bufferIndex = *bufferIndex + strlen(message) +1;

	sop.sem_op = 1;
	semop(sem_id, &sop, 1);

	i=-1;

	while(i == -1){
		sop.sem_num = 0;
		sop.sem_op = -1;
		sop.sem_flg = SEM_UNDO;
		semop(sem_id, &sop, 1);
		if(*goldMessageIndex != 0)
			i = 0;
		sop.sem_op = 1;
		semop(sem_id, &sop, 1);
		if( i == -1)
			usleep(1000);
	}
	init = 1;
	while(*goldMessageIndex > 0 && init  != 0){
		sop.sem_num = 0;
		sop.sem_op = -1;
		sop.sem_flg = SEM_UNDO;
		semop(sem_id, &sop, 1);

		while(init){
			for (i = *goldMessageIndex - 2; i >= 0 && goldMessage[i] != '\0'; --i) {
			}
			++i;
			if (goldMessage[i] == '!')
			sprintf(message, "! %d", getpid());
			usleep(500000);
			if (strncmp(&goldMessage[i],message,strlen(message)) == 0) {
				*goldMessageIndex = i;
				while (*goldMessageIndex > 0 && i > 0) {
					for (i = *goldMessageIndex - 2; i >= 0 && goldMessage[i] != '\0'; --i) {
					}
					++i;
					fprintf(logFile,"%s\n", &goldMessage[i]);
					*goldMessageIndex = i;
					if (goldMessage[*goldMessageIndex] == '!' || i == 0) {
						if(goldMessage[*goldMessageIndex] == '!'){
							*goldMessageIndex = *goldMessageIndex + strlen(&goldMessage[*goldMessageIndex])+1;
						}
						init=0;
						break;
					}
				}
			}
			else{
				break;
			}
		}
		sop.sem_op = 1;
		semop(sem_id, &sop, 1);
		usleep(1000);
	}

	fprintf(logFile,"End time of miner from epoch is %ld second  %ld nanosecond\n",end.tv_sec,end.tv_usec);
	fprintf(logFile,"This miner completed it's job at %ld second and %ld microsecond\n",seconds,useconds);
	fprintf(logFile,"This miner worked on %d file(s)\n",numberOfFile);
	shmdt(bufferIndex);
	shmdt(buffer);
	shmdt(goldMessage);
	shmdt(goldMessageIndex);

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