/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

#ifndef SYMBOLTABLE_H_
#define SYMBOLTABLE_H_

#include "symbolTableDef.h"

void sympush(tableStack *stack, tableStackEle *newNode);
tableStackEle * sympop(tableStack *stack);
int sym_hash_func(hashSym *hashtb,char *str);
symbolTableNode* sym_hash_find(char * str, hashSym * hash_tb);
void sym_hash_insert(symbolTableNode * newNode, hashSym * hash_tb);
hashSym *rehash(hashSym *oldTable);
void initializeHashSym(hashSym *hash_tb);
symbolTable* initializeSymbolTable(char *str, int lineNumStart, int lineNumEnd);
void formulation(astNode* astRoot, symbolTable * current);

#endif