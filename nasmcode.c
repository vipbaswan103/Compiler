#include "codegenDef.h"
#include "symbolTableDef.h"


void nasmCode(IRcode* code, tableStack* tbbStack)
{
    // if(arg1 find in synmol table) load AX, arg1
    // else MOV AX, arg1
    // if(arg2 find in synmol table)  load BX, arg2
    // else MOV BX, arg2
    // ADD CX,BX, AX
    // STORE result, CX

    IRcode *trav = code;
    while(trav!=NULL)
    {
        if(!strcmp(trav->ele->op,"+"))
        {
            ;
        }
        trav = trav->next;
    }
}