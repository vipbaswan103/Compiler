#ifndef PARSERDEF_H_
#define PARSERDEF_H_

#include "lexerDef.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<ctype.h>

#define HASHSIZE 1000
#define RULESIZE 1000
#define NTSIZE 60

extern int enumTerminal, enumNonTerminal, curr_size, epsilonENUM;
char **enumToTerminal;
char **enumToNonTerminal;
Hashtable * hash_tb;


//grammar
typedef struct
{
    int size;
    LinkedList * arr;
}Grammar;

typedef struct nonleaf
{
    NonTerminal nt;
}nonleafNode;

typedef struct leaf
{
    Token tkn;
}leafNode;

typedef union
{
    nonleafNode nonleaf;
    leafNode leaf;
}NodeElement;


//TreeNode can either be a leaf node or a non-leaf node
typedef struct treenode
{
    int tag;
    NodeElement ele;
    struct treenode * sibling;
    struct treenode * child;
}TreeNode;


//Stack contains pointers to the tree nodes
typedef struct stacknode
{
    TreeNode * trnode;
    struct stacknode * next;
}StackNode;

typedef struct{
    StackNode * top;
    int size ;
}Stack;

#endif