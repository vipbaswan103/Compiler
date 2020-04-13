#ifndef CODEGEN_H_
#define CODEGEN_H_
#include "codegenDef.h"
#include "astDef.h"

void getOp(char * name, char * op);
void getTemporary(temporary * tmp);
void initQuad(quad *ele ,char* arg1, char* arg2, char* result);
void getLabel(char* l);
void initializeFinalCode(intermed* final);
void mergeCode(IRcode * code1, IRcode * code2);
intermed * generateIRCode(astNode * currentNode, quad * labels, tableStack * tbStack);
void printCode(IRcode * code);
#endif