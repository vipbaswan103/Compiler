/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

#ifndef NAMSCODE_H_
#define NASMCODE_H_

#include "symbolTableDef.h"
#include "symbolTable.h"
#include "semantics.h"
#include "semanticsDef.h"

symbolTableNode *searchScopeIRcode(tableStack *tbStack, char *key);
IRcode* nasmRecur(IRcode* code, tableStack* tbStack, symbolTable * symT, FILE *fp);
void pre_process(FILE * fp);
#endif