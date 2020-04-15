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
        if(!strcmp(trav->ele->op,"SCOPESTART"))
        {
            newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
            newNode->ele = tmp;
            newNode->next = NULL;
            sympush(tbStack, newNode);
            trav = trav->next;
            trav = nasmRecur(trav, tbStack, tmp);
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
                    printf("MOV AX, %sd\n",trav->ele->arg1);
                    printf("MOV [EBP-8-%d], AX\n", res->offset); 
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
                        printf("MOV AX, [EBP-8-%d]\n",ret->offset);
                        printf("MOV [EBP-8-%d], AX\n", res->offset);
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
                    printf("MOV AX, %sd\n",trav->ele->arg1); 
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
                        printf("MOV AX, [EBP-8-%d]\n",ret1->offset);
                    else
                        printf("MOV EAX, [EBP-8-%d]\n",ret1->offset);
                }
                if(trav->ele->tag2 == NUM)
                {
                    printf("MOV BX, %sd\n",trav->ele->arg2); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                }
                else
                {
                    symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                    if(!strcmp(ret2->ele.data.id.type, "INTEGER"))
                        printf("MOV BX, [EBP-8-%d]\n",ret2->offset);
                    else
                        printf("MOV EBX, [EBP-8-%d]\n",ret2->offset);
                }
                
                if(!strcmp(res->ele.data.id.type,"INTEGER"))
                {    
                    printf("ADD AX, BX\n");
                    printf("MOV [EBP-8-%d], AX\n", res->offset);
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
                    printf("MOV AX, %sd\n", trav->ele->arg1);
                    printf("NEG AX\n");
                    printf("MOV [EBP-8-%d], AX\n", res->offset);
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    
                }
                else
                {
                    symbolTableNode * ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret->ele.data.id.type, "INTEGER"))
                    {
                        printf("MOV AX, [EBP-8-%d]\n",ret->offset);
                        printf("NEG AX\n");
                        printf("MOV [EBP-8-%d], AX\n", res->offset);
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
                    printf("MOV AX, %sd\n",trav->ele->arg1); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                }
                else
                {
                    symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret1->ele.data.id.type, "INTEGER"))
                        printf("MOV AX, [EBP-8-%d]\n",ret1->offset);
                    else
                        printf("MOV EAX, [EBP-8-%d]\n",ret1->offset);
                }
                if(trav->ele->tag2 == NUM)
                {
                    printf("MOV BX, %sd\n",trav->ele->arg2); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                }
                else
                {
                    symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                    if(!strcmp(ret2->ele.data.id.type, "INTEGER"))
                        printf("MOV BX, [EBP-8-%d]\n",ret2->offset);
                    else
                        printf("MOV EBX, [EBP-8-%d]\n",ret2->offset);
                }
                
                if(!strcmp(res->ele.data.id.type,"INTEGER"))
                {    
                    printf("SUB AX, BX\n");
                    printf("MOV [EBP-8-%d], AX\n", res->offset);
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
                printf("MOV AX, %sd\n", trav->ele->arg1);
                if(trav->ele->tag2 == NUM)
                {
                    printf("MOV BX, %sd\n", trav->ele->arg2);
                }
                else
                {
                    ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                    printf("MOV BX, [EBP-8-%d]\n",ret2->offset);
                }
                printf("MUL BX\n");
                printf("MOV [EBP-8-%d], AX\n", res->offset);
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
                    printf("MOV AX, [EBP-8-%d]\n",ret1->offset);
                    if(trav->ele->tag2 == NUM)
                    {
                        printf("MOV BX, %sd\n", trav->ele->arg2);
                    }
                    else
                    {
                        ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                        printf("MOV BX, [EBP-8-%d]\n",ret2->offset);
                    }
                    printf("MUL BX\n");
                    printf("MOV [EBP-8-%d], AX\n", res->offset);
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
                printf("MOV AX, %sd\n", trav->ele->arg1);
                if(trav->ele->tag2 == NUM)
                {
                    printf("MOV CX, %sd\n", trav->ele->arg2);
                }
                else
                {
                    ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                    printf("MOV CX, [EBP-8-%d]\n",ret2->offset);
                }
                printf("XOR DX, DX\n");
                printf("DIV CX");
                printf("MOV [EBP-8-%d], AX\n", res->offset);
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
                    printf("MOV AX, [EBP-8-%d]\n",ret1->offset);
                    if(trav->ele->tag2 == NUM)
                    {
                        printf("MOV CX, %sd\n", trav->ele->arg2);
                    }
                    else
                    {
                        ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                        printf("MOV CX, [EBP-8-%d]\n",ret2->offset);
                    }
                    printf("XOR DX, DX\n");
                    printf("DIV CX");
                    printf("MOV [EBP-8-%d], AX\n", res->offset);
                }
                else
                {
                    
                }
            }
        }
        else if(!strcmp(trav->ele->op, "AND") || !strcmp(trav->ele->op, "OR"))
        {
            symbolTableNode *res = searchScopeIRcode(tbStack, trav->ele->result);
            if(trav->ele->tag1 == BOOL)
            {
                if(!strcmp(trav->ele->arg1, "true"))
                    printf("MOV AL, 1b\n");
                else
                    printf("MOV AL, 0b\n"); 
            }
            else
            {
                symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                printf("MOV AL, [EBP-8-%d]\n",ret1->offset);   
            }
            if(trav->ele->tag2 == BOOL)
            {
                if(!strcmp(trav->ele->arg1, "true"))
                    printf("MOV BL, 1b\n");
                else
                    printf("MOV BL, 0b\n");
            }
            else
            {
                symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                printf("MOV BL, [EBP-8-%d]\n",ret2->offset);
            }
            
            if(!strcmp(trav->ele->op, "AND"))
            {
                printf("AND AL, BL\n");
                printf("MOV [EBP-8-%d], AL\n", res->offset);
            }    
            else
            {
                printf("OR AL, BL\n");
                printf("MOV [EBP-8-%d], AL\n", res->offset);
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
                printf("MOV AX, %sd\n",trav->ele->arg1);
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
                    printf("MOV AX, [EBP-8-%d]\n",ret1->offset);
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
                printf("MOV BX, %sd\n",trav->ele->arg2); 
            }
            else if(trav->ele->tag1 == RNUM)
            {
                // instruction unknown
            }
            else
            {
                symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                printf("MOV BX, [EBP-8-%d]\n",ret2->offset);
            }
            
            if(!strcmp(trav->ele->result, "if\0"))
            {
                if(instType == 0) // NUM
                {    
                    printf("CMP AX, BX\n");
                    if(!strcmp(trav->ele->op, ">="))
                        printf("JGE %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "<="))
                        printf("JLE %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "<"))
                        printf("JL %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, ">"))
                        printf("JG %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "=="))
                        printf("JE %s\n", trav->next->ele->arg1);
                    else if(!strcmp(trav->ele->op, "!="))
                        printf("JNE %s\n", trav->next->ele->arg1);
                    
                    // you have goto in next to next as well
                    // this means you can jump in false case also
                    
                    if(!strcmp(trav->next->next->ele->op, "goto\0"))
                    {
                        printf("JMP %s\n", trav->next->next->ele->arg1);
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
                    printf("CMP AX, BX\n");
                    char label1[21], label2[21];
                    getLabel(label1);
                    getLabel(label2);
                    if(!strcmp(trav->ele->op, ">="))
                        printf("JGE %s\n", label1);
                    else if(!strcmp(trav->ele->op, "<="))
                        printf("JLE %s\n", label1);
                    else if(!strcmp(trav->ele->op, "<"))
                        printf("JL %s\n", label1);
                    else if(!strcmp(trav->ele->op, ">"))
                        printf("JG %s\n", label1);
                    else if(!strcmp(trav->ele->op, "=="))
                        printf("JE %s\n", label1);
                    else if(!strcmp(trav->ele->op, "!="))
                        printf("JNE %s\n", label1);
                    printf("JMP %s\n", label2);
                    printf("%s: MOV [EBP-8-%d], 1b\n", label1, res->offset);
                    printf("%s: MOV [EBP-8-%d], 0b\n", label2, res->offset);
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
                    printf("MOV AX, %sd\n", trav->ele->arg1);
                    printf("MOV [BP-8-%d], AX\n", tempo->offset);
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
                        printf("MOV AX, [BP-8-%d]\n",arr->offset);
                        printf("MOV [BP-8-%d], AX\n",tempo->offset);
                    }
                    else
                    {
                        //allocate the memory to the 
                        if(arr->ele.data.arr.isDynamic == 1)
                        {
                            printf("MOV [EBP-8-%d], ESP", arr->offset);
                            
                            if(!strcmp(arr->ele.data.arr.lowerIndex->type,"NUM"))
                            {
                                printf("MOV BX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                            }
                            else
                            {
                                symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                                printf("MOV BX, [EBP-8-%d]\n", index->offset);
                            }

                            // just to make it zero
                            printf("XOR EAX, EAX\n");
                            if(!strcmp(arr->ele.data.arr.upperIndex->type,"NUM"))
                            {
                                printf("MOV AX, %dd\n", *(int *)arr->ele.data.arr.upperIndex->value);
                            }
                            else
                            {
                                symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.upperIndex->lexeme);
                                printf("MOV AX, [EBP-8-%d]\n", index->offset);
                            }
                            
                            printf("INC AX\n");
                            printf("SUB AX,BX\n");

                            if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                                printf("MUL 2d\n");
                            else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                                printf("MUL 4d\n");
                            else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                                printf("MUL 1d\n");
                            
                            printf("SUB ESP, EAX");
                            
                            // marking that the array is dynamic, and allocated
                            arr->ele.data.arr.isDynamic = 2;
                        }
                        
                        // t0 = A[i]
                        // BaseAdd(A) + i*(width)

                        // printf("MOV AX, [EBP-8-%d]", arr->offset);

                        printf("XOR AX, AX\n");
                        if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                            printf("MOV AX, 2d\n"); 
                        else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                            printf("MOV AX, 4d\n"); 
                        else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                            printf("MOV AX, 1d\n"); 
                        
                        if(trav->ele->tag2 == NUM)
                        {
                            printf("MOV BX, %sd\n", trav->ele->arg2);              
                        }
                        else
                        {
                            symbolTableNode* index = searchScopeIRcode(tbStack, trav->ele->arg2);
                            printf("MOV BX, [EBP-8-%d]\n", index->offset);
                        }
                        if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
                        {
                            printf("SUB BX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                        }
                        else
                        {
                            //it is an ID
                            symbolTableNode *lowerbound = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                            printf("MOV CX, [EBP-8-%d]\n", lowerbound->offset);
                            printf("SUB BX, CX\n");
                        }
                        printf("MUL BX\n");
                        printf("MOV EBX, [EBP-8-%d]\n", arr->offset);
                        printf("ADD EAX, EBX\n");
                        //AX contains the address of the array element on RHS

                        if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                        {
                            printf("MOV BX, [EAX]\n");
                            printf("MOV [EBP-8-%d], EBX\n", tempo->offset);
                        }
                        else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                        {
                            // printf("MOV EBX, [EAX]\n");
                            // printf("MOV [EBP-8-%d], EBX\n", tempo->offset);
                        }
                        else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                        {   
                            printf("MOV BL, [EAX]\n");
                            printf("MOV [EBP-8-%d], EBX\n", tempo->offset); 
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
                        printf("MOV AX, %sd\n", trav->ele->arg1);
                        printf("MOV [EBP-8-%d], AX\n", result->offset);
                    }
                    else
                    {
                        // it is an ID
                        symbolTableNode *ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                        printf("MOV AX, [EBP-8-%d]\n", ret->offset);
                        printf("MOV [EBP-8-%d], AX\n", result->offset);
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
                            printf("MOV AL, 1b\n");
                        else
                            printf("MOV AL, 0b\n");
                        printf("MOV [EBP-8-%d], AL\n", result->offset);
                    }
                    else
                    {
                        symbolTableNode *ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                        printf("MOV AL, [EBP-8-%d]\n", ret->offset);
                        printf("MOV [EBP-8-%d], AL\n", result->offset);
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
                    printf("MOV [EBP-8-%d], ESP\n", arr->offset);
                    
                    if(!strcmp(arr->ele.data.arr.lowerIndex->type,"NUM"))
                    {
                        printf("MOV BX, %dd\n", *(int *)(arr->ele.data.arr.lowerIndex->value));
                    }
                    else
                    {
                        symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                        printf("MOV BX, [EBP-8-%d]\n", index->offset);
                    }

                    // just to make it zero
                    printf("XOR EAX, EAX\n");
                    if(!strcmp(arr->ele.data.arr.upperIndex->type,"NUM"))
                    {
                        printf("MOV AX, %dd\n", *(int *)(arr->ele.data.arr.upperIndex->value));
                    }
                    else
                    {
                        symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.upperIndex->lexeme);
                        printf("MOV AX, [EBP-8-%d]\n", index->offset);
                    }
                    
                    printf("INC AX\n");
                    printf("SUB AX,BX\n");

                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                        printf("MUL 2d\n");
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                        printf("MUL 4d\n");
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                        printf("MUL 1d\n");
                    
                    printf("SUB ESP, EAX\n");
                    
                    // marking that the array is dynamic, and allocated
                    arr->ele.data.arr.isDynamic = 2;
                }
                
                // A[i] := t
                // BaseAdd(A) + i*(width)

                // printf("MOV AX, [EBP-8-%d]", arr->offset);

                printf("XOR AX, AX\n");
                if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    printf("MOV AX, 2d\n"); 
                else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    printf("MOV AX, 4d\n"); 
                else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    printf("MOV AX, 1d\n"); 
                
                if(trav->ele->tag2 == NUM)
                {
                    printf("MOV BX, %sd\n", trav->ele->arg2);              
                }
                else
                {
                    symbolTableNode* index = searchScopeIRcode(tbStack, trav->ele->arg2);
                    printf("MOV BX, [EBP-8-%d]\n", index->offset);
                }


                if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
                {
                    printf("SUB BX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                }
                else
                {
                    //it is an ID
                    symbolTableNode *lowerbound = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                    printf("MOV CX, [EBP-8-%d]\n", lowerbound->offset);
                    printf("SUB BX, CX\n");
                }


                printf("MUL BX\n");
                printf("MOV EBX, [EBP-8-%d]\n", arr->offset);
                printf("ADD EAX, EBX\n");
                //AX contains the address of the array element on LHS

                if(tempo != NULL)
                {
                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    {
                        printf("MOV BX, [EBP-8-%d]\n", tempo->offset);
                        printf("MOV [EAX], BX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    {
                        // printf("MOV EBX, [EBP-8-%d]\n", tempo->offset);
                        // printf("MOV [EAX], EBX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    {   
                        printf("MOV BL, [EBP-8-%d]\n", tempo->offset);
                        printf("MOV [EAX], BL\n"); 
                    }
                }
                else
                {
                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    {
                        printf("MOV BX, %sd\n", trav->ele->arg1);
                        printf("MOV [EAX], BX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    {
                        // printf("MOV EBX, [EBP-8-%d]\n", tempo->offset);
                        // printf("MOV [EAX], EBX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    {   
                        if(!strcmp(trav->ele->arg1, "true"))
                            printf("MOV BL, 1b\n");
                        else
                            printf("MOV BL, 0b\n");
                        printf("MOV [EAX], BL\n"); 
                    }
                }   
            }
        }
        else if(!strcmp(trav->ele->op, "scanf"))
        {
            
        }
        else if(!strcmp(trav->ele->op, "printf"))
        {
            
        }
        else if(!strcmp(trav->ele->op,":"))
        {
            printf("%s:\n",trav->ele->arg1);
        }

        else if(!strcmp(trav->ele->op,"param"))
        {
            // insert the parameter on the stack
            // leave those many spaces and push the value

            symbolTableNode* var = searchScopeIRcode(tbStack,trav->ele->arg1);
            
            if(var->ele.tag == Array)
            {
                // a lot actions taken here
                // bounds are to be pushed as well for static
                // for dynamic, only the address of the array is fine (check this)
            }
            else if(var->ele.tag==Identifier)
            {

                printf("XOR EAX, EAX");
                printf("XOR EBX, EBX");
                
                if(var->ele.data.id.type == "INTEGER")
                {
                    printf("MOV AX, 2d");
                    
                    // TO DO: check this 
                    // @Vipin my phone is not detecting my sim
                    // read this case meanwhile please
                    printf("MOV BX,[EBP-8-%d]", var->offset); 
                    //here EBP is not necessarily the EBP of the variable's scope
                }
                else if(var->ele.data.id.type == "REAL")
                {
                    // printf("MOV AX, 4d");
                    // printf("MOV EBX, %fd", *(float*)(var->ele.data.id.value));

                    //dont know how to load
                }    
                else if(var->ele.data.id.type == "BOOLEAN")
                { 
                    printf("MOV AX, 1d");
                    

                }    

                printf("SUB ESP,EAX");


            }
        }
        else if(!strcmp(trav->ele->op,"call"))
        {
            
        }
        trav = trav->next;
    }
}
