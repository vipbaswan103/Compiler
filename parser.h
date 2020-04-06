/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

#ifndef PARSER_H_
#define PARSER_H_

#include "parserDef.h"
#include "lexerDef.h"

void initializeParser();
void populateGrammarArray(Grammar * grammar, char * str, int TorNT, int index);
void insertInLinkedList(Grammar * grammar, char * str, int TorNT, int index);
void printGrammar(Grammar * grammar);
Grammar * read_grammar(char * filename);
int ** initializeFirst();
void calculateFirstSet(Grammar *grammar, int ** firstSet, int ** firstEquations);
void calculateFirstEquations(Grammar *grammar, int ** firstSet, int **firstEquations);
int ** initializeFollow();
int setOR(int * arr1, int * arr2);
void calculateFollowSet(Grammar * grammar, int ** followSet, int ** followEquations);
void calculateFollowEquations(Grammar * grammar, int ** followSet, int ** firstSet, int ** followEquations);
void map(Grammar * grammar);
void printMap();
void printFirst(int ** firstSet);
void printFollow(int ** followSet);
void printFirstEquations();
void printFollowEquations();
int ** intializeParseTable();
void createParseTable(Grammar *grammar, int **parseTable, int **firstSet, int **followSet);
void printParseTable(Grammar *grammar,int ** parseTable);
TreeNode * siblingInsert(TreeNode * head, TreeNode * node);
void insert(TreeNode * parent, TreeNode * newNode);
void inOrder(FILE * fp, TreeNode * root, TreeNode * parent);
void printTokenStream(TreeNode * root);
TreeNode * pop(Stack * st);
Stack * push(Stack * st, TreeNode * trNode);
void printStack(Stack * st);
TreeNode * parser(Grammar * grammar, int ** parsetable);
void freeprasetree(TreeNode *root);
void freerule(Node* trav);
void freegrammar(Grammar* root);

#endif
