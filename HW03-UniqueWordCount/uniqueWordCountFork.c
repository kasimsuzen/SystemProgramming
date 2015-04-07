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

struct uniqueWords
{
	char buffer[50];
	int count;
};

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

	int  i,count=0,currentPid=0,numberOfSubdirectories=0,status,allocatedSpace,numberOfFile=0;
	DIR *dp;
	struct dirent *ep;
	char ** itemList;

	dp = opendir(rootDirectory); 
	/* open directory and count all elements inside directory include parent('..') and current directory('.') symbol */
	if (dp != NULL)
	{
		for (count=0;ep = readdir (dp);++count);

		(void) closedir(dp);
	}
	else
		perror ("Couldn't open the directory");
	allocatedSpace = count;
	itemList= malloc(count*sizeof(char*));

	for(i=0 ;i < count;++i)
		itemList[i]= malloc(PATH_MAX*sizeof(char));

	/* allocate pid numbers for each directory and file -2 here for . and .. (current and parent directory symbols) */
	pids = malloc((count - 2)*sizeof(pid_t));

	dp = opendir(rootDirectory);

	if(dp != NULL){
		
		/* start from beginnig of the directory and search till the end of directory pointer */ 
		for (count=0;ep = readdir (dp);++count){

			/* if readed element is directory this function will call itself with this directory */
			if(ep->d_type == DT_DIR && strcmp(".",ep->d_name) != 0 && strcmp("..",ep->d_name) != 0){
					++numberOfSubdirectories;

				if( (pids[numberOfSubdirectories] = fork()) < 0 ){
					perror("New process could not created this program will be abort\n");
					abort();
				}

				if(pids[numberOfSubdirectories] == 0){

					/*editing directory name for calling crawler function*/
					strcpy(itemList[count],rootDirectory);
					strcat(itemList[count],"/");
					strcat(itemList[count],ep->d_name);

					crawler(itemList[count]);
					exit(0);

				}
			}

			/* if founded element is not directory it can only be file so name will edit */
			else if(strcmp(ep->d_name,".") != 0 && strcmp(ep->d_name,"..") != 0){
                strcpy(itemList[count],rootDirectory);
                strcat(itemList[count],"/");
                strcat(itemList[count],ep->d_name);
                ++numberOfFile;
            }

        }

        (void)closedir(dp);
    }
    else
        perror ("Couldn't open the directory");

    /* Start new process for founded files. */
    for (i = currentPid; i < count; ++i)
    {

        if ((pids[i] = fork()) < 0)
        {
			perror("New process could not created this program will be abort\n");
            abort();
        }
        else if (pids[i] == 0)
        {
            if(strcmp(itemList[i],".") != 0 && strcmp(itemList[i],"..") != 0)
            {
                resultPrinter(itemList[i],counter(itemList[i]));
            }
            exit(0);
        }
    }

    printf("%d process created for %d subdirectories and %d files of %s\n",count-2,numberOfSubdirectories,numberOfFile,rootDirectory);


    /* Wait for children to exit. */
    for(i=0;count > i;++i)
    {
        pid = wait(&status);
    }

    free(pids);

    for(i=0; i < allocatedSpace;++i)
        free(itemList[i]);

    free(itemList);
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