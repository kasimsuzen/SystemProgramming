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

void usageError();
int isAlpha(char key);
int counter(char * fileName);
void crawler(char *rootDirectory);
void resultPrinter(char * pathOfDirectory,int founded);

int main(int argc,char ** argv){

	DIR *dp;

	if(argc != 2)
		usageError();

	dp = opendir(argv[1]);
	if(dp == NULL){
		fprintf(stderr,"%s named file could not opened either directory does not exist or this user does not have acess to directory \n",argv[1]);
		usageError();
	}

	closedir(dp);

	crawler(argv[1]);

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
void crawler(char *rootDirectory){
	pid_t pid,* pids;

	int  i,pidCount=0,count=0,numberOfSubdirectories=0,status,numberOfFile=0;
	DIR *dp;
	struct dirent *ep;
	char ** dirList,**fileList;

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
	if(numberOfFile != 0){
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
	pids = malloc((count - 2)*sizeof(pid_t));
	dp = opendir(rootDirectory);

	if(dp != NULL){
		
		/* start from beginnig of the directory and search till the end of directory pointer */ 
		for (count=0;ep = readdir (dp);++count){

			/* if readed element is directory this function will call itself with this directory */
			if(ep->d_type == DT_DIR && strcmp(".",ep->d_name) != 0 && strcmp("..",ep->d_name) != 0){

				if( (pids[pidCount] = fork()) < 0 ){
					perror("New process could not created this program will be abort\n");
					abort();
				}

				if(pids[pidCount] == 0){
					///*editing directory name for calling crawler function */
					strcpy(dirList[pidCount],rootDirectory);
					strcat(dirList[pidCount],"/");
					strcat(dirList[pidCount],ep->d_name);

					crawler(dirList[pidCount]);

					exit(0);

				}
				++pidCount;

			}

			/* if founded element is not directory it can only be file so name will edit */
			if(ep->d_type == DT_REG && strcmp(ep->d_name,".") != 0 && strcmp(ep->d_name,"..") != 0){
                strcpy(fileList[count - pidCount],rootDirectory);
                strcat(fileList[count - pidCount],"/");
                strcat(fileList[count - pidCount],ep->d_name);
            }
			
			/* Count variable will increase when . .. readed but should not increade because for this there will be process creation */
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
    for (i = pidCount ; i <  pidCount + numberOfFile ; ++i)
    {

        if ((pids[i] = fork()) < 0)
        {
			perror("New process could not created this program will be abort\n");
            abort();
        }
        else if (pids[i] == 0)
        {
            if(strcmp(fileList[count],".") != 0 && strcmp(fileList[count],"..") != 0)
            {
                resultPrinter(fileList[count],counter(fileList[count]));
            }
            exit(0);
        }
        ++count;
    }

    printf("%d process created for %d subdirectories and %d files of %s\n",numberOfSubdirectories + numberOfFile,numberOfSubdirectories,numberOfFile,rootDirectory);


    /* Wait for children to exit. */
    for(i=0;numberOfFile + numberOfSubdirectories > i;++i)
    {
        pid = wait(&status);
    }

    free(pids);

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

void resultPrinter(char * pathOfDirectory,int founded){
	fprintf(stderr,"%d numbered process founded %d word at %s \n",getpid(),founded,pathOfDirectory);
}

/**
* Counts word in the file which given only word counts are consist only alphabetic characters
* @param: input file name for file
* return counted words number
*/
int counter(char * fileName){
	FILE * input;
	int flag=1,flagForExtraSpace=0,numberOfWords=0;
	char temp;

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
	fclose(input);
	return numberOfWords;
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