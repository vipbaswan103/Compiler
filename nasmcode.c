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
    printf("extern printf;\n");
    printf("SECTION .data;\n");
    printf("printmessage: db \"The number is: %%d\", 10, 0 ");
    printf("scanmessage: db \"Enter the number:\", 0 ");

    printf("SECTION .txt; \n GLOBAL main \n GLOBAL printf \n GLOBAL scanf \n");
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
            //print (A)
            // Output : 2 3 4 5
            //print (x)
            // Output : x

            printf("PUSH _output\n");
            if(trav->ele->tag1 == NUM)
            {
                //printf("%d",);
                printf("MOV AX, %sd\n", trav->ele->arg1);
                printf("PUSH AX\n");
                printf("PUSH _percentD\n");
                printf("CALL printf\n");
                printf("ADD ESP, 4d\n");
            }
            else if(trav->ele->tag1 == RNUM)
            {

            }
            else if(trav->ele->tag1 == BOOL)
            {
                if(!strcmp(trav->ele->arg1, "true"))
                    printf("PUSH _true\n");
                else
                    printf("PUSH _false\n");
                printf("PUSH _percentS\n");
                printf("CALL printf\n");
                printf("ADD ESP, 3d\n");
            }
            else
            {
                symbolTableNode *id =  searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(id->ele.data.id.type, "INTEGER"))
                {
                    //printf("%d", );
                    printf("MOV AX, [EBP-8-%d]\n", id->offset);
                    printf("PUSH AX\n");
                    printf("PUSH _percentD\n");
                    printf("CALL printf\n");
                    printf("ADD ESP, 4d\n");
                }
                else if(!strcmp(id->ele.data.id.type, "REAL"))
                {
                    //printf("%f", );
                }
                else if(!strcmp(id->ele.data.id.type, "BOOL"))
                {
                    //printf("%s", );
                    printf("MOV AL, [EBP-8-%d]\n", id->offset);
                    char label1[21], label2[21];
                    getLabel(label1);
                    getLabel(label2);
                    printf("CMP AL, 1d");
                    printf("JE %s", label1);
                    printf("JMP %s", label2);
                    printf("%s: PUSH _true\n", label1);
                    printf("%s: PUSH _false\n", label2);
                    printf("PUSH _percentS\n");
                    printf("CALL printf\n");
                    printf("ADD ESP, 3d\n");
                }       
            }
        }
        else if(!strcmp(trav->ele->op, "printf_array"))
        {
            
        }
        else if(!strcmp(trav->ele->op, "printf_output"))
        {
            printf("PUSH _output\n");
            printf("CALL printf\n");
            printf("ADD ESP, 1d\n");
        }
        else if(!strcmp(trav->ele->op, "printf_output_end"))
        {
            printf("PUSH _newline\n");
            printf("CALL printf\n");
            printf("ADD ESP, 1d\n");
        }
        else if(!strcmp(trav->ele->op, "scanf_output"))
        {
            //Input: Enter 5 array elements of integer type for range 6 to 10

            //printf("Input: Enter %d array elements of %s type for range %d to %d");
            symbolTableNode * arr = searchScopeIRcode(tbStack, trav->ele->arg1);

            if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
            {
                printf("MOV BX, %sd\n", *(int*)arr->ele.data.arr.lowerIndex->value);
            }
            else
            {
                symbolTableNode * lower = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                printf("MOV BX, [EBP-8-%d]\n", lower->offset);
            }
            
            if(!strcmp(arr->ele.data.arr.upperIndex->type, "NUM"))
            {
                printf("MOV AX, %sd\n", *(int*)arr->ele.data.arr.upperIndex->value);
            }
            else
            {
                symbolTableNode * upper = searchScopeIRcode(tbStack, arr->ele.data.arr.upperIndex->lexeme);
                printf("MOV AX, [EBP-8-%d]\n", upper->offset);
            }

            printf("PUSH AX\n");
            printf("PUSH BX\n");
            printf("PUSH _integerType\n");
            printf("INC AX\n");
            printf("SUB AX, BX\n");
            printf("PUSH AX\n");
            printf("PUSH _arrayInputString\n");
            printf("CALL printf\n");
            priintf("MOV ESP, 7d");
        }
        else if(!strcmp(trav->ele->op, "scanf_array"))
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
                // TODO: Checks for the case when 
                // actual is dynamic and the formal 
                // parameter in the called function is static
                
                // a lot actions taken here
                // bounds are to be pushed as well for static
                // for dynamic, only the address of the array is fine (check this)

                printf("MOV EAX, [EBP-8-%d]\n", var->offset);
                // printf("MOV [ESP], EAX\n");
                // printf("SUB ESP, 4d\n");
                printf("PUSH EAX\n");
            }
            else if(var->ele.tag==Identifier)
            {
                printf("XOR EAX, EAX\n");
                printf("XOR EBX, EBX\n");
                if(var->ele.data.id.type == "INTEGER")
                {
                    // printf("MOV AX, 2d\n");
                    printf("MOV BX,[EBP-8-%d]\n",var->offset); 
                    // printf("MOV [ESP],BX\n");
                    printf("PUSH BX\n");
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
                    printf("MOV BL,[EBP-8-%d]\n",var->offset); 
                    // printf("MOV [ESP],BL\n");          
                    printf("PUSH BL\n");
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

            printf("PUSH EBP\n");
            // printf("MOV ECX,EBP\n");
            printf("MOV EBP,ESP\n");
            // printf("PUSH ECX\n");
            printf("CALL %s\n",trav->ele->arg1);
        }
        else if(!strcmp(trav->ele->op,"inp"))
        {
            // simply pop it
            symbolTableNode* var = searchScopeIRcode(tbStack, trav->ele->arg1);
            
            if(var->ele.tag==Array)
            {
                printf("POP EAX\n");           
            }
            else if(var->ele.tag==Identifier)
            {
                if(!strcmp(var->ele.data.id.type, "INTEGER"))
                {
                    printf("POP AX\n");
                }
                else if(!strcmp(var->ele.data.id.type, "REAL"))
                {
                    //TODO
                }
                else if(!strcmp(var->ele.data.id.type, "BOOLEAN"))
                {
                    printf("POP AL\n");
                }
            }
        }
        else if(!strcmp(trav->ele->op,"out"))
        {
            symbolTableNode * var = searchScopeIRcode(tbStack, trav->ele->arg1);
            if(!strcmp(var->ele.data.id.type, "INTEGER"))
            {
                printf("POP AX\n");
                printf("MOV [EBP-8-%d], AX\n", var->offset);
            }
            else if(!strcmp(var->ele.data.id.type, "REAL"))
            {
                //TODO
            }
            else if(!strcmp(var->ele.data.id.type, "BOOLEAN"))
            {
                printf("POP AL\n");
                printf("MOV [EBP-8-%d], AL\n", var->offset);
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
            printf("MOV EAX, EBP\n");
            printf("MOV ESP, [EBP-8]");
            printf("MOV EBP, [EBP]\n");
            printf("RET\n");
        }
        else if(!strcmp(trav->ele->op, "trigger"))
        {
            printf("MOV ESP, EAX\n");
        }
        trav = trav->next;
    }
}