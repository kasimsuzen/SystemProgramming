/**
* Written by Kasım Süzen at 14/04/2015
* This is midterm project for CSE 244 class at GTU
*/

int Operation1(char * argument1, char * argument2,char * argument3);
int Operation2(char * argument1, char * argument2);
int Operation3(char * argument1, char * argument2,char * argument3);
int Operation4(char * argument1, char * argument2,char * argument3,char * argument4);

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int main(int argc,char ** argv){

}

/**
* This function make predefined operation as ((a^2 +b^2)^1/2)/abs(c)
* @param: variable a as string
* @param: variable b as string
* @param: variable c as string
* Return: returns 0 incase success -1 for divide by zero
*/
int Operation1(char * argument1, char * argument2,char * argument3){
	double variable1,variable2,variable3,result;

	variable1 = atof(argument1);
	variable2 = atof(argument2);
	variable3 = atof(argument3);

	if(variable3 == 0)
		return -1;

	result = sqrt(pow(variable1,2) + pow(variable2,2))/fabs(variable3);

	return 0;
}


/**
* This function make predefined operation as sqrt(a+b)
* @param: variable a as string
* @param: variable b as string
* Return: returns 0 incase success -1 for sum of variables lesser than zero
*/
int Operation2(char * argument1, char * argument2){
	double variable1,variable2,result;

	variable1 = atof(argument1);
	variable2 = atof(argument2);

	if(variable1 + variable2 < 0)
		return -1;

	result = sqrt(variable1 + variable2);

	return 0;
}

/**
* This function make predefined operation as roots of a*x^2 +b*x + c
* @param: variable a as string
* @param: variable b as string
* @param: variable c as string
* Return: returns 0 in case of success -1 for discriminant is lesser than 0
*/
int Operation3(char * argument1, char * argument2,char * argument3){
	double variable1,variable2,variable3,result;

	variable1 = atof(argument1);
	variable2 = atof(argument2);
	variable3 = atof(argument3);

	result = variable2 * variable2 - 4 * variable1 * variable3;


	if(result < 0)
		return -1;

	return 0;
}

/**
* This function make predefined operation as inverse function (a*x + b)/(c*x +d) as string
* @param: variable a as string
* @param: variable b as string
* @param: variable c as string
* @param: variable d as string
* Return: returns 0 incase success -1 for divide by zero
*/
int Operation4(char * argument1, char * argument2,char * argument3,char * argument4){
	char result[100],temp[25];
	double variable1,variable2,variable3;

	variable1 = atof(argument1);
	variable2 = atof(argument4);
	variable3 = atof(argument3);

	if( variable1 == 0 && variable3 == 0)
		return -1;

	strcpy(result,"(");
	sprintf(temp,"%lf",-1 * variable2);
	strcat(result,temp);
	strcat(result,"*x + ");

	memset(temp,'\0',25);

	strcat(result,argument2);
	strcat(result,") / (");

	strcat(result,argument3);
	strcat(result,"*x + ");
	
	sprintf(temp,"%lf",-1 * variable1);
	strcat(result,temp);
	strcat(result,")");

	printf("%s\n",result);
	return 0;
}