#ifndef SYMBOLTABLE_H_
#define SYMBOLTABLE_H_
#include "symbolTableDef.h"

void sympush(tableStack *stack, tableStackEle *newNode);
tableStackEle * sympop(tableStack *stack);
int sym_hash_func(hashSym *hashtb,char *str);
symbolTableNode* sym_hash_find(char * str, hashSym * hash_tb, int replace, symbolTableNode * key);
symbolTableNode* sym_hash_insert(symbolTableNode * newNode, hashSym * hash_tb);
hashSym *rehash(hashSym *oldTable);
void initializeHashSym(hashSym *hash_tb);
symbolTable* initializeSymbolTable(char *str, int lineNumStart, int lineNumEnd);
void formulation(astNode* astRoot, symbolTable * current);
void printSymbolTable(symbolTable *root, int level);
void printSymTableNode(symbolTable *ST, int level);
void printHashTable(hashSym hashtb, int level, char* scopename, int linestart, int lineend);
void initializeErrorList();
void insertSemError(semanticErrorNode *errNode);
void printSemanticErrors();
int gimme_module(char*str, char*name);
void activationRecordSize(symbolTable *root, int level);
#endif