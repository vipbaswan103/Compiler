/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

#ifndef AST_H_
#define AST_H_

#include<stdio.h>
#include<stdlib.h>
#include "astDef.h"

astNode* makeASTnode(char * label, astNode ** childs, int size);
astNode* makeLeafNode(TreeNode * leaf);
astNode * concatenate(astNode * head, astNode * newNode);
astNode * makeListNode(char * label, astNode * list);
astNode * createAST(TreeNode *parseNode, astNode *inh, astNode **syn);
void printAST(astNode * ast, FILE * fp);

#endif