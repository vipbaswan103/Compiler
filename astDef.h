/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

#ifndef ASTDEF_H_
#define ASTDEF_H_

#include "parserDef.h"

typedef enum {Leaf = 1, Internal = 0} nodeType; 

typedef struct leaf
{
    char * lexeme;
    void * value;
    char * type;
    int lineNum;
} leaf;

typedef struct internal
{
    char * label;
    int lineNumStart;
    int lineNumEnd;
} internal;

typedef union nodeEle
{
    leaf * leafNode;
    internal * internalNode;
} nodeEle;

typedef struct astEle
{
    nodeType tag;
    nodeEle ele;
} astEle;

typedef struct astNode
{
    astEle * node;
    struct astNode * sibling;
    struct astNode * child;
}astNode;

#endif