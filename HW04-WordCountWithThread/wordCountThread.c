/**
* This code written by Kasım Süzen at 24 March 2015
* This is a wc(word count) like program which is wrote for CSE 244's fourth homework 
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <linux/limits.h>

int countOfFiles;
int countOfSubdirectories;
int sizeOfFoundData;
int indexOfFoundData;
char ** foundData;

pthread_mutex_t mainMutex;

void usageError();
int isAlpha(char key);
void * counter(void * data);
void * crawler(void * rootDirectoryName);
void resultPrinter(char * directoryName);

int main(int argc,char ** argv){
	int i;
	DIR *dp;
	char * initialDirName;

	if(argc != 2)
		usageError();

	dp = opendir(argv[1]);
	if(dp == NULL){
		fprintf(stderr,"%s named file could not opened either directory does not exist or this user does not have acess to directory \n",argv[1]);
		usageError();
	}

	closedir(dp);
	/* Initialization global values */
	countOfFiles = 0;
	countOfSubdirectories = 0;
	sizeOfFoundData = 100;
	indexOfFoundData = 0;

	foundData = malloc(sizeof(char*)*sizeOfFoundData);

	for(i = 0; i < sizeOfFoundData;++i)
		foundData[i] = malloc(sizeof(char)*PATH_MAX);

	initialDirName = malloc(PATH_MAX * sizeof(char));
	strcpy(initialDirName,argv[1]);

	crawler((void*)initialDirName);

	resultPrinter(initialDirName);

	free(initialDirName);

	for(i = 0; i < sizeOfFoundData;++i)
		free(foundData[i]);

	free(foundData);

	return(0);
}

/**
* If there are any error while running program this function will be printed
*/
void usageError(){
	fprintf(stderr,"Wrong call, should be as:\n");
	fprintf(stderr,"./wordCount directoryName\n");
	exit(-1);
}

/**
* This function will crawl through and into directories and their subdirectories
* @param: name of the root directory which search will begin
*/
void * crawler(void  *rootDirectoryName){
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

				//fprintf(stderr,"%s gidilecek yer %ld \n",dirList[threadCount],pthread_self());
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

	pthread_mutex_lock(&mainMutex);

	countOfFiles = countOfFiles + numberOfFile;
	countOfSubdirectories = countOfSubdirectories + numberOfSubdirectories;

	pthread_mutex_unlock(&mainMutex);

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
	
}

/**
* Counts word in the file which given only word counts are consist only alphabetic characters
* @param: input file name for file
* return counted words number
*/
void *counter(void * data){
	FILE * input;
	int flag=1,flagForExtraSpace=0,numberOfWords=0;
	char temp,fileName[PATH_MAX];
	struct timeval start, end;
	long  seconds, useconds;

	gettimeofday(&start, NULL);

	strcpy(fileName,(char*)data);

	input = fopen(fileName,"r");

	while(!feof(input)){
		fscanf(input,"%c",&temp);

		if(!isAlpha(temp) && temp != ' ' && temp != '\n'){ /* checking character is alphabetic */
			flag = 0;
		}

		if(isAlpha(temp))
			flagForExtraSpace=0; /* after one alphabetic character read we know a word has ocurred so space flag turned to true */

		/*EOF checked because if there is no space or new line character between eof and last character of word */
		if(temp == ' ' || temp == '\n' || feof(input)){

			if(flag == 1 && flagForExtraSpace ==0){ /* if flag equals to 1 characters between previos space and current are alphabetic words so increase word founded */
				++numberOfWords;
				flagForExtraSpace = 1;
			}

			flag = 1; /* reset flag because of space or new line*/
		}
	}

	gettimeofday(&end, NULL);
	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;

	fclose(input);
	pthread_mutex_lock(&mainMutex);
	if(indexOfFoundData >= sizeOfFoundData){
		sizeOfFoundData = sizeOfFoundData * 2;
		foundData = realloc(foundData,sizeof(char*)*sizeOfFoundData);
	}

	if(indexOfFoundData < sizeOfFoundData){
		sprintf(foundData[indexOfFoundData],"%ld numbered thread found %d words at %s in %ld second %lu nanosecond\n",pthread_self(),numberOfWords,fileName,seconds,useconds);
		++indexOfFoundData;
	}

	pthread_mutex_unlock(&mainMutex);
}

/**
* Prints result of search and count
* @param: Directory name that search begin
*/
void resultPrinter(char *directoryName) {
	int i;
	for(i = 0; i < indexOfFoundData; ++i)
		fprintf(stderr,"%s",foundData[i]);

	fprintf(stderr,"Total thread created is %d number of file is %d number of subdirectories is %d under %s\n",countOfSubdirectories+countOfFiles,countOfFiles,countOfSubdirectories,directoryName);
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