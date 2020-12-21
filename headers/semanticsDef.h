/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/    

#ifndef SEMANTICSDEF_H_
#define SEMANTICSDEF_H_

#include "symbolTableDef.h"

typedef enum {ArrayType = 1, IdentifierType = 0} typeTag; 

typedef struct arrayType
{
    char *basicType;
    identifier *lowerBound;
    identifier *upperBound;
}arrayType;

typedef union whichType
{
    arrayType arr;
    char *type;
}whichType;

typedef struct type
{
    whichType tp;
    typeTag tag;
}type;

#endif