/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/ 

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

#define BUFFERSIZE 4096

typedef struct lexerror
{
    char description[200];
    char lexeme[200];
    int lineNum;
}LexicalErr;

typedef struct syntaxerror
{
    char description[200];
    char lexeme[200];
    int lineNum;
}SyntaxErr;

//Wraps around the lexical and syntactical errors 
typedef union error
{
    LexicalErr lex;
    SyntaxErr syn;
}Error;

//Wraps around the lexical and syntactical errors using the union
typedef struct errnode
{
    int tag;            //1 for LexicalError, 2 for Syntactical
    Error err;
    struct errnode * next;
}ErrorNode;

//Global Lists for error reporting at the end
ErrorNode * LexHead;
ErrorNode * SynHead;


typedef struct
{
	char * token;
	char * lexeme;
	void * value;
	int lineNum;
} Token;


typedef struct{
    char str[60];
    int enumcode;
} Terminal;

typedef struct{
    char str[60];
    int enumcode;
} NonTerminal;

typedef union{
        Terminal t;
        NonTerminal nt;
} TokenType; 

//changable element (wraps the T and NT within itself)
typedef struct{
    int tag;
    TokenType type;
}Element;

//node of the hash chain with element inside it
typedef struct node
{
    Element ele ;
    struct node * next ;
}Node;

// base list
typedef struct{
    Node * head;
    Node * tail;
    int size ;
}LinkedList;

//the head of the linked list
typedef struct
{
    LinkedList arr[HASHSIZE] ;
}Hashtable;

//"keyhash" stores the keyword hash table
Hashtable *keyhash;

//lineNum is a global line  number maintanance variable
extern int lineNum;

//buffer (twins)
char *buffer1, *buffer2;

//accessory variables for the twin buffer smooth functioning
int startptr, reading, toRead;

//global filepointer for opening the file
FILE * fp;

#endif
