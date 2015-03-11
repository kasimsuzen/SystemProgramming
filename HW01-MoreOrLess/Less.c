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
void lineRewind(int lineCount);
void print(char *material,int start,int stop);
int fileRead(char *fileName,char *fileContent);


int main (int argc,char **argv){
	if(argc != 3)
		usageError();
	return 0;
}

/* print lines betweem start and stop line number */
void print(char *material,int start,int stop){
	
}

/* Read file which taken by first argument -fileName- and write contents to fileContent character array*/
int fileRead(char *fileName,char *fileContent){

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