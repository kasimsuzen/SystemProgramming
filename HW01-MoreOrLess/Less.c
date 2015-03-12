/**
* This code written by Kasım Süzen at 08 March 2015
* This is a less like program which is wrote for CSE 244's first homework 
*/
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>

static struct termios old, new;
char getch(void);
void initTermios(int echo);
char getch_(int echo);
void resetTermios(void);
void usageError();
int moreOrLess(char *fileName,int numberOfLine);
void lineRewind(int lineCount);
int print(char *material,int start,int stop);
char* fileRead(char *fileName,char *fileContent);
int getTerminalWidth();
int maxLineNumber(char *material);

int main (int argc,char **argv){
	if(argc != 3)
		usageError();

	moreOrLess(argv[1],atoi(argv[2]));

	return 0;
}

/* This function call print, lineRewind, fileRead function when need and decide when which is needed */
int moreOrLess(char *fileName,int numberOfLine){
	
	char *fileContent,bufferForStdin[4];
	int printedLineOnTerminal,topLine=0,botLine=0,maxLine;

	fileContent=fileRead(fileName,fileContent);

	setbuf(stdin,bufferForStdin);

	topLine=1;
	botLine=numberOfLine;

	printedLineOnTerminal=print(fileContent,topLine,botLine);
	printf(" ");

	while(1){

		maxLine=maxLineNumber(fileContent);

		memset(bufferForStdin,'\0',8);	
		getch();

		if(bufferForStdin[0] == 'q' || bufferForStdin[0] == 'Q')
			break;

		if(bufferForStdin[0] == '\n'){
			if(botLine < maxLine){
	        	++topLine;
	        	++botLine;
	        	lineRewind(printedLineOnTerminal);
	        	printedLineOnTerminal = print(fileContent,topLine,botLine);
			}
		}

		if(bufferForStdin[1] == 91){
			getch(); /* skip the [ */
		    switch(getch()) { /* the real value*/
		        case 'A':
		            /* code for arrow up*/		        	
		        	if(numberOfLine > 1){
		        		lineRewind(printedLineOnTerminal);
			        	--topLine;
			        	--botLine;
		        		printedLineOnTerminal = print(fileContent,topLine,botLine);
		        	}		        	
		            break;
		        case 'B':
		            // code for arrow down
		        	if(botLine < maxLine){
			        	++topLine;
			        	++botLine;
			        	lineRewind(printedLineOnTerminal);
			        	printedLineOnTerminal = print(fileContent,topLine,botLine);
			        }
		            break;
		    }
		}
		else{
			if(botLine < maxLine){
				
				if(botLine + numberOfLine > maxLine){
					topLine = maxLine - numberOfLine;
					botLine = maxLine;
				}

				topLine += numberOfLine;
				botLine += numberOfLine;
				lineRewind(printedLineOnTerminal);
		    	printedLineOnTerminal = print(fileContent,topLine,botLine);
				
		    }
		}
	}
	
	free(fileContent);
}

/* print lines between start and stop line number */
int print(char *material,int start,int stop){
	int i,j,position=0,lineCount=1,widthOfTerminal=0;

	widthOfTerminal = getTerminalWidth();

	for(i = 0; lineCount < start; ++i)
	{
		if(material[i] == '\n')
			++lineCount;

		if (i != 0 && i % widthOfTerminal == 0 )
			++lineCount;
	}

	lineCount = 1;
	
	for (j=1; start + lineCount <= stop+1; ++i)
	{
		if(material[i] == '\0')
			break;
		printf("%c",material[i]);

		if (j % widthOfTerminal ==0){
			++lineCount;
			//printf("<");
		}

		++j;

		if(material[i] == '\n'){
			++position;
			++lineCount;
			j=1;
			//printf("|");
		}
	}

	return lineCount;

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
	
	for(i=0; i < lineCount-1; ++i){
		printf("\033[A");
		printf("\033[2K");
		printf("\r");
	}
}

/* This function's code written by a stackoverflow.om user */
int getTerminalWidth(){

	struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
}

int maxLineNumber(char *material){

	int i,newLines=0;

	for(i=0; material[i] != '\0';++i){
		if(material[i] == '\n')
			++newLines;
	}

	return (i / getTerminalWidth())+newLines;
}
