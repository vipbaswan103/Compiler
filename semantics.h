#ifndef SEMANTICS_H_
#define SEMANTICS_H_

#include "astDef.h"
#include "symbolTableDef.h"
#include "semanticsDef.h"

int listCount(astNode* head);
symbolTableNode *searchScope(tableStack *tbStack, astNode *key);
type * typeChecker(astNode * currentNode, tableStack * tbStack);

#endif