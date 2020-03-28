#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define INTIALHASHSIZE 100
typedef enum {Identifier = 2, Array = 1, Module = 0} entryType;

// only declaration of struct
struct elementSym;

// primitive data type 1
typedef struct identifier
{
    char *type;
    char *lexeme;
    int value;
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

// primitive data type 3
typedef struct module
{
    char *lexeme;
    int inputcount;
    int outputcount;
    elementSym * inputList;
    elementSym * outputList;
}module;

// union of all data types
typedef union variable 
{
    identifier id;
    array arr;
    module mod;
}variable;

// wrapper for the union above
typedef struct elementSym
{
    variable data;
    entryType tag;
}elementSym;

//Node of the symbol table with the element and meta data
typedef struct symbolTableNode
{
    elementSym ele;
    int scope;
    int lineNum;
    int offset;
    int width;
    struct symbolTableNode * next;
}symbolTableNode;

// 
typedef struct linkedListSym
{
    symbolTableNode *head;
    symbolTableNode *tail;
    int size;
}linkedListSym;

typedef struct hashSym
{
    int hashtbSize;
    int eleCount;
    linkedListSym *arr;
}hashSym;

typedef struct symbolTable{
    char *symLexeme;
    int lineNumStart;
    int lineNumEnd;
    hashSym hashtb;
    struct symbolTable *child;
    struct symbolTable *sibling;
}symbolTable;

typedef struct tableStackEle
{
    symbolTable *ele;
    struct tableStackEle *next;
}tableStackEle;

typedef struct tableStack
{
    int size;
    tableStackEle *top;
    tableStackEle *bottom;
}tableStack;

symbolTable *symbolTableRoot;