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
int crawler(char *rootDirectory);
void logger();
int strcmpTillSpace(const char * input1,const char * input2);

struct UniqueWords {
	char * word;
	int wordCount;
	struct UniqueWords * next;
}UniqueWords_t;

struct minerInfo{
	int pid,paidCoin;
	long int startSecond,startMicrosecond,endSecond,endMicrosecond;
	struct minerInfo * next;
}minerInfo_t;

int globalCountOfFiles;
int globalCountOfSubdirectories;
int count;

int main(int argc,char ** argv){

	DIR *dp;
	int i=1,j,*bufferIndex,*goldIndex,*foundIndex,sem_id,shmIDIndex,shmIDBuffer,shmIDGold,shmIDGoldIndex,shmIDFoundIndex,shmIDFound;
	key_t sem_key,shmKeyIndex,shmKeyGoldIndex,shmKeyFoundIndex;
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
	shmKeyGoldIndex = ftok(".",SHARED_KEY_FOR_GOLD_INDEX);
	shmKeyFoundIndex = ftok(".",SHARED_KEY_FOR_FOUNDED_INDEX);


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

	if((shmIDGoldIndex = shmget(shmKeyGoldIndex, sizeof(int)*1, IPC_CREAT|IPC_EXCL|0666)) == -1)
	{
		/* Segment probably already exists - try as a client */
		if((shmIDGoldIndex = shmget(shmKeyGoldIndex, sizeof(int), 0)) == -1)
		{
			perror("shmget");
			exit(1);
		}
	}

	/* Attach (map) the shared memory segment into the current process */
	if((goldIndex = (int *)shmat(shmIDGoldIndex, 0, 0)) == (int *)-1)
	{
		perror("shmat");
		exit(1);
	}

	if((shmIDFoundIndex = shmget(shmKeyFoundIndex, sizeof(int)*1, IPC_CREAT|IPC_EXCL|0666)) == -1)
	{
		/* Segment probably already exists - try as a client */
		if((shmIDFoundIndex = shmget(shmKeyFoundIndex, sizeof(int), 0)) == -1)
		{
			perror("shmget");
			exit(1);
		}
	}

	/* Attach (map) the shared memory segment into the current process */
	if((foundIndex = (int *)shmat(shmIDFoundIndex, 0, 0)) == (int *)-1)
	{
		perror("shmat");
		exit(1);
	}

	*foundIndex=0;
	*bufferIndex = 0;
	*goldIndex = 0;

	sem_key = ftok(".", SEMAPHORE_KEY_CHAR);
	sem_id = semget(sem_key, 1, IPC_CREAT | 0666);
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
	while(i) {
		sop.sem_num = 0;
		sop.sem_op = -1;
		sop.sem_flg = SEM_UNDO;
		semop(sem_id, &sop, 1);

		i = *bufferIndex;
		j= *foundIndex;
		sop.sem_op = 1;
		semop(sem_id, &sop, 1);

		while(i > 0 && j <= 0) {
			usleep(50000);

			sop.sem_num = 0;
			sop.sem_op = -1;
			sop.sem_flg = SEM_UNDO;
			semop(sem_id, &sop, 1);

			i = *bufferIndex;
			j= *foundIndex;

			sop.sem_op = 1;
			semop(sem_id, &sop, 1);

			i=1;
		}
		if(i < 0){
			i=-1;
			break;
		}
		if(i == 0 && j > 0) {
			sleep(1);
			logger();
			break;
		}
	}

	fprintf(stderr,"There are %d unique word\n",count);
	fprintf(stderr,"There are %d subdirectories and %d files under %s\n",globalCountOfSubdirectories,globalCountOfFiles,argv[1]);
	if (semctl(sem_id, 0, IPC_RMID) < 0) {
		perror("Could not delete semaphore");
	}

	if(shmdt(bufferIndex) != 0) {
		perror("Error at shmdt index buffer main");
	}
	if(shmdt(goldIndex) != 0) {
		perror("Error at shmdt gold index buffer main");
	}

	shmIDBuffer = shmget(ftok(".",SHARED_KEY_FOR_FILE_LIST), sizeof(char) * WORD_COUNT_LIMIT * WORD_SIZE, 0);
	shmIDGold = shmget(ftok(".",SHARED_KEY_FOR_GOLD), sizeof(char) * WORD_COUNT_LIMIT * WORD_SIZE, 0);
	shmIDFound = shmget(ftok(".",SHARED_KEY_FOR_FOUNDED_WORD),sizeof(char) * WORD_COUNT_LIMIT * WORD_SIZE,0);

	if(shmctl(shmIDBuffer,IPC_RMID,NULL) != 0) {
		perror("Error at shmctl index main mine");
	}

	if(shmctl(shmIDIndex,IPC_RMID,NULL) != 0) {
		perror("Error at shmctl at message main mine");
	}

	if(shmctl(shmIDFound,IPC_RMID,NULL) != 0)
		perror("Error at shmctl index main mine");

	if(shmctl(shmIDFoundIndex,IPC_RMID,NULL) != 0)
		perror("Error at shmctl at gold distrubution main mine ");

	if(shmctl(shmIDGold,IPC_RMID,NULL) != 0)
		perror("Error at shmctl at gold distrubution main mine ");

	if(shmctl(shmIDGoldIndex,IPC_RMID,NULL) != 0)
		perror("Error at shmctl at gold index main mine ");

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

				/*editing directory name for calling receiver function*/
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
	struct UniqueWords founded,*UniqueWords_position,*UniqueWords_temp;
	struct minerInfo minerList,*minerInfo_position,*minerInfo_temp;
	int init=0,i,sem_id,shmIDIndex,shmIDBuffer,*bufferIndex,*goldMessageIndex,shmIDGold,shmIDGoldIndex,tempCoin=0;
	char *buffer,*goldMessage,tempMessage[WORD_SIZE];
	key_t sem_key,shmKeyIndex,shmKeyBuffer,shmKeyGoldMessage,shmKeyGoldMessageIndex;
	struct sembuf sop;

	minerList.pid = -1;

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

	UniqueWords_position = &founded;
	minerInfo_position = &minerList;

	logFile = fopen("serverLogFile","w+");

	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = SEM_UNDO;
	semop(sem_id, &sop, 1);

	while(1){

		if(*bufferIndex <= 0) {
			break;
		}

		if(*bufferIndex >= 0 && init == 0){
			for(i = *bufferIndex-2;i >= 0 && buffer[i] != '\0';--i){
			}
			*bufferIndex = i+1;
			founded.word = malloc(sizeof(char) * strlen(&buffer[*bufferIndex]));
			strncpy(founded.word,&buffer[*bufferIndex],strchr(&buffer[*bufferIndex],' ') - &buffer[*bufferIndex]);
			founded.wordCount= atoi(strchr(&buffer[*bufferIndex],' ')+1);
			++count;
			sprintf(tempMessage,"%s %d",founded.word,founded.wordCount*10);
			tempCoin = tempCoin + founded.wordCount *10;
			strcpy(&goldMessage[*goldMessageIndex],tempMessage);
			*goldMessageIndex += strlen(tempMessage)+1;
		}

		if(*bufferIndex >= 0 && init == 1) {
			do{
				for(i = *bufferIndex-2;i >= 0 && buffer[i] != '\0';--i){
				}

				if(UniqueWords_position->next == NULL)
					break;
				UniqueWords_position = UniqueWords_position->next;
			}while(strcmpTillSpace(UniqueWords_position->word,&buffer[i+1]));

			if(strcmpTillSpace(UniqueWords_position->word,&buffer[i+1]) == 0 && strlen(&buffer[i+1]) >0){
				*bufferIndex=i+1;
				UniqueWords_position->wordCount += atoi(strchr(&buffer[i+1],' ')+1);
				tempCoin = tempCoin + atoi(strchr(&buffer[i+1],' ')+1);
				sprintf(tempMessage,"%s %d", UniqueWords_position->word,atoi(strchr(&buffer[i+1],' ')+1));
				strcpy(&goldMessage[*goldMessageIndex],tempMessage);
				*goldMessageIndex += strlen(tempMessage)+1;
			}

			else if(UniqueWords_position->next == NULL){
				if(buffer[i+1] != '!') {
					UniqueWords_position->next = malloc(sizeof(struct UniqueWords));
					UniqueWords_position = UniqueWords_position->next;

					UniqueWords_position->wordCount = atoi(strchr(&buffer[i + 1], ' ') + 1);
					UniqueWords_position->next = NULL;

					UniqueWords_position->word = malloc(sizeof(char) * strlen(&buffer[i + 1]));
					strncpy(UniqueWords_position->word, &buffer[i + 1], strchr(&buffer[i + 1], ' ') - &buffer[i + 1]);

					sprintf(tempMessage, "%s %d", UniqueWords_position->word, UniqueWords_position->wordCount * 10);
					tempCoin = tempCoin + UniqueWords_position->wordCount *10;
					strcpy(&goldMessage[*goldMessageIndex], tempMessage);
					*goldMessageIndex += strlen(tempMessage) + 1;
					*bufferIndex = i + 1;
					++count;
				}
				else{
					strncpy(tempMessage,&buffer[i+1],strchr(&buffer[i+3],' ') - &buffer[i+1]);
					if(minerList.pid == -1){
						minerList.pid = atoi(&buffer[i+3]);

						i += strchr(&buffer[i+3],' ') + 1 - &buffer[i+3] ;
						minerList.startSecond = atoi(&buffer[i]);
						i += strchr(&buffer[i],' ') -&buffer[i] + 1;
						minerList.startMicrosecond = atoi(&buffer[i]);

						minerList.paidCoin = tempCoin;
						tempCoin = 0;

						minerList.next = NULL;
					}
					else if(minerList.pid != -1){
						if(minerInfo_position->next == NULL)
							minerInfo_position ->next = malloc(sizeof(struct minerInfo));

						minerInfo_position = minerInfo_position->next;

						minerInfo_position->pid = atoi(&buffer[i+3]);
						i += strchr(&buffer[i+3],' ') - &buffer[i+3] + 1;
						minerInfo_position->startSecond = atoi(&buffer[i]);
						i += strchr(&buffer[i],' ') - &buffer[i] + 1;
						minerInfo_position->startMicrosecond = atoi(&buffer[i]);
						minerInfo_position->next = NULL;

						minerInfo_position->paidCoin =tempCoin;
						tempCoin = 0;
					}
					else{
						fprintf(stderr,"There is an error at minerInfo logging\n");
					}
					for(i = *bufferIndex-2;i >= 0 && buffer[i] != '\0';--i){
					}
					strncpy(tempMessage,&buffer[i+1],strchr(&buffer[i+3],' ') - &buffer[i+1]);
					strcat(tempMessage,"!");
					strcpy(&goldMessage[*goldMessageIndex],tempMessage);
					*goldMessageIndex += strlen(tempMessage) + 1;
					*bufferIndex = i + 1;
				}
			}
		}

		UniqueWords_position = &founded;
		init = 1;
	}
	sop.sem_op = 1;
	semop(sem_id, &sop, 1);

	UniqueWords_position = &founded;

	while(UniqueWords_position->next != NULL){
		fprintf(logFile,"%s %d\n", UniqueWords_position->word, UniqueWords_position->wordCount);
		UniqueWords_position = UniqueWords_position->next;
	}

	fprintf(logFile,"%s %d\n", UniqueWords_position->word, UniqueWords_position->wordCount);
	i=0;
	fprintf(logFile,"%d pid numbered miner finished it job at %ld second and %ld microsecond and paid %d coin of gold\n",minerList.pid,minerList.startSecond,minerList.startMicrosecond,minerList.paidCoin);

	for(minerInfo_position = minerList.next; minerInfo_position != NULL; ){
		fprintf(logFile,"%d pid numbered miner finished it job at %ld second and %ld microsecond and paid %d coin of gold\n",minerInfo_position->pid,minerInfo_position->startSecond,minerInfo_position->startMicrosecond,minerInfo_position->paidCoin);
		minerInfo_temp = minerInfo_position;
		if(minerInfo_position->next != NULL)
			free(minerInfo_position);
		minerInfo_position = minerInfo_temp->next;
		++i;
	}

	fprintf(logFile,"Total number of miner served is %d",i);

	for(UniqueWords_position = founded.next; UniqueWords_position != NULL; ){
		UniqueWords_temp = UniqueWords_position;
		free(UniqueWords_temp->word);
		if(UniqueWords_position->next != NULL){
			free(UniqueWords_position);
		}
		UniqueWords_position = UniqueWords_temp->next;
	}

	free(minerInfo_temp);
	free(UniqueWords_temp);
	free(founded.word);

	shmdt(buffer);
	shmdt(bufferIndex);
	shmdt(goldMessage);
	shmdt(goldMessageIndex);

	fclose(logFile);
}


int strcmpTillSpace(const char *input1, const char *input2) {

	while((*input1 != ' ' || *input2 != ' ' )&& *input1 != '\0' && *input2 != '\0'){
		if((int)*input1 -(int)*input2 != 0 )
			return (int)*input1 -(int)*input2 != 0;
		++input1;
		++input2;
	}
	return 0;
}
