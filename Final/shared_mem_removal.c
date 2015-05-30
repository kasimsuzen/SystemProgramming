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

int main(int argc,char ** argv){

	int sem_id,shmIDFileIndex,shmIDFile,shmIDGold,shmIDGoldIndex,shmIDFound,shmIDFoundIndex;
	key_t sem_key,shmKeyFileIndex,shmKeyFile,shmKeyGoldMessage,shmKeyGoldMessageIndex,shmKeyFound,shmKeyFoundIndex;

	shmKeyFileIndex = ftok(".",SHARED_KEY_FOR_FILE_LIST_INDEX);
	shmKeyFoundIndex = ftok(".",SHARED_KEY_FOR_FOUNDED_INDEX);
	shmKeyGoldMessageIndex = ftok(".",SHARED_KEY_FOR_GOLD_INDEX);

	shmKeyFile = ftok(".",SHARED_KEY_FOR_FILE_LIST);
	shmKeyFound = ftok(".",SHARED_KEY_FOR_FOUNDED_WORD);
	shmKeyGoldMessage = ftok(".",SHARED_KEY_FOR_GOLD);

	shmIDFile = shmget(shmKeyFile,sizeof(char) * WORD_COUNT_LIMIT * WORD_SIZE,0);
	shmIDFound = shmget(shmKeyFound,sizeof(char) * WORD_COUNT_LIMIT * WORD_SIZE,0);
	shmIDGold = shmget(shmKeyGoldMessage, sizeof(char) * WORD_COUNT_LIMIT * WORD_SIZE, 0);

	shmIDFileIndex = shmget(shmKeyFileIndex,sizeof(int),0);
	shmIDFoundIndex = shmget(shmKeyFoundIndex,sizeof(int),0);
	shmIDGoldIndex = shmget(shmKeyGoldMessageIndex,sizeof(int),0);

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

	if (semctl(sem_id, 0, IPC_RMID) < 0) 
		perror("Could not delete semaphore");

	if(shmctl(shmIDFile,IPC_RMID,NULL) != 0) 
		perror("Error at shmctl at message main mine");

	if(shmctl(shmIDFileIndex,IPC_RMID,NULL) != 0) 
		perror("Error at shmctl at message main mine");

	if(shmctl(shmIDFound,IPC_RMID,NULL) != 0) 
		perror("Error at shmctl index main mine");

	if(shmctl(shmIDFoundIndex,IPC_RMID,NULL) != 0)
		perror("Error at shmctl at gold distrubution main mine ");

	if(shmctl(shmIDGold,IPC_RMID,NULL) != 0)
		perror("Error at shmctl at gold index main mine ");

	if(shmctl(shmIDGoldIndex,IPC_RMID,NULL) != 0)
		perror("Error at shmctl at gold index main mine ");
}