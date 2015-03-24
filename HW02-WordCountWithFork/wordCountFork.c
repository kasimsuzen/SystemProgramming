/**
* This code written by Kasım Süzen at 24 March 2015
* This is a wc(word count) like program which is wrote for CSE 244's second homework 
*/
#include <stdio.h>
#include <stdlib.h>

void usageError();

int main(int argc,char ** argv){

	if(argc != 2)
		usageError();
}

void usageError(){
	fprintf(stderr,"Wrong call, should be as:\n");
	fprintf(stderr,"./wordCount -directoryName\n");
	exit(-1);
}