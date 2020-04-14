#ifndef NAMSCODE_H_
#define NASMCODE_H_

#include "symbolTableDef.h"
#include "symbolTable.h"
#include "semantics.h"
#include "semanticsDef.h"

symbolTableNode *searchScopeIRcode(tableStack *tbStack, char *key);
IRcode* nasmRecur(IRcode* code, tableStack* tbStack, symbolTable * symT);


#endif