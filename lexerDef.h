#ifndef LEXERDEF_H_
#define LEXERDEF_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<ctype.h>

#define HASHSIZE 1000
#define RULESIZE 1000
#define NTSIZE 60

#define BUFFERSIZE 35

char *keyhash[32] ;

typedef struct lexerror
{
    char description[200];
    char lexeme[50];
    int lineNum;
}LexicalErr;

typedef struct syntaxerror
{
    char description[200];
    char lexeme[50];
    int lineNum;
}SyntaxErr;


typedef union error
{
    LexicalErr lex;
    SyntaxErr syn;
}Error;

typedef struct errnode
{
    int tag;            //1 for LexicalError, 2 for Syntactical
    Error err;
    struct errnode * next;
}ErrorNode;

ErrorNode * LexHead;
ErrorNode * SynHead;

typedef struct
{
	char * token;
	char * lexeme;
	void * value;
	int lineNum;
} Token;


extern int lineNum;
char *buffer1, *buffer2;
int startptr, reading, toRead;
FILE * fp;


#endif