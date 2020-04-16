#include "codegenDef.h"
#include "symbolTableDef.h"
#include "symbolTable.h"
#include "semantics.h"
#include "codegen.h"
#include "nasmcode.h"

symbolTableNode *searchScopeIRcode(tableStack *tbStack, char *key)
{
    //initialise the temporary stack
    tableStack *tempStack = (tableStack *)malloc(sizeof(tableStack));
    tempStack->top = NULL;
    tempStack->size = 0;
    tempStack->bottom = NULL;

    symbolTableNode *ret = NULL;
    tableStackEle *temp = NULL;
    
    //its not the scope of module declarations and I did not find anything, put it on the other stack
    while((ret = sym_hash_find(key, &(tbStack->top->ele->hashtb), 0, NULL)) == NULL)
    {
        temp = sympop(tbStack);
        sympush(tempStack, temp);
    }
    //Push everything in tempStack into the tbStack
    while(tempStack->size  != 0)
    {
        temp = sympop(tempStack);
        sympush(tbStack, temp);
    }
    return ret;
}

void pre_process(FILE * fp)
{
    //fprintf(fp,"extern printf;\n");
    
    fprintf(fp,"SECTION .data;\n");
    fprintf(fp,"\tprintmessage: db \"The number is: %%d\", 10, 0\n");
    fprintf(fp,"\tscanmessage: db \"Enter the number:\", 0\n");
    fprintf(fp,"\t_booleanMem: db 1d\n");
    fprintf(fp,"\t_integerMem: dw 24d\n");
    fprintf(fp,"\t_integerMsg: db \"Enter an integer value\", 10, 0\n");
    fprintf(fp,"\t_booleanMsg: db \"Enter a boolean value\", 10, 0\n");
    fprintf(fp,"\t_realMsg: db \"Enter a real value\", 10, 0\n");
    fprintf(fp,"\t_output: db \"Output:\", 0\n");
    fprintf(fp,"\t_percentD: db \"%%d\", 0\n");
    fprintf(fp,"\t_percentS: db \"%%s\", 0\n");
    fprintf(fp,"\t_true: dw 1d\n");
    fprintf(fp,"\t_false: dw 0d\n");
    fprintf(fp,"\t_arrayInputString: db \"Input: Enter %%d array elements of %%s type for range %%d to %%d\", 10, 0\n\n");
    fprintf(fp,"SECTION .txt\n\t GLOBAL main \n\t extern printf \n\t extern scanf \n");

}

