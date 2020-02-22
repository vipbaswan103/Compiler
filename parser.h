#ifndef PARSER_H_
#define PARSER_H_

#include "parserDef.h"
#include "lexerDef.h"

void initializeParser();
// int hash_func(char *str);
// Element* hash_find(char * str, Hashtable * hash_tb);
// void hash_insert(Element * ele, Hashtable * hash_tb);
void populateGrammarArray(Grammar * grammar, char * str, int TorNT, int index);
void insertInLinkedList(Grammar * grammar, char * str, int TorNT, int index);
void printGrammar(Grammar * grammar);
Grammar * read_grammar(char * filename);
int ** initializeFirst();
int * calculateFirstSet(Grammar *grammar, int nonTerminal, int ** firstSet);
int ** initializeFollow();
void setOR(int * arr1, int * arr2);
int * calculateFollowSet(Grammar * grammar, int nonTerminal, int ** followSet, int ** firstSet);
void map(Grammar * grammar);
void printMap();
void printFirst(int ** firstSet);
void printFollow(int ** followSet);
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

#endif