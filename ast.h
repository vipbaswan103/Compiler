#include<stdio.h>
#include<stdlib.h>
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

astNode* makeASTnode(char * label, astNode ** childs, int size);
astNode* makeLeafNode(TreeNode * leaf);
astNode * concatenate(astNode * head, astNode * newNode);
astNode * makeListNode(char * label, astNode * list);
astNode * createAST(TreeNode *parseNode, astNode *inh, astNode **syn);
void printAST(astNode * ast, FILE * fp);