IRcode* nasmRecur(IRcode* code, tableStack* tbStack, symbolTable * symT, FILE * fp)
{
    
    // tmp keeps a track of which children we go inside
    // when we start a new scope
    symbolTable * tmp ;
    
    if(symT == symbolTableRoot)
        tmp = symT->child->sibling; // MODULES1
    else
        tmp = symT->child; // MODULEDEC
    
    // the current IRcode we are on
    IRcode * trav = code;

    tableStackEle* newNode = NULL;
    
    // keep going till the end of the loop
    while(trav != NULL)
    {
        if(!strcmp(trav->ele->op,"SCOPESTARTMODULE"))
        {
            newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
            newNode->ele = tmp;
            newNode->next = NULL;
            sympush(tbStack, newNode);

            newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
            newNode->ele = tmp->child;
            newNode->next = NULL;
            sympush(tbStack, newNode);

            trav = trav->next;
            trav = nasmRecur(trav, tbStack, tmp->child, fp);
            tmp = tmp->sibling;
        }
        else if(!strcmp(trav->ele->op,"SCOPESTART"))
        {
            newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
            newNode->ele = tmp;
            newNode->next = NULL;
            sympush(tbStack, newNode);
            trav = trav->next;
            trav = nasmRecur(trav, tbStack, tmp, fp);
            tmp = tmp->sibling;
        }
		else if(!strcmp(trav->ele->op,"SCOPEENDMODULE"))
        {
            sympop(tbStack);
            sympop(tbStack);
            return trav;
        }
        else if(!strcmp(trav->ele->op,"SCOPEEND"))
		{
            sympop(tbStack);
			return trav;
		}
        else if(!strcmp(trav->ele->op,"+"))
        {
            // _t0 = + _t1
            // unary operator
            if(!strcmp(trav->ele->arg2, "\0"))
            {
                symbolTableNode * res = searchScopeIRcode(tbStack, trav->ele->result);
                if(trav->ele->tag1 == NUM)
                {
                    fprintf(fp, "\tMOV AX, %sd\n",trav->ele->arg1); 

                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], AX\n", res->offset);     
                    else
                        fprintf(fp,"\tMOV [EBP+%d], AX\n", res->offset); 
                    
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // fprintf(fp,"\tMOV EAX, %sf",trav->ele->arg1); 

                    // if(res->isParameter == 0)
                    //     fprintf(fp,"\tMOV [EBP-8-%d], EAX", res->offset);
                    // else
                    //     fprintf(fp,"\tMOV [EBP+%d], EAX", res->offset);
                }
                else
                {
                    symbolTableNode * ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret->ele.data.id.type, "INTEGER"))
                    {
                        if(ret->isParameter == 0)
                            fprintf(fp,"\tMOV AX, [EBP-8-%d]\n",ret->offset);
                        else
                            fprintf(fp,"\tMOV AX, [EBP+%d]\n",ret->offset);
                        
                        if(res->isParameter == 0)
                            fprintf(fp,"\tMOV [EBP-8-%d], AX\n", res->offset);
                        else
                            fprintf(fp,"\tMOV [EBP+%d], AX\n", res->offset);
                    }
                    else
                    {
                        
                    }       
                }
            }
            // _t0 = _t1 + _t2
            // binary operator
            else
            {
                symbolTableNode *res = searchScopeIRcode(tbStack, trav->ele->result);
                
                if(trav->ele->tag1 == NUM)
                {
                    fprintf(fp,"\tMOV AX, %sd\n",trav->ele->arg1); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // fprintf(fp,"\tMOV EAX, %sd\n",trav->ele->arg1); 
                }
                else
                {
                    // Its an ID
                    symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret1->ele.data.id.type, "INTEGER"))
                    {
                        if(ret1->isParameter == 0)
                            fprintf(fp,"\tMOV AX, [EBP-8-%d]\n",ret1->offset);
                        else
                            fprintf(fp,"\tMOV AX, [EBP+%d]\n",ret1->offset);
                    }
                    else
                    {
                        // fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret1->offset);
                    }
                }
                if(trav->ele->tag2 == NUM)
                {
                    fprintf(fp,"\tMOV BX, %sd\n",trav->ele->arg2); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                }
                else
                {
                    symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                    if(!strcmp(ret2->ele.data.id.type, "INTEGER"))
                    {
                        if(ret2->isParameter == 0)
                            fprintf(fp,"\tMOV BX, [EBP-8-%d]\n",ret2->offset);
                        else
                            fprintf(fp,"\tMOV BX, [EBP+%d]\n",ret2->offset);
                    }
                    else
                    {
                        // fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                    }
                }
                
                if(!strcmp(res->ele.data.id.type,"INTEGER"))
                {    
                    fprintf(fp,"\tADD AX, BX\n");

                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], AX\n", res->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], AX\n", res->offset);
                }
                else
                {
                    // Real Operation
                }   
            }
        }
        else if(!strcmp(trav->ele->op,"-"))
        {
            // _t0 = - _t1
            if(!strcmp(trav->ele->arg2, "\0"))
            {
                symbolTableNode * res = searchScopeIRcode(tbStack, trav->ele->result);
                if(trav->ele->tag1 == NUM)
                {
                    fprintf(fp,"\tMOV AX, %sd\n", trav->ele->arg1);
                    fprintf(fp,"\tNEG AX\n");

                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], AX\n", res->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], AX\n", res->offset);
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    
                }
                else
                {
                    symbolTableNode * ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret->ele.data.id.type, "INTEGER"))
                    {
                        if(ret->isParameter == 0)
                            fprintf(fp,"\tMOV AX, [EBP-8-%d]\n",ret->offset);
                        else
                            fprintf(fp,"\tMOV AX, [EBP+%d]\n",ret->offset);
                        fprintf(fp,"\tNEG AX\n");

                        if(res->isParameter == 0)
                            fprintf(fp,"\tMOV [EBP-8-%d], AX\n", res->offset);
                        else
                            fprintf(fp,"\tMOV [EBP+%d], AX\n", res->offset);
                    }
                    else
                    {
                        // REAL OP
                    }   
                }
            }
            // _t0 = _t1 - _t2
            else
            {
                symbolTableNode *res = searchScopeIRcode(tbStack, trav->ele->result);
                
                if(trav->ele->tag1 == NUM)
                {
                    fprintf(fp,"\tMOV AX, %sd\n",trav->ele->arg1); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                }
                else
                {
                    symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret1->ele.data.id.type, "INTEGER"))
                    {
                        if(ret1->isParameter == 0)
                            fprintf(fp,"\tMOV AX, [EBP-8-%d]\n",ret1->offset);
                        else
                            fprintf(fp,"\tMOV AX, [EBP+%d]\n",ret1->offset);
                    }
                    else
                    {
                        // fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret1->offset);
                    }
                }
                if(trav->ele->tag2 == NUM)
                {
                    fprintf(fp,"\tMOV BX, %sd\n",trav->ele->arg2); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                }
                else
                {
                    symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                    if(!strcmp(ret2->ele.data.id.type, "INTEGER"))
                    {
                        if(ret2->isParameter == 0)
                            fprintf(fp,"\tMOV BX, [EBP-8-%d]\n",ret2->offset);
                        else
                            fprintf(fp,"\tMOV BX, [EBP+%d]\n",ret2->offset);
                    }
                    else
                    {
                        // fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                    }
                }
                
                if(!strcmp(res->ele.data.id.type,"INTEGER"))
                {    
                    fprintf(fp,"\tSUB AX, BX\n");
                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], AX\n", res->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], AX\n", res->offset);
                }
                else
                {
                    // Real Operation
                }
            }
        }
        else if(!strcmp(trav->ele->op,"*"))
        {
            symbolTableNode *ret1, *ret2, *res; 
            
            res = searchScopeIRcode(tbStack, trav->ele->result);
            if(trav->ele->tag1 == NUM)
            {
                fprintf(fp,"\tMOV AX, %sd\n", trav->ele->arg1);
                if(trav->ele->tag2 == NUM)
                {
                    fprintf(fp,"\tMOV BX, %sd\n", trav->ele->arg2);
                }
                else
                {
                    ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                    if(ret2->isParameter == 0)
                        fprintf(fp,"\tMOV BX, [EBP-8-%d]\n",ret2->offset);
                    else
                        fprintf(fp,"\tMOV BX, [EBP+%d]\n",ret2->offset);
                }
                fprintf(fp,"\tMUL BX\n");

                if(res->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], AX\n", res->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], AX\n", res->offset);
            }
            else if(trav->ele->tag1 == RNUM)
            {
                if(trav->ele->tag2 == RNUM)
                {

                }
                else
                {
                    
                }
            }
            else
            {
                ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(res->ele.data.id.type, "INTEGER"))
                {
                    if(ret1->isParameter == 0)
                        fprintf(fp,"\tMOV AX, [EBP-8-%d]\n",ret1->offset);
                    else
                        fprintf(fp,"\tMOV AX, [EBP+%d]\n",ret1->offset);
                    if(trav->ele->tag2 == NUM)
                    {
                        fprintf(fp,"\tMOV BX, %sd\n", trav->ele->arg2);
                    }
                    else
                    {
                        ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                        if(ret2->isParameter == 0)
                            fprintf(fp,"\tMOV BX, [EBP-8-%d]\n",ret2->offset);
                        else
                            fprintf(fp,"\tMOV BX, [EBP+%d]\n",ret2->offset);
                    }
                    fprintf(fp,"\tMUL BX\n");

                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], AX\n", res->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], AX\n", res->offset);
                }
                else
                {
                    // the operands are real type 
                    // instruction unknown
                }
            }
        }
        else if(!strcmp(trav->ele->op,"/"))
        {
            symbolTableNode *ret1, *ret2, *res; 
            res = searchScopeIRcode(tbStack, trav->ele->result);
            if(trav->ele->tag1 == NUM)
            {
                fprintf(fp,"\tMOV AX, %sd\n", trav->ele->arg1);
                if(trav->ele->tag2 == NUM)
                {
                    fprintf(fp,"\tMOV CX, %sd\n", trav->ele->arg2);
                }
                else
                {
                    ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                    if(ret2->isParameter == 0)
                        fprintf(fp,"\tMOV CX, [EBP-8-%d]\n",ret2->offset);
                    else
                        fprintf(fp,"\tMOV CX, [EBP+%d]\n",ret2->offset);
                }
                fprintf(fp,"\tXOR DX, DX\n");
                fprintf(fp,"\tDIV CX\n");

                if(res->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], AX\n", res->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], AX\n", res->offset);
            }
            else if(trav->ele->tag1 == RNUM)
            {
                if(trav->ele->tag2 == RNUM)
                {
                    // instruction unknown
                }
                else
                {
                    // it's a "real" ID
                    // instruction unknown
                }
            }
            else
            {
                ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(res->ele.data.id.type, "INTEGER"))
                {
                    if(ret1->isParameter == 0)
                        fprintf(fp,"\tMOV AX, [EBP-8-%d]\n",ret1->offset);
                    else
                        fprintf(fp,"\tMOV AX, [EBP+%d]\n",ret1->offset);

                    if(trav->ele->tag2 == NUM)
                    {
                        fprintf(fp,"\tMOV CX, %sd\n", trav->ele->arg2);
                    }
                    else
                    {
                        ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                        if(ret2->isParameter == 0)
                            fprintf(fp,"\tMOV CX, [EBP-8-%d]\n",ret2->offset);
                        else
                            fprintf(fp,"\tMOV CX, [EBP+%d]\n",ret2->offset);
                    }
                    fprintf(fp,"\tXOR DX, DX\n");
                    fprintf(fp,"\tDIV CX\n");

                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], AX\n", res->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], AX\n", res->offset);
                }
                else
                {
                    ///REAL 
                }
            }
        }
        else if(!strcmp(trav->ele->op, "AND") || !strcmp(trav->ele->op, "OR"))
        {
            symbolTableNode *res = searchScopeIRcode(tbStack, trav->ele->result);
            if(trav->ele->tag1 == BOOL)
            {
                if(!strcmp(trav->ele->arg1, "true"))
                    fprintf(fp,"\tMOV AL, 1b\n");
                else
                    fprintf(fp,"\tMOV AL, 0b\n"); 
            }
            else
            {
                symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);

                if(ret1->isParameter == 0)
                    fprintf(fp,"\tMOV AL, [EBP-8-%d]\n",ret1->offset);   
                else
                    fprintf(fp,"\tMOV AL, [EBP+%d]\n",ret1->offset); 
            }
            if(trav->ele->tag2 == BOOL)
            {
                if(!strcmp(trav->ele->arg1, "true"))
                    fprintf(fp,"\tMOV BL, 1b\n");
                else
                    fprintf(fp,"\tMOV BL, 0b\n");
            }
            else
            {
                symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                if(ret2->isParameter == 0)
                    fprintf(fp,"\tMOV BL, [EBP-8-%d]\n",ret2->offset);
                else
                    fprintf(fp,"\tMOV BL, [EBP+%d]\n",ret2->offset);
            }
            
            if(!strcmp(trav->ele->op, "AND"))
            {
                fprintf(fp,"\tAND AL, BL\n");
                if(res->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], AL\n", res->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], AL\n", res->offset);
            }    
            else
            {
                fprintf(fp,"\tOR AL, BL\n");
                if(res->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], AL\n", res->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], AL\n", res->offset);
            }
        }
        
        

        else if (!strcmp(trav->ele->op, "<") ||
        !strcmp(trav->ele->op, ">") ||
        !strcmp(trav->ele->op, "<=") ||
        !strcmp(trav->ele->op, ">=") ||
        !strcmp(trav->ele->op, "==") ||
        !strcmp(trav->ele->op, "!="))
        {
            int instType;
            if(trav->ele->tag1 == NUM)
            {
                fprintf(fp,"\tMOV AX, %sd\n",trav->ele->arg1);
                instType = 0; 
            }
            else if(trav->ele->tag1 == RNUM)
            {
                instType = 1;
                // instruction unknown
            }
            else
            {
                // Its an ID
                symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(ret1->ele.data.id.type, "INTEGER"))
                {
                    if(ret1->isParameter == 0)
                        fprintf(fp,"\tMOV AX, [EBP-8-%d]\n",ret1->offset);
                    else
                        fprintf(fp,"\tMOV AX, [EBP+%d]\n",ret1->offset);
                    instType = 0;    
                }
                else
                {
                    // Real
                    instType = 1;
                }
                
            }
            
            if(trav->ele->tag2 == NUM)
            {
                fprintf(fp,"\tMOV BX, %sd\n",trav->ele->arg2); 
            }
            else if(trav->ele->tag1 == RNUM)
            {
                // instruction unknown
            }
            else
            {
                symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                if(ret2->isParameter == 0)
                    fprintf(fp,"\tMOV BX, [EBP-8-%d]\n",ret2->offset);
                else
                    fprintf(fp,"\tMOV BX, [EBP+%d]\n",ret2->offset);
            }
            
            if(!strcmp(trav->ele->result, "if\0"))
            {
                if(instType == 0) // NUM
                {    
                    fprintf(fp,"CMP AX, BX\n");
                    if(!strcmp(trav->ele->op, ">="))
                        fprintf(fp,"\tJGE %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "<="))
                        fprintf(fp,"\tJLE %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "<"))
                        fprintf(fp,"\tJL %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, ">"))
                        fprintf(fp,"\tJG %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "=="))
                        fprintf(fp,"\tJE %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "!="))
                        fprintf(fp,"\tJNE %s\n", trav->next->ele->arg1);
                    
                    // you have goto in next to next as well
                    // this means you can jump in false case also
                    
                    if(!strcmp(trav->next->next->ele->op, "goto\0"))
                    {
                        fprintf(fp,"\tJMP %s\n", trav->next->next->ele->arg1);
                        trav = trav->next;
                    }
                    trav = trav->next;
                }
                else
                {
                    // Real Operation
                }
            }
            else
            {
                symbolTableNode *res = searchScopeIRcode(tbStack, trav->ele->result);
                // Integer
                if(instType == 0)
                {    
                    fprintf(fp,"\tCMP AX, BX\n");
                    char label1[21], label2[21];
                    getLabel(label1);
                    getLabel(label2);
                    if(!strcmp(trav->ele->op, ">="))
                        fprintf(fp,"\tJGE %s\n", label1);
                    else if(!strcmp(trav->ele->op, "<="))
                        fprintf(fp,"\tJLE %s\n", label1);
                    else if(!strcmp(trav->ele->op, "<"))
                        fprintf(fp,"\tJL %s\n", label1);
                    else if(!strcmp(trav->ele->op, ">"))
                        fprintf(fp,"\tJG %s\n", label1);
                    else if(!strcmp(trav->ele->op, "=="))
                        fprintf(fp,"\tJE %s\n", label1);
                    else if(!strcmp(trav->ele->op, "!="))
                        fprintf(fp,"\tJNE %s\n", label1);
                    fprintf(fp,"\tJMP %s\n", label2);

                    if(res->isParameter == 0)
                    {
                        fprintf(fp,"\t\t%s: MOV [EBP-8-%d], 1b\n", label1, res->offset);
                        fprintf(fp,"\t\t%s: MOV [EBP-8-%d], 0b\n", label2, res->offset);
                    }
                    else
                    {
                        fprintf(fp,"\t\t%s: MOV [EBP+%d], 1b\n", label1, res->offset);
                        fprintf(fp,"\t\t%s: MOV [EBP+%d], 0b\n", label2, res->offset);
                    }
                    
                }
                else
                {
                    // Real Operation
                }
            }
        }
        
        else if(!strcmp(trav->ele->op, "="))
        {
            if(trav->ele->result[0]=='_')
            {
                // BaseAdd(Array) = ESP 
                // ESP = ESP - (m-n+1)*(width)
                
                symbolTableNode * tempo = searchScopeIRcode(tbStack, trav->ele->result);
                if(trav->ele->tag1 == NUM)
                {
                    //_t0 := 10
                    fprintf(fp,"\tMOV AX, %sd\n", trav->ele->arg1);

                    if(tempo->isParameter)
                        fprintf(fp,"\tMOV [BP-8-%d], AX\n", tempo->offset);
                    else 
                        fprintf(fp,"\tMOV [BP+%d], AX\n", tempo->offset);
                        
                }
                else if(trav->ele->tag1 == RNUM)
                {

                }
                else if(trav->ele->tag1 == BOOL)
                {

                }
                else
                {
                    symbolTableNode * arr = searchScopeIRcode(tbStack, trav->ele->arg1);

                    if(arr->ele.tag != Array)
                    {
                        //_t0 = low;
                        if(arr->isParameter == 0)
                            fprintf(fp,"\tMOV AX, [BP-8-%d]\n",arr->offset);
                        else
                            fprintf(fp,"\tMOV AX, [BP+%d]\n",arr->offset);
                            
                        if(tempo->isParameter == 0)
                            fprintf(fp,"\tMOV [BP-8-%d], AX\n",tempo->offset);
                        else
                            fprintf(fp,"\tMOV [BP+%d], AX\n",tempo->offset);
                    }
                    else
                    {
                        //allocate the memory to the 
                        if(arr->ele.data.arr.isDynamic == 1)
                        {
                            if(arr->isParameter == 0)
                                fprintf(fp,"\tMOV [EBP-8-%d], ESP\n", arr->offset);
                            else
                                fprintf(fp,"\tMOV [EBP+%d], ESP\n", arr->offset);
                            
                            if(!strcmp(arr->ele.data.arr.lowerIndex->type,"NUM"))
                            {
                                fprintf(fp,"\tMOV BX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                            }
                            else
                            {
                                symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                                
                                if(index->isParameter == 0)
                                    fprintf(fp,"\tMOV BX, [EBP-8-%d]\n", index->offset);
                                else
                                    fprintf(fp,"\tMOV BX, [EBP+%d]\n", index->offset);
                            }

                            // just to make it zero
                            fprintf(fp,"\tXOR EAX, EAX\n");
                            if(!strcmp(arr->ele.data.arr.upperIndex->type,"NUM"))
                            {
                                fprintf(fp,"\tMOV AX, %dd\n", *(int *)arr->ele.data.arr.upperIndex->value);
                            }
                            else
                            {
                                symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.upperIndex->lexeme);
                                
                                if(index->isParameter==0)
                                    fprintf(fp,"\tMOV AX, [EBP-8-%d]\n", index->offset);
                                else 
                                    fprintf(fp,"\tMOV AX, [EBP+%d]\n", index->offset);
                            }
                            
                            fprintf(fp,"\tINC AX\n");
                            fprintf(fp,"\tSUB AX,BX\n");

                            if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                                fprintf(fp,"\tMUL 2d\n");
                            else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                                fprintf(fp,"\tMUL 4d\n");
                            else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                                fprintf(fp,"\tMUL 1d\n");
                            
                            fprintf(fp,"\tSUB ESP, EAX\n");
                            
                            // marking that the array is dynamic, and allocated
                            arr->ele.data.arr.isDynamic = 2;
                        }
                        
                        // t0 = A[i]
                        // BaseAdd(A) + i*(width)

                        // fprintf(fp,"MOV AX, [EBP-8-%d]", arr->offset);

                        fprintf(fp,"\tXOR AX, AX\n");
                        if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                            fprintf(fp,"\tMOV AX, 2d\n"); 
                        else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                            fprintf(fp,"\tMOV AX, 4d\n"); 
                        else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                            fprintf(fp,"\tMOV AX, 1d\n"); 
                        
                        if(trav->ele->tag2 == NUM)
                        {
                            fprintf(fp,"\tMOV BX, %sd\n", trav->ele->arg2);              
                        }
                        else
                        {
                            symbolTableNode* index = searchScopeIRcode(tbStack, trav->ele->arg2);
                            
                            if(index->isParameter==0)
                                fprintf(fp,"\tMOV BX, [EBP-8-%d]\n", index->offset);
                            else
                                fprintf(fp,"\tMOV BX, [EBP+%d]\n", index->offset);
                                
                        }
                        if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
                        {
                            fprintf(fp,"\tSUB BX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                        }
                        else
                        {
                            //it is an ID
                            symbolTableNode *lowerbound = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                            
                            if(lowerbound->isParameter == 0)
                                fprintf(fp,"\tMOV CX, [EBP-8-%d]\n", lowerbound->offset);
                            else
                                fprintf(fp,"\tMOV CX, [EBP+%d]\n", lowerbound->offset);
                            
                            
                            fprintf(fp,"\tSUB BX, CX\n");
                        }
                        
                        fprintf(fp,"\tMUL BX\n");
                        
                        if(arr->isParameter == 0)
                            fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n", arr->offset);
                        else
                            fprintf(fp,"\tMOV EBX, [EBP+%d]\n", arr->offset);
                        
                        
                        fprintf(fp,"\tADD EAX, EBX\n");
                        //AX contains the address of the array element on RHS

                        if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                        {
                            fprintf(fp,"\tMOV BX, [EAX]\n");
                            if(tempo->isParameter == 0)
                                fprintf(fp,"\tMOV [EBP-8-%d], EBX\n", tempo->offset);
                            else
                                fprintf(fp,"\tMOV [EBP+%d], EBX\n", tempo->offset);
                        }
                        else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                        {
                            // fprintf(fp,"MOV EBX, [EAX]\n");
                            // fprintf(fp,"MOV [EBP-8-%d], EBX\n", tempo->offset);
                        }
                        else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                        {   
                            fprintf(fp,"\tMOV BL, [EAX]\n");

                            if(tempo->isParameter==0)
                                fprintf(fp,"\tMOV [EBP-8-%d], EBX\n", tempo->offset); 
                            else 
                                fprintf(fp,"\tMOV [EBP+%d], EBX\n", tempo->offset); 
                        }
                    }
                }
            }
            else if(!strcmp(trav->ele->arg2, "\0"))
            {
                // x := y 
                // x := t1
                // the result was not a temporary, and arg2 was empty

                symbolTableNode *result =  searchScopeIRcode(tbStack, trav->ele->result);
                if(!strcmp(result->ele.data.id.type,"INTEGER"))
                {
                    if(trav->ele->tag1 == NUM)
                    {
                        fprintf(fp,"\tMOV AX, %sd\n", trav->ele->arg1);
                        if(result->isParameter == 0)
                            fprintf(fp,"\tMOV [EBP-8-%d], AX\n", result->offset);
                        else
                            fprintf(fp,"\tMOV [EBP+%d], AX\n", result->offset);
                    }
                    else
                    {
                        // it is an ID
                        symbolTableNode *ret = searchScopeIRcode(tbStack, trav->ele->arg1);

                        if(ret->isParameter == 0)
                            fprintf(fp,"\tMOV AX, [EBP-8-%d]\n", ret->offset);
                        else
                            fprintf(fp,"\tMOV AX, [EBP-8-%d]\n", ret->offset);

                        if(result->isParameter == 0)
                            fprintf(fp,"\tMOV [EBP-8-%d], AX\n", result->offset);
                        else
                            fprintf(fp,"\tMOV [EBP+%d], AX\n", result->offset);
                    }
                }
                else if(!strcmp(result->ele.data.id.type,"REAL"))
                {
                    if(trav->ele->tag1 == RNUM)
                    {
                        // unknown instructions
                    }
                    else
                    {
                        // symbolTableNode *ret = searchScopeIRcode(tbStack, trav->ele->arg1);
    
                    }
                }
                else if(!strcmp(result->ele.data.id.type,"BOOLEAN"))
                {
                    if(trav->ele->tag1 == BOOL)
                    {
                        if(!strcmp(trav->ele->arg1, "true"))
                            fprintf(fp,"\tMOV AL, 1b\n");
                        else
                            fprintf(fp,"\tMOV AL, 0b\n");
                        
                        if(result->isParameter == 0)
                            fprintf(fp,"\tMOV [EBP-8-%d], AL\n", result->offset);
                        else
                            fprintf(fp,"\tMOV [EBP+%d], AL\n", result->offset);
                    }
                    else
                    {
                        symbolTableNode *ret = searchScopeIRcode(tbStack, trav->ele->arg1);

                        if(ret->isParameter == 0)
                            fprintf(fp,"\tMOV AL, [EBP-8-%d]\n", ret->offset);
                        else
                            fprintf(fp,"\tMOV AL, [EBP+%d]\n", ret->offset);

                        if(result->isParameter==0)
                            fprintf(fp,"\tMOV [EBP-8-%d], AL\n", result->offset);
                        else 
                            fprintf(fp,"\tMOV [EBP+%d], AL\n", result->offset);
                    }
                }
                
            }
            
            else
            {

                //A[5] := t

                // BaseAdd(Array) = ESP 
                // ESP = ESP - (m-n+1)*(width)
                
                symbolTableNode * tempo = NULL;
                if(trav->ele->tag1 == ID)
                    tempo = searchScopeIRcode(tbStack, trav->ele->arg1);
                symbolTableNode * arr = searchScopeIRcode(tbStack, trav->ele->result);
                
                //allocate the memory to the 
                if(arr->ele.data.arr.isDynamic == 1)
                {
                    if(arr->isParameter==0)
                        fprintf(fp,"\tMOV [EBP-8-%d], ESP\n", arr->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], ESP\n", arr->offset);

                    if(!strcmp(arr->ele.data.arr.lowerIndex->type,"NUM"))
                    {
                        fprintf(fp,"\tMOV BX, %dd\n", *(int *)(arr->ele.data.arr.lowerIndex->value));
                    }
                    else
                    {
                        symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                        
                        if(index->isParameter == 0)
                            fprintf(fp,"\tMOV BX, [EBP-8-%d]\n", index->offset);
                        else
                            fprintf(fp,"\tMOV BX, [EBP+%d]\n", index->offset);
                    }

                    // just to make it zero
                    fprintf(fp,"\tXOR EAX, EAX\n");
                    if(!strcmp(arr->ele.data.arr.upperIndex->type,"NUM"))
                    {
                        fprintf(fp,"\tMOV AX, %dd\n", *(int *)(arr->ele.data.arr.upperIndex->value));
                    }
                    else
                    {
                        symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.upperIndex->lexeme);
                        
                        if(index->isParameter == 0)
                            fprintf(fp,"\tMOV AX, [EBP-8-%d]\n", index->offset);
                        else
                            fprintf(fp,"\tMOV AX, [EBP+%d]\n", index->offset);
                    }
                    
                    fprintf(fp,"\tINC AX\n");
                    fprintf(fp,"\tSUB AX,BX\n");

                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                        fprintf(fp,"\tMUL 2d\n");
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                        fprintf(fp,"\tMUL 4d\n");
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                        fprintf(fp,"\tMUL 1d\n");
                    
                    fprintf(fp,"\tSUB ESP, EAX\n");
                    
                    // marking that the array is dynamic, and allocated
                    arr->ele.data.arr.isDynamic = 2;
                }
                
                // A[i] := t
                // BaseAdd(A) + i*(width)

                // fprintf(fp,"MOV AX, [EBP-8-%d]", arr->offset);

                fprintf(fp,"XOR AX, AX\n");
                if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    fprintf(fp,"\tMOV AX, 2d\n"); 
                else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    fprintf(fp,"\tMOV AX, 4d\n"); 
                else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    fprintf(fp,"\tMOV AX, 1d\n"); 
                
                if(trav->ele->tag2 == NUM)
                {
                    fprintf(fp,"\tMOV BX, %sd\n", trav->ele->arg2);              
                }
                else
                {
                    symbolTableNode* index = searchScopeIRcode(tbStack, trav->ele->arg2);

                    if(index->isParameter == 0)
                        fprintf(fp,"\tMOV BX, [EBP-8-%d]\n", index->offset);
                    else
                        fprintf(fp,"\tMOV BX, [EBP+%d]\n", index->offset);
                }


                if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
                {
                    fprintf(fp,"\tSUB BX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                }
                else
                {
                    //it is an ID
                    symbolTableNode *lowerbound = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                    
                    if(lowerbound->isParameter==0)
                    fprintf(fp,"\tMOV CX, [EBP-8-%d]\n", lowerbound->offset);
                    else 
                    fprintf(fp,"\tMOV CX, [EBP+%d]\n", lowerbound->offset);
                    
                    
                    fprintf(fp,"\tSUB BX, CX\n");
                }


                fprintf(fp,"\tMUL BX\n");

                if(arr->isParameter == 0)
                    fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n", arr->offset);
                else
                    fprintf(fp,"\tMOV EBX, [EBP+%d]\n", arr->offset);
                
                fprintf(fp,"\tADD EAX, EBX\n");
                //AX contains the address of the array element on LHS

                if(tempo != NULL)
                {
                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    {
                        if(tempo->isParameter==0)
                            fprintf(fp,"\tMOV BX, [EBP-8-%d]\n", tempo->offset);
                        else
                            fprintf(fp,"\tMOV BX, [EBP+%d]\n", tempo->offset);
                        
                        
                        fprintf(fp,"\tMOV [EAX], BX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    {
                        // fprintf(fp,"MOV EBX, [EBP-8-%d]\n", tempo->offset);
                        // fprintf(fp,"MOV [EAX], EBX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    {   
                        if(tempo->isParameter == 0)
                            fprintf(fp,"\tMOV BL, [EBP-8-%d]\n", tempo->offset);
                        else
                            fprintf(fp,"\tMOV BL, [EBP+%d]\n", tempo->offset);
                            
                        fprintf(fp,"\tMOV [EAX], BL\n"); 
                    }
                }
                else
                {
                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    {
                        fprintf(fp,"\tMOV BX, %sd\n", trav->ele->arg1);
                        fprintf(fp,"\tMOV [EAX], BX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    {
                        // fprintf(fp,"MOV EBX, [EBP-8-%d]\n", tempo->offset);
                        // fprintf(fp,"MOV [EAX], EBX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    {   
                        if(!strcmp(trav->ele->arg1, "true"))
                            fprintf(fp,"\tMOV BL, 1b\n");
                        else
                            fprintf(fp,"\tMOV BL, 0b\n");
                        fprintf(fp,"\tMOV [EAX], BL\n"); 
                    }
                }   
            }
        }
        else if(!strcmp(trav->ele->op, "scanf"))
        {
            // Input: Enter an integer value
            // scanf("Input: Enter an %s value", &int)

            symbolTableNode * var = searchScopeIRcode(tbStack, trav->ele->arg1);

            if(!strcmp(var->ele.data.id.type, "INTEGER"))
            {
                // _integerMem dw ?
                fprintf(fp,"\tPUSH _integerMem\n");
                fprintf(fp,"\tPUSH _integerMsg\n");
                fprintf(fp,"\tCALL scanf\n");
                fprintf(fp,"\tMOV AX, [_integerMem]\n");

                if(var->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], AX\n", var->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], AX\n", var->offset);
                
                fprintf(fp,"\tADD ESP, 8d\n");
            }
            else if(!strcmp(var->ele.data.id.type, "REAL"))
            {
                // unknown commands
            }
            else if(!strcmp(var->ele.data.id.type, "BOOL"))
            {
                fprintf(fp,"\tPUSH _booleanMem\n");
                fprintf(fp,"\tPUSH _booleanMsg\n");
                fprintf(fp,"\tCALL scanf\n");
                fprintf(fp,"\tMOV AL, [_booleanMem]\n");
                
                if(var->isParameter==0)
                    fprintf(fp,"\tMOV [EBP-8-%d], AL\n", var->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], AL\n", var->offset);
                
                fprintf(fp,"\tADD ESP, 8d\n");
            }
        }
        else if(!strcmp(trav->ele->op, "scanf_array"))
        {
            symbolTableNode * var = searchScopeIRcode(tbStack, trav->ele->arg1);

            if(!strcmp(var->ele.data.id.type, "INTEGER"))
            {
                // _integerMem dw ?
                fprintf(fp,"\tPUSH _integerMem\n");
                fprintf(fp,"\tCALL scanf\n");
                fprintf(fp,"\tMOV AX, [_integerMem]\n");
                
                if(var->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], AX\n", var->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], AX\n", var->offset);
                    
                fprintf(fp,"\tADD ESP, 4d\n");
            }
            else if(!strcmp(var->ele.data.id.type, "REAL"))
            {
                // unknown commands
            }
            else if(!strcmp(var->ele.data.id.type, "BOOL"))
            {
                fprintf(fp,"\tPUSH _booleanMem\n");
                fprintf(fp,"\tCALL scanf\n");
                fprintf(fp,"\tMOV AL, [_booleanMem]\n");

                if(var->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], AL\n", var->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], AL\n", var->offset);
                    
                fprintf(fp,"\tADD ESP, 4d\n");
            }
        }
        else if(!strcmp(trav->ele->op, "fprintf") || !strcmp(trav->ele->op, "fprintf_array"))
        {
            //print (A)
            // Output : 2 3 4 5
            //print (x)
            // Output : x

            if(!strcmp(trav->ele->op, "fprintf"))
                fprintf(fp,"\tPUSH _output\n");

            if(trav->ele->tag1 == NUM)
            {
                //fprintf(fp,"%d",);
                fprintf(fp,"\tMOV EAX, %sd\n", trav->ele->arg1);
                fprintf(fp,"\tPUSH EAX\n");
                fprintf(fp,"\tPUSH _percentD\n");
                fprintf(fp,"\tCALL fprintf\n");

                if(!strcmp(trav->ele->op, "fprintf"))
                    fprintf(fp,"\tADD ESP, 10d\n");
                else
                    fprintf(fp,"\tADD ESP, 6d\n");
            }
            else if(trav->ele->tag1 == RNUM)
            {

            }
            else if(trav->ele->tag1 == BOOL)
            {
                if(!strcmp(trav->ele->arg1, "true"))
                    fprintf(fp,"\tPUSH _true\n");
                else
                    fprintf(fp,"\tPUSH _false\n");

                fprintf(fp,"\tPUSH _percentS\n");
                fprintf(fp,"\tCALL fprintf\n");

                if(!strcmp(trav->ele->op, "fprintf"))
                    fprintf(fp,"\tADD ESP, 12d\n");
                else
                    fprintf(fp,"\tADD ESP, 8d\n");
            }
            else
            {
                symbolTableNode *id =  searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(id->ele.data.id.type, "INTEGER"))
                {
                    //fprintf(fp,"%d", );

                    if(id->isParameter==0)
                        fprintf(fp,"\tMOV AX, [EBP-8-%d]\n", id->offset);
                    else
                        fprintf(fp,"\tMOV AX, [EBP+%d]\n", id->offset);
                    
                    
                    fprintf(fp,"\tPUSH EAX\n");
                    fprintf(fp,"\tPUSH _percentD\n");
                    fprintf(fp,"\tCALL fprintf\n");
                    if(!strcmp(trav->ele->op, "fprintf"))
                        fprintf(fp,"\tADD ESP, 10d \n");
                    else
                        fprintf(fp,"\tADD ESP, 6d\n");
                }
                else if(!strcmp(id->ele.data.id.type, "REAL"))
                {
                    //fprintf(fp,"%f", );
                }
                else if(!strcmp(id->ele.data.id.type, "BOOL"))
                {
                    //fprintf(fp,"%s", );
                    
                    if(id->isParameter == 0)
                        fprintf(fp,"\tMOV AL, [EBP-8-%d]\n", id->offset);
                    else
                        fprintf(fp,"\tMOV AL, [EBP+%d]\n", id->offset);

                    char label1[21], label2[21];
                    getLabel(label1);
                    getLabel(label2);
                    fprintf(fp,"\tCMP AL, 1d\n");
                    fprintf(fp,"\tJE %s\n", label1);
                    fprintf(fp,"\tJMP %s\n", label2);
                    fprintf(fp,"\t\t%s: PUSH _true\n", label1);
                    fprintf(fp,"\t\t%s: PUSH _false\n", label2);
                    fprintf(fp,"\tPUSH _percentS\n");
                    fprintf(fp,"\tCALL fprintf\n");
                    if(!strcmp(trav->ele->op, "fprintf"))
                        fprintf(fp,"\tADD ESP, 12d\n");
                    else
                        fprintf(fp,"\tADD ESP, 8d\n");
                }       
            }
        }
        else if(!strcmp(trav->ele->op, "fprintf_output"))
        {
            fprintf(fp,"\tPUSH _output\n");
            fprintf(fp,"\tCALL fprintf\n");
            fprintf(fp,"\tADD ESP, 4d\n");
        }
        else if(!strcmp(trav->ele->op, "fprintf_output_end"))
        {
            fprintf(fp,"\tPUSH _newline\n");
            fprintf(fp,"\tCALL fprintf\n");
            fprintf(fp,"\tADD ESP, 4d\n");
        }
        else if(!strcmp(trav->ele->op, "scanf_output"))
        {
            //Input: Enter 5 array elements of integer type for range 6 to 10

            //fprintf(fp,"Input: Enter %d array elements of %s type for range %d to %d");
            symbolTableNode * arr = searchScopeIRcode(tbStack, trav->ele->arg1);

            if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
            {
                fprintf(fp,"\tMOV BX, %dd\n", *(int*)arr->ele.data.arr.lowerIndex->value);
            }
            else
            {
                symbolTableNode * lower = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);

                if(lower->isParameter == 0)
                    fprintf(fp,"\tMOV BX, [EBP-8-%d]\n", lower->offset);
                else
                    fprintf(fp,"\tMOV BX, [EBP+%d]\n", lower->offset);
            }
            
            if(!strcmp(arr->ele.data.arr.upperIndex->type, "NUM"))
            {
                fprintf(fp,"\tMOV AX, %dd\n", *(int*)arr->ele.data.arr.upperIndex->value);
            }
            else
            {
                symbolTableNode * upper = searchScopeIRcode(tbStack, arr->ele.data.arr.upperIndex->lexeme);
                if(upper->isParameter==0)
                    fprintf(fp,"\tMOV AX, [EBP-8-%d]\n", upper->offset);
                else 
                    fprintf(fp,"\tMOV AX, [EBP+%d]\n", upper->offset);
            }

            fprintf(fp,"\tPUSH EAX\n");
            fprintf(fp,"\tPUSH EBX\n");

            if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                fprintf(fp,"\tPUSH _integerType\n");
            else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                fprintf(fp,"\tPUSH _realType\n");
            else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                fprintf(fp,"\tPUSH _booleanType\n");
        
            fprintf(fp,"\tINC AX\n");
            fprintf(fp,"\tSUB AX, BX\n");
            fprintf(fp,"\tPUSH EAX\n");
            fprintf(fp,"\tPUSH _arrayInputString\n");
            fprintf(fp,"\tCALL fprintf\n");
            fprintf(fp,"\tMOV ESP, 14d\n");
        }
        else if(!strcmp(trav->ele->op,":"))
        {
            fprintf(fp,"\n%s:\n",trav->ele->arg1);

            if(!strcmp(trav->ele->arg1, "main"))
            {
                fprintf(fp,"\tMOV ESP, EBP\n");
                fprintf(fp,"\tPUSH EAX\n");
                fprintf(fp,"\tPUSH EAX\n");
            }
        }

        else if(!strcmp(trav->ele->op,"param"))
        {
            // insert the parameter on the stack
            // leave those many spaces and push the value

            symbolTableNode* var = searchScopeIRcode(tbStack,trav->ele->arg1);
            
            if(var->ele.tag == Array)
            {
                // TODO: Checks for the case when 
                // actual is dynamic and the formal 
                // parameter in the called function is static
                
                // a lot actions taken here
                // bounds are to be pushed as well for static
                // for dynamic, only the address of the array is fine (check this)

                if(var->isParameter == 0)
                    fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", var->offset);
                else
                    fprintf(fp,"\tMOV EAX, [EBP+%d]\n", var->offset);

                // fprintf(fp,"MOV [ESP], EAX\n");
                // fprintf(fp,"SUB ESP, 4d\n");

                fprintf(fp,"\tPUSH EAX\n");
                fprintf(fp,"XOR EBX, EBX\n");
                fprintf(fp,"\tPUSH EBX\n");
            }
            else if(var->ele.tag==Identifier)
            {
                fprintf(fp,"\tXOR EAX, EAX\n");
                fprintf(fp,"\tXOR EBX, EBX\n");
                if(var->ele.data.id.type == "INTEGER")
                {
                    // fprintf(fp,"MOV AX, 2d\n");
                    if(var->isParameter == 0)
                        fprintf(fp,"\tMOV BX,[EBP-8-%d]\n",var->offset); 
                    else
                        fprintf(fp,"\tMOV BX,[EBP+%d]\n",var->offset); 
                    // fprintf(fp,"MOV [ESP],BX\n");
                    fprintf(fp,"\tPUSH EBX\n");
                }
                else if(var->ele.data.id.type == "REAL")
                {
                    // fprintf(fp,"MOV AX, 4d");
                    // fprintf(fp,"MOV EBX, %fd", *(float*)(var->ele.data.id.value));

                    //dont know how to load
                }    
                else if(var->ele.data.id.type == "BOOLEAN")
                { 
                    // fprintf(fp,"MOV AX, 1d\n");
                    if(var->isParameter == 0)
                        fprintf(fp,"\tMOV BL,[EBP-8-%d]\n",var->offset); 
                    else
                        fprintf(fp,"\tMOV BL,[EBP+%d]\n",var->offset); 
                    // fprintf(fp,"MOV [ESP],BL\n");          
                    fprintf(fp,"\tPUSH BX\n");
                }    
                // fprintf(fp,"SUB ESP,EAX\n");
            }
        }
        else if(!strcmp(trav->ele->op,"call"))
        {
            //arg1 : name of the module
            //arg2 : number of parameters
            //call : op

            // [x,y] = call [u,p,q]
            // u,p,q,x,y
            
            // params 
            // call
            // inp u -> pop
            // inp p
            // inp q
            // out x -> pop and populate
            // out y

            fprintf(fp,"\tPUSH EBP\n");
            // fprintf(fp,"MOV ECX,EBP\n");
            fprintf(fp,"\tMOV EBP,ESP\n");
            // fprintf(fp,"PUSH ECX\n");
            fprintf(fp,"\tCALL %s\n",trav->ele->arg1);
        }
        else if(!strcmp(trav->ele->op,"inp"))
        {
            // simply pop it
            symbolTableNode* var = searchScopeIRcode(tbStack, trav->ele->arg1);
            
            if(var->ele.tag==Array)
            {
                fprintf(fp,"\tPOP EAX\n");
                fprintf(fp,"\tPOP EAX\n");           
            }
            else if(var->ele.tag==Identifier)
            {
                if(!strcmp(var->ele.data.id.type, "INTEGER"))
                {
                    fprintf(fp,"\tPOP EAX\n");
                }
                else if(!strcmp(var->ele.data.id.type, "REAL"))
                {
                    //POP EAX
                }
                else if(!strcmp(var->ele.data.id.type, "BOOLEAN"))
                {
                    fprintf(fp,"\tPOP AX\n");
                }
            }
        }
        else if(!strcmp(trav->ele->op,"out"))
        {
            symbolTableNode * var = searchScopeIRcode(tbStack, trav->ele->arg1);
            if(!strcmp(var->ele.data.id.type, "INTEGER"))
            {
                fprintf(fp,"\tPOP EAX\n");
                if(var->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], AX\n", var->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], AX\n", var->offset);
            }
            else if(!strcmp(var->ele.data.id.type, "REAL"))
            {
                //TODO
            }
            else if(!strcmp(var->ele.data.id.type, "BOOLEAN"))
            {
                fprintf(fp,"\tPOP AX\n");

                if(var->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], AL\n", var->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], AL\n", var->offset);
            }
        }
        else if(!strcmp(trav->ele->op,"RET"))
        {
            // <--  ESP
            // EPC
            // EBP
            // <-EBP <-EAX
            // ....
            // ....
            // .... 
            fprintf(fp,"\tMOV EAX, EBP\n");
            fprintf(fp,"\tMOV ESP, [EBP-8]\n");
            fprintf(fp,"\tMOV EBP, [EBP]\n");
            fprintf(fp,"\tRET\n");
        }
        else if(!strcmp(trav->ele->op, "trigger"))
        {
            fprintf(fp,"\tMOV ESP, EAX\n");
        }
        trav = trav->next;
    }
}