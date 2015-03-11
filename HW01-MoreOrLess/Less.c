/**
* This code written by Kasım Süzen at 08 March 2015
* This is a less like program which is wrote for CSE 244's first homework 
*/
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

static struct termios old, new;
char getch(void);
void initTermios(int echo);
char getch_(int echo);
void resetTermios(void);
void usageError();
int moreOrLess(char *fileName,int numberOfLine);
void lineRewind(int lineCount);
void print(char *material,int start,int stop);
char* fileRead(char *fileName,char *fileContent);


int main (int argc,char **argv){
	if(argc != 3)
		usageError();

	moreOrLess(argv[1],atoi(argv[2]));

	return 0;
}

/* This function call print, lineRewind, fileRead function when need and decide when which is needed */
int moreOrLess(char *fileName,int numberOfLine){
	char *fileContent;

	fileContent=fileRead(fileName,fileContent);

	printf("moreOrLess function\n");

	print(fileContent,2,5);

	free(fileContent);
}

/* print lines between start and stop line number */
void print(char *material,int start,int stop){
	int i,position=0;

	for(i=0;position < start;++i){
		if(material[i] == '\n')
			++position;
	}

	for (;position < stop; ++i)
	{
		printf("%c",material[i]);
		if(material[i] == '\n')
			++position;
	}

}

/* Read file which taken by first argument -fileName- and write contents to fileContent character array*/
char* fileRead(char *fileName,char *fileContent){

	int sizeOfFile,i=0;
	FILE * input;

	input = fopen(fileName,"r");

	if(input == NULL){
		fprintf(stderr,"There is no file as %s\n",fileName);
		exit(-1);
	}

	fseek(input, 0, 2);    /* file pointer at the end of file */
    sizeOfFile = ftell(input);

    freopen(fileName,"r",input);

    fileContent=(char*)malloc(sizeOfFile*sizeof(char)+1);

    while(!feof(input)){
    	fscanf(input,"%c",&fileContent[i]);
    	++i;
    }

    fileContent[i]='\0';

    return fileContent;
}

/* Initialize new terminal i/o settings */
void initTermios(int echo){
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	new = old; /* make new settings same as old settings */
	new.c_lflag &= ~ICANON; /* disable buffered i/o */
	new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
	tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void){
	tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo){
	char ch;
	initTermios(echo);
	ch = getchar();
	resetTermios();
	return ch;
}

/* Read 1 character without echo */
char getch(void){
	return getch_(0);
}

void usageError(){
	printf("\nParameter Error this program should run as:\n");
	printf("Less \"fileName\" lineNumber \n");
	exit(-1);
}

/* Rewind stdout - terminal output - as much as lineCount */
void lineRewind(int lineCount){
	int i;
	
	for(i=0; i < lineCount; ++i)
		fputs("\033[A\033[2K",stdout);
	
	rewind(stdout); /* will rewind stdout to the very beginning */
	ftruncate(1,0); /* will truncate stdout fully*/
}