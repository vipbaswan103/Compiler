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

void pre_process()
{
    //printf("extern printf;\n");
    printf("SECTION .data;\n");
    printf("\tprintmessage: db \"The number is: %%d\", 10, 0\n");
    printf("\tscanmessage: db \"Enter the number:\", 0\n");
    printf("\t_booleanMem: db ?\n");
    printf("\t_integerMem: dw ?\n");
    printf("\t_output: db \"Output:\", 0\n");
    printf("\t_percentD: db \"%%d\", 0\n");
    printf("\t_percentS: db \"%%s\", 0\n");
    printf("\t_true: dw 1d\n");
    printf("\t_false: dw 0d\n");
    printf("\t_arrayInputString: db \"Input: Enter %%d array elements of %%s type for range %%d to %%d\", 10, 0\n\n");
    printf("SECTION .txt\n\t GLOBAL main \n\t extern printf \n\t extern scanf \n");
}

IRcode* nasmRecur(IRcode* code, tableStack* tbStack, symbolTable * symT)
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
            trav = nasmRecur(trav, tbStack, tmp->child);
            tmp = tmp->sibling;
        }
        else if(!strcmp(trav->ele->op,"SCOPESTART"))
        {
            newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
            newNode->ele = tmp;
            newNode->next = NULL;
            sympush(tbStack, newNode);
            trav = trav->next;
            trav = nasmRecur(trav, tbStack, tmp->child);
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
                    printf("\tMOV AX, %sd\n",trav->ele->arg1);
                    printf("\tMOV [EBP-8-%d], AX\n", res->offset);     
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                }
                else
                {
                    symbolTableNode * ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret->ele.data.id.type, "INTEGER"))
                    {
                        printf("\tMOV AX, [EBP-8-%d]\n",ret->offset);
                        printf("\tMOV [EBP-8-%d], AX\n", res->offset);
                    }
                    else
                    {
                        //REAL OP
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
                    printf("\tMOV AX, %sd\n",trav->ele->arg1); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                }
                else
                {
                    // Its an ID
                    symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret1->ele.data.id.type, "INTEGER"))
                        printf("\tMOV AX, [EBP-8-%d]\n",ret1->offset);
                    else
                    {
                        // printf("\tMOV EAX, [EBP-8-%d]\n",ret1->offset);
                    }
                }
                if(trav->ele->tag2 == NUM)
                {
                    printf("\tMOV BX, %sd\n",trav->ele->arg2); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                }
                else
                {
                    symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                    if(!strcmp(ret2->ele.data.id.type, "INTEGER"))
                        printf("\tMOV BX, [EBP-8-%d]\n",ret2->offset);
                    else
                    {
                        // printf("\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                    }
                }
                
                if(!strcmp(res->ele.data.id.type,"INTEGER"))
                {    
                    printf("\tADD AX, BX\n");
                    printf("\tMOV [EBP-8-%d], AX\n", res->offset);
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
                    printf("\tMOV AX, %sd\n", trav->ele->arg1);
                    printf("\tNEG AX\n");
                    printf("\tMOV [EBP-8-%d], AX\n", res->offset);
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    
                }
                else
                {
                    symbolTableNode * ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret->ele.data.id.type, "INTEGER"))
                    {
                        printf("\tMOV AX, [EBP-8-%d]\n",ret->offset);
                        printf("\tNEG AX\n");
                        printf("\tMOV [EBP-8-%d], AX\n", res->offset);
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
                    printf("\tMOV AX, %sd\n",trav->ele->arg1); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                }
                else
                {
                    symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret1->ele.data.id.type, "INTEGER"))
                        printf("\tMOV AX, [EBP-8-%d]\n",ret1->offset);
                    else
                        // printf("\tMOV EAX, [EBP-8-%d]\n",ret1->offset);
                }
                if(trav->ele->tag2 == NUM)
                {
                    printf("\tMOV BX, %sd\n",trav->ele->arg2); 
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
                        printf("\tMOV BX, [EBP-8-%d]\n",ret2->offset);
                    }
                    else
                        // printf("\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                }
                
                if(!strcmp(res->ele.data.id.type,"INTEGER"))
                {    
                    printf("\tSUB AX, BX\n");
                    printf("\tMOV [EBP-8-%d], AX\n", res->offset);
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
                printf("\tMOV AX, %sd\n", trav->ele->arg1);
                if(trav->ele->tag2 == NUM)
                {
                    printf("\tMOV BX, %sd\n", trav->ele->arg2);
                }
                else
                {
                    ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                    printf("\tMOV BX, [EBP-8-%d]\n",ret2->offset);
                }
                printf("\tMUL BX\n");
                printf("\tMOV [EBP-8-%d], AX\n", res->offset);
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
                    printf("\tMOV AX, [EBP-8-%d]\n",ret1->offset);
                    if(trav->ele->tag2 == NUM)
                    {
                        printf("\tMOV BX, %sd\n", trav->ele->arg2);
                    }
                    else
                    {
                        ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                        printf("\tMOV BX, [EBP-8-%d]\n",ret2->offset);
                    }
                    printf("\tMUL BX\n");
                    printf("\tMOV [EBP-8-%d], AX\n", res->offset);
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
                printf("\tMOV AX, %sd\n", trav->ele->arg1);
                if(trav->ele->tag2 == NUM)
                {
                    printf("\tMOV CX, %sd\n", trav->ele->arg2);
                }
                else
                {
                    ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                    printf("\tMOV CX, [EBP-8-%d]\n",ret2->offset);
                }
                printf("\tXOR DX, DX\n");
                printf("\tDIV CX\n");
                printf("\tMOV [EBP-8-%d], AX\n", res->offset);
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
                    printf("\tMOV AX, [EBP-8-%d]\n",ret1->offset);
                    if(trav->ele->tag2 == NUM)
                    {
                        printf("\tMOV CX, %sd\n", trav->ele->arg2);
                    }
                    else
                    {
                        ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                        printf("\tMOV CX, [EBP-8-%d]\n",ret2->offset);
                    }
                    printf("\tXOR DX, DX\n");
                    printf("\tDIV CX\n");
                    printf("\tMOV [EBP-8-%d], AX\n", res->offset);
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
                    printf("\tMOV AL, 1b\n");
                else
                    printf("\tMOV AL, 0b\n"); 
            }
            else
            {
                symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                printf("\tMOV AL, [EBP-8-%d]\n",ret1->offset);   
            }
            if(trav->ele->tag2 == BOOL)
            {
                if(!strcmp(trav->ele->arg1, "true"))
                    printf("\tMOV BL, 1b\n");
                else
                    printf("\tMOV BL, 0b\n");
            }
            else
            {
                symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                printf("\tMOV BL, [EBP-8-%d]\n",ret2->offset);
            }
            
            if(!strcmp(trav->ele->op, "AND"))
            {
                printf("\tAND AL, BL\n");
                printf("\tMOV [EBP-8-%d], AL\n", res->offset);
            }    
            else
            {
                printf("\tOR AL, BL\n");
                printf("\tMOV [EBP-8-%d], AL\n", res->offset);
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
                printf("\tMOV AX, %sd\n",trav->ele->arg1);
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
                    printf("\tMOV AX, [EBP-8-%d]\n",ret1->offset);
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
                printf("\tMOV BX, %sd\n",trav->ele->arg2); 
            }
            else if(trav->ele->tag1 == RNUM)
            {
                // instruction unknown
            }
            else
            {
                symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                printf("\tMOV BX, [EBP-8-%d]\n",ret2->offset);
            }
            
            if(!strcmp(trav->ele->result, "if\0"))
            {
                if(instType == 0) // NUM
                {    
                    printf("CMP AX, BX\n");
                    if(!strcmp(trav->ele->op, ">="))
                        printf("\tJGE %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "<="))
                        printf("\tJLE %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "<"))
                        printf("\tJL %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, ">"))
                        printf("\tJG %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "=="))
                        printf("\tJE %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "!="))
                        printf("\tJNE %s\n", trav->next->ele->arg1);
                    
                    // you have goto in next to next as well
                    // this means you can jump in false case also
                    
                    if(!strcmp(trav->next->next->ele->op, "goto\0"))
                    {
                        printf("\tJMP %s\n", trav->next->next->ele->arg1);
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
                    printf("\tCMP AX, BX\n");
                    char label1[21], label2[21];
                    getLabel(label1);
                    getLabel(label2);
                    if(!strcmp(trav->ele->op, ">="))
                        printf("\tJGE %s\n", label1);
                    else if(!strcmp(trav->ele->op, "<="))
                        printf("\tJLE %s\n", label1);
                    else if(!strcmp(trav->ele->op, "<"))
                        printf("\tJL %s\n", label1);
                    else if(!strcmp(trav->ele->op, ">"))
                        printf("\tJG %s\n", label1);
                    else if(!strcmp(trav->ele->op, "=="))
                        printf("\tJE %s\n", label1);
                    else if(!strcmp(trav->ele->op, "!="))
                        printf("\tJNE %s\n", label1);
                    printf("\tJMP %s\n", label2);
                    printf("\t\t%s: MOV [EBP-8-%d], 1b\n", label1, res->offset);
                    printf("\t\t%s: MOV [EBP-8-%d], 0b\n", label2, res->offset);
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
                    printf("\tMOV AX, %sd\n", trav->ele->arg1);
                    printf("\tMOV [BP-8-%d], AX\n", tempo->offset);
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
                        printf("\tMOV AX, [BP-8-%d]\n",arr->offset);
                        printf("\tMOV [BP-8-%d], AX\n",tempo->offset);
                    }
                    else
                    {
                        //allocate the memory to the 
                        if(arr->ele.data.arr.isDynamic == 1)
                        {
                            printf("\tMOV [EBP-8-%d], ESP\n", arr->offset);
                            
                            if(!strcmp(arr->ele.data.arr.lowerIndex->type,"NUM"))
                            {
                                printf("\tMOV BX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                            }
                            else
                            {
                                symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                                printf("\tMOV BX, [EBP-8-%d]\n", index->offset);
                            }

                            // just to make it zero
                            printf("\tXOR EAX, EAX\n");
                            if(!strcmp(arr->ele.data.arr.upperIndex->type,"NUM"))
                            {
                                printf("\tMOV AX, %dd\n", *(int *)arr->ele.data.arr.upperIndex->value);
                            }
                            else
                            {
                                symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.upperIndex->lexeme);
                                printf("\tMOV AX, [EBP-8-%d]\n", index->offset);
                            }
                            
                            printf("\tINC AX\n");
                            printf("\tSUB AX,BX\n");

                            if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                                printf("\tMUL 2d\n");
                            else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                                printf("\tMUL 4d\n");
                            else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                                printf("\tMUL 1d\n");
                            
                            printf("\tSUB ESP, EAX\n");
                            
                            // marking that the array is dynamic, and allocated
                            arr->ele.data.arr.isDynamic = 2;
                        }
                        
                        // t0 = A[i]
                        // BaseAdd(A) + i*(width)

                        // printf("MOV AX, [EBP-8-%d]", arr->offset);

                        printf("\tXOR AX, AX\n");
                        if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                            printf("\tMOV AX, 2d\n"); 
                        else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                            printf("\tMOV AX, 4d\n"); 
                        else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                            printf("\tMOV AX, 1d\n"); 
                        
                        if(trav->ele->tag2 == NUM)
                        {
                            printf("\tMOV BX, %sd\n", trav->ele->arg2);              
                        }
                        else
                        {
                            symbolTableNode* index = searchScopeIRcode(tbStack, trav->ele->arg2);
                            printf("\tMOV BX, [EBP-8-%d]\n", index->offset);
                        }
                        if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
                        {
                            printf("\tSUB BX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                        }
                        else
                        {
                            //it is an ID
                            symbolTableNode *lowerbound = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                            printf("\tMOV CX, [EBP-8-%d]\n", lowerbound->offset);
                            printf("\tSUB BX, CX\n");
                        }
                        printf("\tMUL BX\n");
                        printf("\tMOV EBX, [EBP-8-%d]\n", arr->offset);
                        printf("\tADD EAX, EBX\n");
                        //AX contains the address of the array element on RHS

                        if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                        {
                            printf("\tMOV BX, [EAX]\n");
                            printf("\tMOV [EBP-8-%d], EBX\n", tempo->offset);
                        }
                        else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                        {
                            // printf("MOV EBX, [EAX]\n");
                            // printf("MOV [EBP-8-%d], EBX\n", tempo->offset);
                        }
                        else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                        {   
                            printf("\tMOV BL, [EAX]\n");
                            printf("\tMOV [EBP-8-%d], EBX\n", tempo->offset); 
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
                        printf("\tMOV AX, %sd\n", trav->ele->arg1);
                        printf("\tMOV [EBP-8-%d], AX\n", result->offset);
                    }
                    else
                    {
                        // it is an ID
                        symbolTableNode *ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                        printf("\tMOV AX, [EBP-8-%d]\n", ret->offset);
                        printf("\tMOV [EBP-8-%d], AX\n", result->offset);
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
                        symbolTableNode *ret = searchScopeIRcode(tbStack, trav->ele->arg1);
    
                    }
                }
                else if(!strcmp(result->ele.data.id.type,"BOOLEAN"))
                {
                    if(trav->ele->tag1 == BOOL)
                    {
                        if(!strcmp(trav->ele->arg1, "true"))
                            printf("\tMOV AL, 1b\n");
                        else
                            printf("\tMOV AL, 0b\n");
                        printf("\tMOV [EBP-8-%d], AL\n", result->offset);
                    }
                    else
                    {
                        symbolTableNode *ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                        printf("\tMOV AL, [EBP-8-%d]\n", ret->offset);
                        printf("\tMOV [EBP-8-%d], AL\n", result->offset);
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
                    printf("\tMOV [EBP-8-%d], ESP\n", arr->offset);
                    
                    if(!strcmp(arr->ele.data.arr.lowerIndex->type,"NUM"))
                    {
                        printf("\tMOV BX, %dd\n", *(int *)(arr->ele.data.arr.lowerIndex->value));
                    }
                    else
                    {
                        symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                        printf("\tMOV BX, [EBP-8-%d]\n", index->offset);
                    }

                    // just to make it zero
                    printf("\tXOR EAX, EAX\n");
                    if(!strcmp(arr->ele.data.arr.upperIndex->type,"NUM"))
                    {
                        printf("\tMOV AX, %dd\n", *(int *)(arr->ele.data.arr.upperIndex->value));
                    }
                    else
                    {
                        symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.upperIndex->lexeme);
                        printf("\tMOV AX, [EBP-8-%d]\n", index->offset);
                    }
                    
                    printf("\tINC AX\n");
                    printf("\tSUB AX,BX\n");

                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                        printf("\tMUL 2d\n");
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                        printf("\tMUL 4d\n");
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                        printf("\tMUL 1d\n");
                    
                    printf("\tSUB ESP, EAX\n");
                    
                    // marking that the array is dynamic, and allocated
                    arr->ele.data.arr.isDynamic = 2;
                }
                
                // A[i] := t
                // BaseAdd(A) + i*(width)

                // printf("MOV AX, [EBP-8-%d]", arr->offset);

                printf("XOR AX, AX\n");
                if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    printf("\tMOV AX, 2d\n"); 
                else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    printf("\tMOV AX, 4d\n"); 
                else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    printf("\tMOV AX, 1d\n"); 
                
                if(trav->ele->tag2 == NUM)
                {
                    printf("\tMOV BX, %sd\n", trav->ele->arg2);              
                }
                else
                {
                    symbolTableNode* index = searchScopeIRcode(tbStack, trav->ele->arg2);
                    printf("\tMOV BX, [EBP-8-%d]\n", index->offset);
                }


                if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
                {
                    printf("\tSUB BX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                }
                else
                {
                    //it is an ID
                    symbolTableNode *lowerbound = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                    printf("\tMOV CX, [EBP-8-%d]\n", lowerbound->offset);
                    printf("\tSUB BX, CX\n");
                }


                printf("\tMUL BX\n");
                printf("\tMOV EBX, [EBP-8-%d]\n", arr->offset);
                printf("\tADD EAX, EBX\n");
                //AX contains the address of the array element on LHS

                if(tempo != NULL)
                {
                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    {
                        printf("\tMOV BX, [EBP-8-%d]\n", tempo->offset);
                        printf("\tMOV [EAX], BX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    {
                        // printf("MOV EBX, [EBP-8-%d]\n", tempo->offset);
                        // printf("MOV [EAX], EBX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    {   
                        printf("\tMOV BL, [EBP-8-%d]\n", tempo->offset);
                        printf("\tMOV [EAX], BL\n"); 
                    }
                }
                else
                {
                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    {
                        printf("\tMOV BX, %sd\n", trav->ele->arg1);
                        printf("\tMOV [EAX], BX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    {
                        // printf("MOV EBX, [EBP-8-%d]\n", tempo->offset);
                        // printf("MOV [EAX], EBX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    {   
                        if(!strcmp(trav->ele->arg1, "true"))
                            printf("\tMOV BL, 1b\n");
                        else
                            printf("\tMOV BL, 0b\n");
                        printf("\tMOV [EAX], BL\n"); 
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
                printf("\tPUSH _integerMem\n");
                printf("\tPUSH _integerMsg\n");
                printf("\tCALL scanf\n");
                printf("\tMOV AX, [_integerMem]\n");
                printf("\tMOV [EBP-8-%d], AX\n", var->offset);
                printf("\tADD ESP, 5d\n");
            }
            else if(!strcmp(var->ele.data.id.type, "REAL"))
            {
                // unknown commands
            }
            else if(!strcmp(var->ele.data.id.type, "BOOL"))
            {
                printf("\tPUSH _booleanMem\n");
                printf("\tPUSH _booleanMsg\n");
                printf("\tCALL scanf\n");
                printf("\tMOV AL, [_booleanMem]\n");
                printf("\tMOV [EBP-8-%d], AL\n", var->offset);
                printf("\tADD ESP, 5d\n");
            }
        }
        else if(!strcmp(trav->ele->op, "scanf_array"))
        {
            symbolTableNode * var = searchScopeIRcode(tbStack, trav->ele->arg1);

            if(!strcmp(var->ele.data.id.type, "INTEGER"))
            {
                // _integerMem dw ?
                printf("\tPUSH _integerMem\n");
                printf("\tCALL scanf\n");
                printf("\tMOV AX, [_integerMem]\n");
                printf("\tMOV [EBP-8-%d], AX\n", var->offset);
                printf("\tADD ESP, 4d\n");
            }
            else if(!strcmp(var->ele.data.id.type, "REAL"))
            {
                // unknown commands
            }
            else if(!strcmp(var->ele.data.id.type, "BOOL"))
            {
                printf("\tPUSH _booleanMem\n");
                printf("\tCALL scanf\n");
                printf("\tMOV AL, [_booleanMem]\n");
                printf("\tMOV [EBP-8-%d], AL\n", var->offset);
                printf("\tADD ESP, 4d\n");
            }
        }
        else if(!strcmp(trav->ele->op, "printf") || !strcmp(trav->ele->op, "printf_array"))
        {
            //print (A)
            // Output : 2 3 4 5
            //print (x)
            // Output : x

            if(!strcmp(trav->ele->op, "printf"))
                printf("\tPUSH _output\n");

            if(trav->ele->tag1 == NUM)
            {
                //printf("%d",);
                printf("\tMOV AX, %sd\n", trav->ele->arg1);
                printf("\tPUSH AX\n");
                printf("\tPUSH _percentD\n");
                printf("\tCALL printf\n");

                if(!strcmp(trav->ele->op, "printf"))
                    printf("\tADD ESP, 4d\n");
                else
                    printf("\tADD ESP, 3d\n");
            }
            else if(trav->ele->tag1 == RNUM)
            {

            }
            else if(trav->ele->tag1 == BOOL)
            {
                if(!strcmp(trav->ele->arg1, "true"))
                    printf("\tPUSH _true\n");
                else
                    printf("\tPUSH _false\n");
                printf("\tPUSH _percentS\n");
                printf("\tCALL printf\n");
                if(!strcmp(trav->ele->op, "printf"))
                    printf("\tADD ESP, 3d\n");
                else
                    printf("\tADD ESP, 2d\n");
            }
            else
            {
                symbolTableNode *id =  searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(id->ele.data.id.type, "INTEGER"))
                {
                    //printf("%d", );
                    printf("\tMOV AX, [EBP-8-%d]\n", id->offset);
                    printf("\tPUSH AX\n");
                    printf("\tPUSH _percentD\n");
                    printf("\tCALL printf\n");
                    if(!strcmp(trav->ele->op, "printf"))
                        printf("\tADD ESP, 4d\n");
                    else
                        printf("\tADD ESP, 3d\n");
                }
                else if(!strcmp(id->ele.data.id.type, "REAL"))
                {
                    //printf("%f", );
                }
                else if(!strcmp(id->ele.data.id.type, "BOOL"))
                {
                    //printf("%s", );
                    printf("\tMOV AL, [EBP-8-%d]\n", id->offset);
                    char label1[21], label2[21];
                    getLabel(label1);
                    getLabel(label2);
                    printf("\tCMP AL, 1d\n");
                    printf("\tJE %s\n", label1);
                    printf("\tJMP %s\n", label2);
                    printf("\t\t%s: PUSH _true\n", label1);
                    printf("\t\t%s: PUSH _false\n", label2);
                    printf("\tPUSH _percentS\n");
                    printf("\tCALL printf\n");
                    if(!strcmp(trav->ele->op, "printf"))
                        printf("\tADD ESP, 3d\n");
                    else
                        printf("\tADD ESP, 2d\n");
                }       
            }
        }
        else if(!strcmp(trav->ele->op, "printf_output"))
        {
            printf("\tPUSH _output\n");
            printf("\tCALL printf\n");
            printf("\tADD ESP, 1d\n");
        }
        else if(!strcmp(trav->ele->op, "printf_output_end"))
        {
            printf("\tPUSH _newline\n");
            printf("\tCALL printf\n");
            printf("\tADD ESP, 1d\n");
        }
        else if(!strcmp(trav->ele->op, "scanf_output"))
        {
            //Input: Enter 5 array elements of integer type for range 6 to 10

            //printf("Input: Enter %d array elements of %s type for range %d to %d");
            symbolTableNode * arr = searchScopeIRcode(tbStack, trav->ele->arg1);

            if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
            {
                printf("\tMOV BX, %dd\n", *(int*)arr->ele.data.arr.lowerIndex->value);
            }
            else
            {
                symbolTableNode * lower = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                printf("\tMOV BX, [EBP-8-%d]\n", lower->offset);
            }
            
            if(!strcmp(arr->ele.data.arr.upperIndex->type, "NUM"))
            {
                printf("\tMOV AX, %dd\n", *(int*)arr->ele.data.arr.upperIndex->value);
            }
            else
            {
                symbolTableNode * upper = searchScopeIRcode(tbStack, arr->ele.data.arr.upperIndex->lexeme);
                printf("\tMOV AX, [EBP-8-%d]\n", upper->offset);
            }

            printf("\tPUSH AX\n");
            printf("\tPUSH BX\n");

            if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                printf("\tPUSH _integerType\n");
            else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                printf("\tPUSH _realType\n");
            else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                printf("\tPUSH _booleanType\n");
        
            printf("\tINC AX\n");
            printf("\tSUB AX, BX\n");
            printf("\tPUSH AX\n");
            printf("\tPUSH _arrayInputString\n");
            printf("\tCALL printf\n");
            printf("\tMOV ESP, 8d\n");
        }
        else if(!strcmp(trav->ele->op,":"))
        {
            printf("\n%s:\n",trav->ele->arg1);
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

                printf("\tMOV EAX, [EBP-8-%d]\n", var->offset);
                // printf("MOV [ESP], EAX\n");
                // printf("SUB ESP, 4d\n");
                printf("\tPUSH EAX\n");
            }
            else if(var->ele.tag==Identifier)
            {
                printf("\tXOR EAX, EAX\n");
                printf("\tXOR EBX, EBX\n");
                if(var->ele.data.id.type == "INTEGER")
                {
                    // printf("MOV AX, 2d\n");
                    printf("\tMOV BX,[EBP-8-%d]\n",var->offset); 
                    // printf("MOV [ESP],BX\n");
                    printf("\tPUSH BX\n");
                }
                else if(var->ele.data.id.type == "REAL")
                {
                    // printf("MOV AX, 4d");
                    // printf("MOV EBX, %fd", *(float*)(var->ele.data.id.value));

                    //dont know how to load
                }    
                else if(var->ele.data.id.type == "BOOLEAN")
                { 
                    // printf("MOV AX, 1d\n");
                    printf("\tMOV BL,[EBP-8-%d]\n",var->offset); 
                    // printf("MOV [ESP],BL\n");          
                    printf("\tPUSH BL\n");
                }    
                // printf("SUB ESP,EAX\n");
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

            printf("\tPUSH EBP\n");
            // printf("MOV ECX,EBP\n");
            printf("\tMOV EBP,ESP\n");
            // printf("PUSH ECX\n");
            printf("\tCALL %s\n",trav->ele->arg1);
        }
        else if(!strcmp(trav->ele->op,"inp"))
        {
            // simply pop it
            symbolTableNode* var = searchScopeIRcode(tbStack, trav->ele->arg1);
            
            if(var->ele.tag==Array)
            {
                printf("\tPOP EAX\n");           
            }
            else if(var->ele.tag==Identifier)
            {
                if(!strcmp(var->ele.data.id.type, "INTEGER"))
                {
                    printf("\tPOP AX\n");
                }
                else if(!strcmp(var->ele.data.id.type, "REAL"))
                {
                    //TODO
                }
                else if(!strcmp(var->ele.data.id.type, "BOOLEAN"))
                {
                    printf("\tPOP AL\n");
                }
            }
        }
        else if(!strcmp(trav->ele->op,"out"))
        {
            symbolTableNode * var = searchScopeIRcode(tbStack, trav->ele->arg1);
            if(!strcmp(var->ele.data.id.type, "INTEGER"))
            {
                printf("\tPOP AX\n");
                printf("\tMOV [EBP-8-%d], AX\n", var->offset);
            }
            else if(!strcmp(var->ele.data.id.type, "REAL"))
            {
                //TODO
            }
            else if(!strcmp(var->ele.data.id.type, "BOOLEAN"))
            {
                printf("\tPOP AL\n");
                printf("\tMOV [EBP-8-%d], AL\n", var->offset);
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
            printf("\tMOV EAX, EBP\n");
            printf("\tMOV ESP, [EBP-8]\n");
            printf("\tMOV EBP, [EBP]\n");
            printf("\tRET\n");
        }
        else if(!strcmp(trav->ele->op, "trigger"))
        {
            printf("\tMOV ESP, EAX\n");
        }
        trav = trav->next;
    }
}