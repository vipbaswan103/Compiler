#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include "ast.h"
#include "parser.h"
#define INTIALHASHSIZE 100
typedef enum {Identifier = 2, Array = 1, Module = 0} entryType;

typedef struct identifier
{
    char *type;
    char *lexeme;
    void *value;
}identifier;

typedef struct array
{
    char *type;
    char *lexeme;
    int isDynamic;
    identifier * lowerIndex;
    identifier * upperIndex;
}array;

// typedef union id_arr
// {
//     identifier id;
//     array arr;
// }id_arr;

struct elementSym;
typedef struct module
{
    char *lexeme;
    int inputcount;
    int outputcount;
    struct elementSym * inputList;
    struct elementSym * outputList;
}module;

typedef union variable 
{
    identifier id;
    array arr;
    module mod;
}variable;

typedef struct elementSym
{
    // union
    // {
    //     identifier id;
    //     array arr;
    //     module mod;
    // }data;
    variable data;
    entryType tag;
}elementSym;

typedef struct symbolTableNode
{
    elementSym ele;
    int scope;
    int lineNum;
    int offset;
    int width;
    struct symbolTableNode * next;
}symbolTableNode;

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

symbolTable *symbolTableRoot;

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
