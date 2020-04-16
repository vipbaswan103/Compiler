/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	// Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

#ifndef SYMBOLTABLEDEF_H_
#define SYMBOLTABLEDEF_H_

#define INTEGER_SIZE 4
#define REAL_SIZE 8
#define BOOLEAN_SIZE 2
#define POINTER_SIZE 8

#include<stdio.h>
#include<stdlib.h>

#include "astDef.h"

#define INTIALHASHSIZE 100
typedef enum {Identifier = 2, Array = 1, Module = 0} entryType;
typedef enum {Declaration = 2, Type = 1, Others = 0} errorType;

int currentOffset;

//primitive data type 1
typedef struct identifier
{
    char *type;
    char *lexeme;
    void *value;
    int isAssigned;
    int isIndex;
}identifier;

//primitive data type 2
typedef struct array
{
    char *type;
    char *lexeme;
    int isDynamic;
    identifier * lowerIndex;
    identifier * upperIndex;
}array;

// declaration for future structure
struct elementSym;

// primitive data type 3
typedef struct module
{
    char *lexeme;
    int inputcount;
    int outputcount;
    struct elementSym * inputList;
    struct elementSym * outputList;
}module;

//union for the primitive data tyoes
typedef union variable 
{
    identifier id;
    array arr;
    module mod;
}variable;

// wrapper for the above union
typedef struct elementSym
{
    variable data;
    entryType tag;
}elementSym;

// symbol table's node with elements and their metadata
typedef struct symbolTableNode
{
    elementSym ele;
    int aux;
    int scope;      //Depicts the nesting level, TODO (yet to be populated)
    int lineNum;
    int offset;
    int width;
    int isParameter;    //Set when ele is actually an input or output parameter
    struct symbolTableNode * next;
}symbolTableNode;

// linked list ADT of the above elements
typedef struct linkedListSym
{
    symbolTableNode *head;
    symbolTableNode *tail;
    int size;
}linkedListSym;

// hash table of the symbols
// contains arrays of linked lists indexed by the hash
typedef struct hashSym
{
    int hashtbSize;
    int eleCount;
    linkedListSym *arr;
}hashSym;

//symbol table wrapper to for the symbol table tree
typedef struct symbolTable{
    char *symLexeme;
    int lineNumStart;
    int lineNumEnd;
    hashSym hashtb;
    int currentOffset;
    // int sizeActivationRecord;
    struct symbolTable *child;
    struct symbolTable *sibling;
}symbolTable;

// root of the symbol table tree
symbolTable *symbolTableRoot;

// node of stack of the symbol table(s)
typedef struct tableStackEle
{
    symbolTable *ele;
    struct tableStackEle *next;
}tableStackEle;

// stack of the symbol table(s)
typedef struct tableStack
{
    int size;
    tableStackEle *top;
    tableStackEle *bottom;
}tableStack;

typedef struct semanticErrorNode
{
    char * errorMessage;
    struct semanticErrorNode *next;
}semanticErrorNode;

typedef struct semanticError
{
    int numErrors;
    semanticErrorNode* head;
}semanticError;

semanticError* semErrorList;

#endif