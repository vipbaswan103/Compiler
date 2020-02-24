
/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

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

extern int enumTerminal, enumNonTerminal, epsilonENUM;
char **enumToTerminal;
char **enumToNonTerminal;

//this hash_tb stores the enumeration for the terminals and non terminal
Hashtable * hash_tb;


// "Grammar" is an array of LinkedList nodes
// LinkedList nodes are defined in lexerDef.h
// They have the head and tail of the LinkedList in front of them
// While "size" variable in Grammar structure will thus store the number of rules,
// The "size" variable of the LinkenList stored the number of elements in the linked list (minus one),
// which in this case is the rule's length of onlt RHS side 
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

//union to wrap around leaf and non leaf structures of the syntax tree
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
    
    // The first left most child and the sibling is stored
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
