/**
* This code written by Kasım Süzen at 24 March 2015
* This is a wc(word count) like program which is wrote for CSE 244's second homework 
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

void usageError();
int isAlpha(char key);
int counter(FILE * input);

int main(int argc,char ** argv){

	DIR *dp;

	if(argc != 2)
		usageError();

	dp = opendir(argv[1]);
	if(dp == NULL){
		fprintf(stderr,"%s named file could not opened either directory does not exist or this user does not have acess to directory \n",argv[1]);
		usageError();
	}
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
* Counts word in the file which given only word counts are consist only alphabetic characters
* @param: input file pointer for file
* return counted words number
*/
int counter(FILE * input){
	int flag=1,numberOfWords=0;
	char temp;
	
	while(!feof(input)){
		fscanf(input,"%c",&temp);

		if(!isAlpha(temp) && temp != ' ' && temp != '\n') /* checking character is alphabetic */
			flag = 0;
		
		/*EOF checked because if there is no space or new line character between eof and last character of word */
		if(temp == ' ' || temp == '\n' || feof(input)){
			
			if(flag == 1) /* if flag equals to 1 characters between previos space and current are alphabetic words so increase word founded */
				++numberOfWords;

			flag = 1; /* reset flag because of space or new line*/
		}
	}
	
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