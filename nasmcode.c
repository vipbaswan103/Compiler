/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

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

    int flag=0;
    
    //its not the scope of module declarations and I did not find anything, put it on the other stack
    while(tbStack->size >= 2 && ((ret = sym_hash_find(key, &(tbStack->top->ele->hashtb), 0, NULL)) == NULL))
    {
        flag = 1;
        temp = sympop(tbStack);
        sympush(tempStack, temp);
    }
    //Push everything in tempStack into the tbStack

    // if(ret != NULL && flag == 0 && key[0] != '_')
    // {
    //     if(ret->aux != 2)
    //     {
    //         symbolTableNode *tmp = (symbolTableNode *)malloc(sizeof(symbolTableNode));
    //         *tmp = *ret;
            
    //         temp = sympop(tbStack);
    //         sympush(tempStack, temp);
            
    //         while(tbStack->size >0 
    //         && (ret = sym_hash_find(key, &(tbStack->top->ele->hashtb), 0, NULL)) == NULL)
    //         {
    //             temp = sympop(tbStack);
    //             sympush(tempStack, temp);
    //         }
    //     }   
    // }
    
    // If key is found, check whether it exists 
    if(ret != NULL && key[0] != '_' && tbStack->size >= 3)
    {
        if(ret->aux != 2)
        {
            if(tbStack->size == 0)
            {
                return NULL;
            }
            temp = sympop(tbStack);
            sympush(tempStack, temp);

            ret = sym_hash_find(key, &(tbStack->top->ele->hashtb), 0, NULL);
        
            while(tbStack->size >= 2)
            {
                //i.e. you have searched in the moudle sym table, return whatever you found,
                //you won't find anything useful if you haven't found anything useful yet
                if(tbStack->size == 2)
                    break;

                //i.e. some chances of finding the correct variable
                //If already found, i.e. declaration exists, simply break, restore the stack and return ret
                if(ret != NULL && ret->aux == 2)
                    break;

                //i.e. haven't found anything yet, but not yet reached dead end, keep searching 
                temp = sympop(tbStack);
                sympush(tempStack, temp);
                ret = sym_hash_find(key, &(tbStack->top->ele->hashtb), 0, NULL);
            }
        }   
    }

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
    fprintf(fp,"extern printf \nextern scanf\n");
    fprintf(fp,"SECTION .data\n");
    fprintf(fp,"\tprintmessage: db \"The number is: %%d\", 10, 0\n");
    fprintf(fp,"\tscanmessage: db \"Enter the number:\", 0\n");
    fprintf(fp,"\tnum1: dd 0.0\n");
    fprintf(fp,"\tnum2: dd 0.0\n");
    fprintf(fp,"\t_booleanMem: dd 1d\n");
    fprintf(fp,"\t_integerMem: dd 24d\n");
    fprintf(fp,"\t_floatMem: dq 0.0\n");
    fprintf(fp,"\t_Mem: dd 24d\n");
    fprintf(fp,"\t_integerMsg: db \"Input: Enter an integer value \", 10, 0\n");
    fprintf(fp,"\t_booleanMsg: db \"Input: Enter a boolean value \", 10, 0\n");
    fprintf(fp,"\t_realMsg: db \"Input: Enter a real value \", 10, 0\n");
    fprintf(fp,"\t_inpPercentD: db \"%%d\", 0\n");
    fprintf(fp,"\t_inpPercentF: db \"%%lf\", 0\n");
    fprintf(fp,"\t_error1: db \"RUNTIME ERROR: Array index out of bounds, exiting ...\", 10, 0\n");
    fprintf(fp,"\t_error2: db \"RUNTIME ERROR: Bounds mismatch, exiting ...\", 10, 0\n");
    fprintf(fp,"\t_error3: db \"RUNTIME ERROR: Lower index of array variable is greater than upper index, exiting ...\", 10, 0\n");
    fprintf(fp,"\t_error4: db \"RUNTIME ERROR: Lower index of array variable is negative, exiting ...\", 10, 0\n");
    fprintf(fp,"\t_error5: db \"RUNTIME ERROR: Upper index of array variable is negative, exiting ...\", 10, 0\n");
    fprintf(fp,"\t_output: db \"Output: \", 0\n");
    fprintf(fp,"\t_percentD: db \"%%d\", 10, 0\n");
    fprintf(fp,"\t_percentS: db \"%%s\", 10, 0\n");
    fprintf(fp,"\t_percentF: db \"%%lf\", 10, 0\n");
    fprintf(fp,"\t_percentF_array: db \"%%lf \",  0\n");
    fprintf(fp,"\t_percentD_array: db \"%%d \", 0\n");
    fprintf(fp,"\t_percentS_array: db \"%%s \", 0\n");
    fprintf(fp,"\t_true: db \"true\", 0\n");
    fprintf(fp,"\t_false: db \"false\", 0\n");
    fprintf(fp, "\t_integerType: db \"integer\", 0\n");
    fprintf(fp, "\t_booleanType: db \"boolean\", 0\n");
    fprintf(fp, "\t_realType: db \"real\", 0\n");
    fprintf(fp, "\t_newline: db \"\", 10, 0\n");
    fprintf(fp,"\t_arrayInputString: db \"Input: Enter %%d array elements of %%s type for range %%d to %%d\", 10, 0\n\n");
    fprintf(fp,"SECTION .text\n\tglobal main\n\n");

    fprintf(fp,"RUNTIME_ERROR:\n");
    fprintf(fp,"\tPUSH dword _error1\n");
    fprintf(fp,"\tCALL printf\n");
    fprintf(fp,"\tADD ESP, 4d\n");
    fprintf(fp,"\tMOV EBX, 0\n");
    fprintf(fp,"\tMOV EAX, 1\n");
    fprintf(fp,"\tINT 80h\n\n");

    fprintf(fp,"RUNTIME_ERROR_2:\n");
    fprintf(fp,"\tPUSH dword _error2\n");
    fprintf(fp,"\tCALL printf\n");
    fprintf(fp,"\tADD ESP, 4d\n");
    fprintf(fp,"\tMOV EBX, 0\n");
    fprintf(fp,"\tMOV EAX, 1\n");
    fprintf(fp,"\tINT 80h\n\n");

    fprintf(fp,"RUNTIME_ERROR_3:\n");
    fprintf(fp,"\tPUSH dword _error3\n");
    fprintf(fp,"\tCALL printf\n");
    fprintf(fp,"\tADD ESP, 4d\n");
    fprintf(fp,"\tMOV EBX, 0\n");
    fprintf(fp,"\tMOV EAX, 1\n");
    fprintf(fp,"\tINT 80h\n\n");

    fprintf(fp,"RUNTIME_ERROR_4:\n");
    fprintf(fp,"\tPUSH dword _error4\n");
    fprintf(fp,"\tCALL printf\n");
    fprintf(fp,"\tADD ESP, 4d\n");
    fprintf(fp,"\tMOV EBX, 0\n");
    fprintf(fp,"\tMOV EAX, 1\n");
    fprintf(fp,"\tINT 80h\n\n");

    fprintf(fp,"RUNTIME_ERROR_5:\n");
    fprintf(fp,"\tPUSH dword _error5\n");
    fprintf(fp,"\tCALL printf\n");
    fprintf(fp,"\tADD ESP, 4d\n");
    fprintf(fp,"\tMOV EBX, 0\n");
    fprintf(fp,"\tMOV EAX, 1\n");
    fprintf(fp,"\tINT 80h\n\n");
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
        if(!strcmp(trav->ele->op,"SCOPESTARTMODULE") || !strcmp(trav->ele->op,"SCOPESTARTDRIVER"))
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
        else if(!strcmp(trav->ele->op,"SCOPEENDDRIVER"))
        {
            sympop(tbStack);
            sympop(tbStack);
            fprintf(fp,"\tMOV EBX, 0\n");
            
            fprintf(fp,"\tMOV EAX, 1\n");
            fprintf(fp,"\tINT 80h\n");
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
                    fprintf(fp, "\tMOV EAX, %sd\n",trav->ele->arg1); 

                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);     
                    else
                        fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset); 
                    
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // fprintf(fp,"\tMOV EAX, %sf",trav->ele->arg1); 
                    fprintf(fp,"\tFINIT\n");
                    fprintf(fp,"\tMOV dword [num1], __float32__(%s)\n",trav->ele->arg1); 
                    fprintf(fp,"\tFLD dword [num1]\n");
                    if(res->isParameter == 0)
                        fprintf(fp,"\tFSTP qword [EBP-8-%d]\n", res->offset);
                    else
                        fprintf(fp,"\tFSTP qword [EBP+%d]\n", res->offset);
                }
                else
                {
                    symbolTableNode * ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret->ele.data.id.type, "INTEGER"))
                    {
                        if(ret->isParameter == 0)
                            fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret->offset);
                        else
                            fprintf(fp,"\tMOV EAX, [EBP+%d]\n",ret->offset);
                        
                        if(res->isParameter == 0)
                            fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);
                        else
                            fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset);
                    }
                    else
                    {
                        fprintf(fp,"\tFINIT\n");
                        // fprintf(fp,"\tMOV dword [num1], __float32__(%s)\n",trav->ele->arg1); 
                        // fprintf(fp,"\tFLD qword []\n");
                        if(ret->isParameter == 0)
                            fprintf(fp,"\tFLD qword [EBP-8-%d]\n",ret->offset);
                        else
                            fprintf(fp,"\tFLD qword [EBP+%d]\n",ret->offset);
                        
                        if(res->isParameter == 0)
                            fprintf(fp,"\tFSTP qword [EBP-8-%d]\n", res->offset);
                        else
                            fprintf(fp,"\tFSTP qword [EBP+%d]\n", res->offset);    
                    }       
                }
            }
            // _t0 = _t1 + _t2
            // binary operator
            else
            {
                symbolTableNode *res = searchScopeIRcode(tbStack, trav->ele->result);

                if(!strcmp(res->ele.data.id.type, "REAL"))
                    fprintf(fp,"\tFINIT\n");
                
                if(trav->ele->tag1 == NUM)
                {
                    fprintf(fp,"\tMOV EAX, %sd\n",trav->ele->arg1); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // fprintf(fp,"\tMOV EAX, %sd\n",trav->ele->arg1);
                    fprintf(fp,"\tMOV dword [num1], __float32__(%s)\n",trav->ele->arg1); 
                    fprintf(fp,"\tFLD dword [num1]\n"); 
                }
                else
                {
                    // Its an ID
                    symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret1->ele.data.id.type, "INTEGER"))
                    {
                        if(ret1->isParameter == 0)
                            fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret1->offset);
                        else
                            fprintf(fp,"\tMOV EAX, [EBP+%d]\n",ret1->offset);
                    }
                    else
                    {
                        // fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret1->offset);
                        if(ret1->isParameter == 0)
                            fprintf(fp,"\tFLD qword [EBP-8-%d]\n",ret1->offset);
                        else
                            fprintf(fp,"\tFLD qword [EBP+%d]\n",ret1->offset);

                    }
                }
                if(trav->ele->tag2 == NUM)
                {
                    fprintf(fp,"\tMOV EBX, %sd\n",trav->ele->arg2); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                    fprintf(fp,"\tMOV dword [num2], __float32__(%s)\n",trav->ele->arg2); 
                    fprintf(fp,"\tFLD dword [num2]\n");
                }
                else
                {
                    symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                    if(!strcmp(ret2->ele.data.id.type, "INTEGER"))
                    {
                        if(ret2->isParameter == 0)
                            fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                        else
                            fprintf(fp,"\tMOV EBX, [EBP+%d]\n",ret2->offset);
                    }
                    else
                    {
                        // fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                        if(ret2->isParameter == 0)
                            fprintf(fp,"\tFLD qword [EBP-8-%d]\n",ret2->offset);
                        else
                            fprintf(fp,"\tFLD qword [EBP+%d]\n",ret2->offset);
                    }
                }
                
                if(!strcmp(res->ele.data.id.type,"INTEGER"))
                {    
                    fprintf(fp,"\tADD EAX, EBX\n");

                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset);
                }
                else
                {
                    // Real Operation
                    fprintf(fp,"\tFADD \n");
                    
                    if(res->isParameter == 0)
                        fprintf(fp,"\tFSTP qword [EBP-8-%d]\n", res->offset);
                    else
                        fprintf(fp,"\tFSTP qword [EBP+%d]\n", res->offset);  
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
                    fprintf(fp,"\tMOV EAX, %sd\n", trav->ele->arg1);
                    fprintf(fp,"\tNEG EAX\n");

                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset);
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    fprintf(fp,"\tFINIT\n");
                    
                    fprintf(fp,"\tMOV dword [num1], __float32__(0.0)\n"); 
                    fprintf(fp,"\tFLD dword [num1]\n");
                    fprintf(fp,"\tMOV dword [num2], __float32__(%s)\n", trav->ele->arg1); 
                    fprintf(fp,"\tFLD dword [num2]\n");
                    
                    fprintf(fp,"\tFSUB\n");

                    if(res->isParameter == 0)
                        fprintf(fp,"\tFSTP qword [EBP-8-%d]\n", res->offset);
                    else
                        fprintf(fp,"\tFSTP qword [EBP+%d]\n", res->offset);
                }
                else
                {
                    symbolTableNode * ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret->ele.data.id.type, "INTEGER"))
                    {
                        if(ret->isParameter == 0)
                            fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret->offset);
                        else
                            fprintf(fp,"\tMOV EAX, [EBP+%d]\n",ret->offset);
                        fprintf(fp,"\tNEG EAX\n");

                        if(res->isParameter == 0)
                            fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);
                        else
                            fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset);
                    }
                    else
                    {
                        // REAL OP
                        fprintf(fp,"\tFINIT\n");
                        // fprintf(fp,"\tMOV dword [num1], __float32__(%s)\n",trav->ele->arg1); 
                        // fprintf(fp,"\tFLD qword []\n");
                        fprintf(fp, "\tFLD dword [num1], __float32__(0.0)\n");
                        if(ret->isParameter == 0)
                            fprintf(fp,"\tFLD qword [EBP-8-%d]\n",ret->offset);
                        else
                            fprintf(fp,"\tFLD qword [EBP+%d]\n",ret->offset);

                        fprintf(fp, "\tFSUB\n");
                        
                        if(res->isParameter == 0)
                            fprintf(fp,"\tFSTP qword [EBP-8-%d]\n", res->offset);
                        else
                            fprintf(fp,"\tFSTP qword [EBP+%d]\n", res->offset);
                    }   
                }
            }
            // _t0 = _t1 - _t2
            else
            {
                symbolTableNode *res = searchScopeIRcode(tbStack, trav->ele->result);
                if(!strcmp(res->ele.data.id.type, "REAL"))
                    fprintf(fp,"\tFINIT\n");

                if(trav->ele->tag1 == NUM)
                {
                    fprintf(fp,"\tMOV EAX, %sd\n",trav->ele->arg1); 
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    // instruction unknown
                    fprintf(fp,"\tMOV dword [num1], __float32__(%s)\n",trav->ele->arg1); 
                    fprintf(fp,"\tFLD dword [num1]\n");
                }
                else
                {
                    symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(!strcmp(ret1->ele.data.id.type, "INTEGER"))
                    {
                        if(ret1->isParameter == 0)
                            fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret1->offset);
                        else
                            fprintf(fp,"\tMOV EAX, [EBP+%d]\n",ret1->offset);
                    }
                    else
                    {
                        // fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret1->offset);
                        if(ret1->isParameter == 0)
                            fprintf(fp,"\tFLD qword [EBP-8-%d]\n",ret1->offset);
                        else
                            fprintf(fp,"\tFLD qword [EBP+%d]\n",ret1->offset);
                    }
                }

                if(trav->ele->tag2 == NUM)
                {
                    fprintf(fp,"\tMOV EBX, %sd\n",trav->ele->arg2); 
                }
                else if(trav->ele->tag2 == RNUM)
                {
                    // instruction unknown
                    fprintf(fp,"\tMOV dword [num2], __float32__(%s)\n",trav->ele->arg2); 
                    fprintf(fp,"\tFLD dword [num2]\n");
                }
                else
                {
                    symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                    if(!strcmp(ret2->ele.data.id.type, "INTEGER"))
                    {
                        if(ret2->isParameter == 0)
                            fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                        else
                            fprintf(fp,"\tMOV EBX, [EBP+%d]\n",ret2->offset);
                    }
                    else
                    {
                        // fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                        if(ret2->isParameter == 0)
                            fprintf(fp,"\tFLD qword [EBP-8-%d]\n",ret2->offset);
                        else
                            fprintf(fp,"\tFLD qword [EBP+%d]\n",ret2->offset);   
                    }
                }
                
                if(!strcmp(res->ele.data.id.type,"INTEGER"))
                {    
                    fprintf(fp,"\tSUB EAX, EBX\n");
                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset);
                }
                else
                {
                    // Real Operation
                    fprintf(fp,"\tFSUB \n");
                    
                    if(res->isParameter == 0)
                        fprintf(fp,"\tFSTP qword [EBP-8-%d]\n", res->offset);
                    else
                        fprintf(fp,"\tFSTP qword [EBP+%d]\n", res->offset);    
                }
            }
        }
        else if(!strcmp(trav->ele->op,"*"))
        {
            symbolTableNode *ret1, *ret2, *res; 
            
            res = searchScopeIRcode(tbStack, trav->ele->result);

            if(!strcmp(res->ele.data.id.type, "REAL"))
                fprintf(fp,"\tFINIT\n");

            if(trav->ele->tag1 == NUM)
            {
                fprintf(fp,"\tMOV EAX, %sd\n", trav->ele->arg1);
                if(trav->ele->tag2 == NUM)
                {
                    fprintf(fp,"\tMOV EBX, %sd\n", trav->ele->arg2);
                }
                else
                {
                    ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                    if(ret2->isParameter == 0)
                        fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                    else
                        fprintf(fp,"\tMOV EBX, [EBP+%d]\n",ret2->offset);
                }
                fprintf(fp,"\tMUL EBX\n");

                if(res->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset);
            }
            else if(trav->ele->tag1 == RNUM)
            {
                fprintf(fp, "\tMOV dword [num1], __float32__(%s)\n", trav->ele->arg1);
                fprintf(fp, "\tFLD dword [num1]\n");

                if(trav->ele->tag2 == RNUM)
                {
                    fprintf(fp, "\tMOV dword [num2], __float32__(%s)\n", trav->ele->arg2);
                    fprintf(fp, "\tFLD dword [num2]\n");
                }
                else
                {
                    ret2 = searchScopeIRcode(tbStack, trav->ele->arg1);   

                    if(ret2->isParameter == 0)
                        fprintf(fp, "\tFLD qword [EBP-8-%d]\n", ret2->offset);
                    else
                        fprintf(fp, "\tFLD qword [EBP+%d]\n", ret2->offset);
                }
                fprintf(fp, "\tFMUL\n");

                if(res->isParameter == 0)
                    fprintf(fp, "\tFSTP qword [EBP-8-%d]\n", res->offset);
                else
                    fprintf(fp, "\tFSTP qword [EBP+%d]\n", res->offset);
            }
            else
            {
                ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(res->ele.data.id.type, "INTEGER"))
                {
                    if(ret1->isParameter == 0)
                        fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret1->offset);
                    else
                        fprintf(fp,"\tMOV EAX, [EBP+%d]\n",ret1->offset);
                    if(trav->ele->tag2 == NUM)
                    {
                        fprintf(fp,"\tMOV EBX, %sd\n", trav->ele->arg2);
                    }
                    else
                    {
                        ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                        if(ret2->isParameter == 0)
                            fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                        else
                            fprintf(fp,"\tMOV EBX, [EBP+%d]\n",ret2->offset);
                    }
                    fprintf(fp,"\tMUL EBX\n");

                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset);
                }
                else
                {
                    // the operands are real type 
                    // instruction unknown
                    if(ret1->isParameter == 0)
                        fprintf(fp, "\tFLD qword [EBP-8-%d]\n", ret1->offset);
                    else
                        fprintf(fp, "\tFLD qword [EBP+%d]\n", ret1->offset);

                    if(trav->ele->tag2 == RNUM)
                    {
                        fprintf(fp, "\tMOV dword [num2], __float32__(%s)\n", trav->ele->arg2);
                        fprintf(fp, "\tFLD dword [num2]\n");
                    }
                    else
                    {
                        ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                        if(ret2->isParameter == 0)
                            fprintf(fp, "\tFLD qword [EBP-8-%d]\n", ret2->offset);
                        else
                            fprintf(fp, "\tFLD qword [EBP+%d]\n", ret2->offset);
                    }

                    fprintf(fp, "\tFMUL\n");
                    if(res->isParameter == 0)
                        fprintf(fp, "\tFSTP qword [EBP-8-%d]\n", res->offset);
                    else
                        fprintf(fp, "\tFSTP qword [EBP+%d]\n", res->offset);
                }
            }
        }
        else if(!strcmp(trav->ele->op,"/"))
        {
            symbolTableNode *ret1, *ret2, *res; 
            res = searchScopeIRcode(tbStack, trav->ele->result);
            if(!strcmp(res->ele.data.id.type, "REAL"))
                fprintf(fp,"\tFINIT\n");
            if(trav->ele->tag1 == NUM)
            {
                fprintf(fp,"\tMOV EAX, %sd\n", trav->ele->arg1);
                if(trav->ele->tag2 == NUM)
                {
                    fprintf(fp,"\tMOV ECX, %sd\n", trav->ele->arg2);
                }
                else
                {
                    ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                    if(ret2->isParameter == 0)
                        fprintf(fp,"\tMOV ECX, [EBP-8-%d]\n",ret2->offset);
                    else
                        fprintf(fp,"\tMOV ECX, [EBP+%d]\n",ret2->offset);
                }
                fprintf(fp,"\tXOR EDX, EDX\n");
                fprintf(fp,"\tDIV ECX\n");

                if(res->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset);
            }
            else if(trav->ele->tag1 == RNUM)
            {
                fprintf(fp, "\tMOV dword [num1], __float32__(%s)\n", trav->ele->arg1);
                fprintf(fp, "\tFLD dword [num1]\n");
                if(trav->ele->tag2 == RNUM)
                {
                    fprintf(fp, "\tMOV dword [num2], __float32__(%s)\n", trav->ele->arg2);
                    fprintf(fp, "\tFLD dword [num2]\n");
                }
                else
                {
                    // it's a "real" ID
                    // instruction unknown
                    ret2 = searchScopeIRcode(tbStack, trav->ele->arg1);   

                    if(ret2->isParameter == 0)
                        fprintf(fp, "\tFLD qword [EBP-8-%d]\n", ret2->offset);
                    else
                        fprintf(fp, "\tFLD qword [EBP+%d]\n", ret2->offset);
                }
                fprintf(fp, "\tFDIV\n");

                if(res->isParameter == 0)
                    fprintf(fp, "\tFSTP qword [EBP-8-%d]\n", res->offset);
                else
                    fprintf(fp, "\tFSTP qword [EBP+%d]\n", res->offset);
            }
            else
            {
                ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(res->ele.data.id.type, "INTEGER"))
                {
                    if(ret1->isParameter == 0)
                        fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret1->offset);
                    else
                        fprintf(fp,"\tMOV EAX, [EBP+%d]\n",ret1->offset);

                    if(trav->ele->tag2 == NUM)
                    {
                        fprintf(fp,"\tMOV ECX, %sd\n", trav->ele->arg2);
                    }
                    else
                    {
                        ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                        if(ret2->isParameter == 0)
                            fprintf(fp,"\tMOV ECX, [EBP-8-%d]\n",ret2->offset);
                        else
                            fprintf(fp,"\tMOV ECX, [EBP+%d]\n",ret2->offset);
                    }
                    fprintf(fp,"\tXOR EDX, EDX\n");
                    fprintf(fp,"\tDIV ECX\n");

                    if(res->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset);
                }
                else
                {
                    ///REAL 
                    if(ret1->isParameter == 0)
                        fprintf(fp, "\tFLD qword [EBP-8-%d]\n", ret1->offset);
                    else
                        fprintf(fp, "\tFLD qword [EBP+%d]\n", ret1->offset);

                    if(trav->ele->tag2 == RNUM)
                    {
                        fprintf(fp, "\tMOV dword [num2], __float32__(%s)\n", trav->ele->arg2);
                        fprintf(fp, "\tFLD dword [num2]\n");
                    }
                    else
                    {
                        ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);
                        if(ret2->isParameter == 0)
                            fprintf(fp, "\tFLD qword [EBP-8-%d]\n", ret2->offset);
                        else
                            fprintf(fp, "\tFLD qword [EBP+%d]\n", ret2->offset);
                    }
                    fprintf(fp, "\tFDIV\n");
                    if(res->isParameter == 0)
                        fprintf(fp, "\tFSTP qword [EBP-8-%d]\n", res->offset);
                    else
                        fprintf(fp, "\tFSTP qword [EBP+%d]\n", res->offset);
                }
            }
        }
        else if(!strcmp(trav->ele->op, "AND") || !strcmp(trav->ele->op, "OR"))
        {
            symbolTableNode *res = searchScopeIRcode(tbStack, trav->ele->result);
            if(trav->ele->tag1 == BOOL)
            {
                if(!strcmp(trav->ele->arg1, "true"))
                    fprintf(fp,"\tMOV EAX, 1b\n");
                else
                    fprintf(fp,"\tMOV EAX, 0b\n"); 
            }
            else
            {
                symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);

                if(ret1->isParameter == 0)
                    fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret1->offset);   
                else
                    fprintf(fp,"\tMOV EAX, [EBP+%d]\n",ret1->offset); 
            }
            if(trav->ele->tag2 == BOOL)
            {
                if(!strcmp(trav->ele->arg1, "true"))
                    fprintf(fp,"\tMOV EBX, 1b\n");
                else
                    fprintf(fp,"\tMOV EBX, 0b\n");
            }
            else
            {
                symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                if(ret2->isParameter == 0)
                    fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                else
                    fprintf(fp,"\tMOV EBX, [EBP+%d]\n",ret2->offset);
            }
            
            if(!strcmp(trav->ele->op, "AND"))
            {
                fprintf(fp,"\tAND EAX, EBX\n");
                if(res->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset);
            }    
            else
            {
                fprintf(fp,"\tOR EAX, EBX\n");
                if(res->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", res->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], EAX\n", res->offset);
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
            
            if(trav->ele->tag1 == RNUM)
            {
                fprintf(fp,"\tFINIT\n");
            }
            else if(trav->ele->tag1 == ID)
            {
                symbolTableNode * ret = searchScopeIRcode(tbStack, trav->ele->arg1);

                if(!strcmp(ret->ele.data.arr.type, "REAL"))
                {
                    fprintf(fp, "\tFINIT\n");
                }
            }
            
            if(trav->ele->tag1 == NUM)
            {
                fprintf(fp,"\tMOV EAX, %sd\n",trav->ele->arg1);
                instType = 0; 
            }
            else if(trav->ele->tag1 == RNUM)
            {
                instType = 1;
                fprintf(fp, "\tMOV dword [num1], __float32__(%s)\n", trav->ele->arg1);
                fprintf(fp, "\tFLD dword [num1]\n");
                // instruction unknown

            }
            else
            {
                // Its an ID
                symbolTableNode * ret1 = searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(ret1->ele.data.id.type, "INTEGER")
                || !strcmp(ret1->ele.data.id.type, "BOOLEAN"))
                {
                    if(ret1->isParameter == 0)
                        fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",ret1->offset);
                    else
                        fprintf(fp,"\tMOV EAX, [EBP+%d]\n",ret1->offset);
                    instType = 0;    
                }
                else
                {
                    // Real
                    instType = 1;
                    
                    if(ret1->isParameter == 0)
                        fprintf(fp, "\tFLD qword [EBP-8-%d]\n", ret1->offset);
                    else
                        fprintf(fp, "\tFLD qword [EBP+%d]\n", ret1->offset);
                }
            }
            
            if(trav->ele->tag2 == NUM)
            {
                fprintf(fp,"\tMOV EBX, %sd\n",trav->ele->arg2); 
            }
            else if(trav->ele->tag2 == RNUM)
            {
                // instruction unknown
                fprintf(fp, "\tMOV dword [num2], __float32__(%s)\n", trav->ele->arg2);
                fprintf(fp, "\tFLD dword [num2]\n");
            }
            // else if(trav->ele->tag2 == BOOL)
            // {
                
            // }
            else
            {
                symbolTableNode * ret2 = searchScopeIRcode(tbStack, trav->ele->arg2);

                if(!strcmp(ret2->ele.data.id.type, "INTEGER")
                || !strcmp(ret2->ele.data.id.type, "BOOLEAN"))
                {
                    if(ret2->isParameter == 0)
                        fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n",ret2->offset);
                    else
                        fprintf(fp,"\tMOV EBX, [EBP+%d]\n",ret2->offset);
                }
                else
                {
                    // Real
                    if(ret2->isParameter == 0)
                        fprintf(fp, "\tFLD qword [EBP-8-%d]\n", ret2->offset);
                    else
                        fprintf(fp, "\tFLD qword [EBP+%d]\n", ret2->offset);
                    instType = 1;
                }
            }
            
            if(!strcmp(trav->ele->result, "if\0"))
            {
                if(instType == 0) // NUM
                {    
                    fprintf(fp,"\tCMP EAX, EBX\n");
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
                    fprintf(fp, "\tFCOM st0, st1\n");
                    fprintf(fp, "\tFSTSW AX\n");
                    
                    fprintf(fp, "\tAND EAX, 0100011100000000B\n");
                    
                    if(!strcmp(trav->ele->op, "<="))
                    {
                        fprintf(fp, "\tCMP EAX, 0000000000000000B\n");
                        // char label[15];
                        // getLabel(label);
                        fprintf(fp, "\tJE %s\n", trav->next->ele->arg1);
                        fprintf(fp, "\tCMP EAX, 0100000000000000B\n");
                        fprintf(fp, "\tJE %s\n", trav->next->ele->arg1);
                    }
                    else if(!strcmp(trav->ele->op, ">="))
                    {
                        fprintf(fp, "\tCMP EAX, 0100000000000000B\n");
                        // char label[15];
                        // getLabel(label);
                        fprintf(fp, "\tJE %s\n", trav->next->ele->arg1);
                        fprintf(fp, "\tCMP EAX, 0000000100000000B\n");
                        fprintf(fp, "\tJE %s\n", trav->next->ele->arg1);
                    }
                    else if(!strcmp(trav->ele->op, ">"))
                    {
                        fprintf(fp, "\tCMP EAX, 0000000100000000B\n");
                        fprintf(fp,"\tJE %s\n", trav->next->ele->arg1);
                    }
                    else if(!strcmp(trav->ele->op, "<"))
                    {
                        fprintf(fp, "\tCMP EAX, 0000000000000000B\n");
                        fprintf(fp,"\tJE %s\n", trav->next->ele->arg1);
                    }
                    else if(!strcmp(trav->ele->op, "=="))
                    {
                        fprintf(fp, "\tCMP EAX, 0100000000000000B\n");    
                        fprintf(fp,"\tJE %s\n", trav->next->ele->arg1);
                    }
                    else if(!strcmp(trav->ele->op, "!="))
                    {
                        fprintf(fp, "\tCMP EAX, 0100000000000000B\n");
                        fprintf(fp,"\tJNE %s\n", trav->next->ele->arg1);
                    }

                    if(!strcmp(trav->next->next->ele->op, "goto\0"))
                    {
                        fprintf(fp,"\tJMP %s\n", trav->next->next->ele->arg1);
                        trav = trav->next;
                    }
                    trav = trav->next;
                }
            }
            else
            {
                symbolTableNode *res = searchScopeIRcode(tbStack, trav->ele->result);
                // Integer
                if(instType == 0)
                {    
                    fprintf(fp,"\tCMP EAX, EBX\n");
                    char label1[21], label2[21], label3[21];
                    getLabel(label1);
                    getLabel(label2);
                    getLabel(label3);
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
                        fprintf(fp,"\t\t%s: MOV dword [EBP-8-%d], 1d\n\t\t\tJMP %s\n", label1, res->offset, label3);
                        fprintf(fp,"\t\t%s: MOV dword [EBP-8-%d], 0d\n", label2, res->offset);
                    }
                    else
                    {
                        fprintf(fp,"\t\t%s: MOV dword [EBP+%d], 1d\n\t\t\tJMP %s\n", label1, res->offset, label3);
                        fprintf(fp,"\t\t%s: MOV dword [EBP+%d], 0d\n", label2, res->offset);
                    }
                    fprintf(fp,"\t%s:\n", label3);
                }
                else
                {
                    // Real Operation
                    char label1[21], label2[21], label3[21];
                    getLabel(label1);
                    getLabel(label2);
                    getLabel(label3);

                    fprintf(fp, "\tFCOM st0, st1\n");
                    fprintf(fp, "\tFSTSW AX\n");
                    fprintf(fp, "\tAND EAX, 0100011100000000B\n");
                    if(!strcmp(trav->ele->op, "<="))
                    {
                        fprintf(fp, "\tCMP EAX, 0000000000000000B\n");
                        // char label[15];
                        // getLabel(label);
                        fprintf(fp, "\tJE %s\n", label1);
                        fprintf(fp, "\tCMP EAX, 0100000000000000B\n");
                        fprintf(fp, "\tJE %s\n", label1);
                    }
                    else if(!strcmp(trav->ele->op, ">="))
                    {
                        fprintf(fp, "\tCMP EAX, 0100000000000000B\n");
                        // char label[15];
                        // getLabel(label);
                        fprintf(fp, "\tJE %s\n", label1);
                        fprintf(fp, "\tCMP EAX, 0000000100000000B\n");
                        fprintf(fp, "\tJE %s\n", label1);
                    }
                    else if(!strcmp(trav->ele->op, ">"))
                    {
                        fprintf(fp, "\tCMP EAX, 0000000100000000B\n");
                        fprintf(fp,"\tJE %s\n", label1);
                    }
                    else if(!strcmp(trav->ele->op, "<"))
                    {
                        fprintf(fp, "\tCMP EAX, 0000000000000000B\n");
                        fprintf(fp,"\tJE %s\n", label1);
                    }
                    else if(!strcmp(trav->ele->op, "=="))
                    {
                        fprintf(fp, "\tCMP EAX, 0100000000000000B\n");    
                        fprintf(fp,"\tJE %s\n", label1);
                    }
                    else if(!strcmp(trav->ele->op, "!="))
                    {
                        fprintf(fp, "\tCMP EAX, 0100000000000000B\n");
                        fprintf(fp,"\tJNE %s\n", label1);
                    }
                    fprintf(fp,"\tJMP %s\n", label2);
                    
                    if(res->isParameter == 0)
                    {
                        fprintf(fp,"\t\t%s: MOV dword [EBP-8-%d], 1d\n\t\t\tJMP %s\n", label1, res->offset, label3);
                        fprintf(fp,"\t\t%s: MOV dword [EBP-8-%d], 0d\n", label2, res->offset);
                    }
                    else
                    {
                        fprintf(fp,"\t\t%s: MOV dword [EBP+%d], 1d\n\t\t\tJMP %s\n", label1, res->offset, label3);
                        fprintf(fp,"\t\t%s: MOV dword [EBP+%d], 0d\n", label2, res->offset);
                    }
                    fprintf(fp,"\t%s:\n", label3);
                }
            }
        }

        //only for arrays we have this declare
        else if(!strcmp(trav->ele->op, "declare"))
        {
            symbolTableNode * rest = sym_hash_find(trav->ele->arg1, &(tbStack->top->ele->hashtb),0,NULL);
            rest->aux = 2;
            symbolTableNode * arr = searchScopeIRcode(tbStack, trav->ele->arg1);
            if(arr->ele.tag == Array)
            {
                if(arr->ele.data.arr.isDynamic == 0)
                {
                    fprintf(fp, "\tMOV EAX, EBP\n");
                    fprintf(fp, "\tSUB EAX, 8d\n");
                    fprintf(fp, "\tSUB EAX, %dd\n", arr->offset);

                    if(!strcmp(arr->ele.data.arr.type, "REAL"))
                        fprintf(fp, "\tSUB EAX, 8d\n");
                    else
                        fprintf(fp, "\tSUB EAX, 4d\n");
                    
                    fprintf(fp, "\tMOV [EBP-8-%d], EAX\n", arr->offset);
                }
                else
                {
                    fprintf(fp, "\tMOV EAX, ESP\n");
                    if(!strcmp(arr->ele.data.arr.type, "REAL"))
                        fprintf(fp, "\tSUB EAX, 8d\n");
                    else
                        fprintf(fp, "\tSUB EAX, 4d\n");
                    if(arr->isParameter == 0)
                    {
                        fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", arr->offset);
                    }
                    else
                    {
                        fprintf(fp,"\tMOV [EBP+%d], EAX\n", arr->offset);
                    }
                    
                    if(!strcmp(arr->ele.data.arr.lowerIndex->type,"NUM"))
                    {
                        fprintf(fp,"\tMOV EBX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                    }
                    else
                    {
                        symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                        
                        if(index->isParameter == 0)
                            fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n", index->offset);
                        else
                            fprintf(fp,"\tMOV EBX, [EBP+%d]\n", index->offset);
                    }

                    // just to make it zero
                    fprintf(fp,"\tXOR EAX, EAX\n");
                    if(!strcmp(arr->ele.data.arr.upperIndex->type,"NUM"))
                    {
                        fprintf(fp,"\tMOV EAX, %dd\n", *(int *)arr->ele.data.arr.upperIndex->value);
                    }
                    else
                    {
                        symbolTableNode * index = searchScopeIRcode(tbStack, arr->ele.data.arr.upperIndex->lexeme);
                        
                        if(index->isParameter==0)
                            fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", index->offset);
                        else 
                            fprintf(fp,"\tMOV EAX, [EBP+%d]\n", index->offset);
                    }
                    
                    fprintf(fp,"\tINC EAX\n");
                    fprintf(fp,"\tSUB EAX, EBX\n");

                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                        fprintf(fp,"\tMOV EBX, 4d\n");
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                        fprintf(fp,"\tMOV EBX, 8d\n");
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                        fprintf(fp,"\tMOV EBX, 4d\n");
                    fprintf(fp, "\tMUL BX\n");
                    fprintf(fp,"\tSUB ESP, EAX\n");
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
                if(!strcmp(tempo->ele.data.id.type, "REAL"))
                    fprintf(fp,"\tFINIT\n");

                if(trav->ele->tag1 == NUM)
                {
                    //_t0 := 10
                    fprintf(fp,"\tMOV EAX, %sd\n", trav->ele->arg1);

                    if(tempo->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", tempo->offset);
                    else 
                        fprintf(fp,"\tMOV [EBP+%d], EAX\n", tempo->offset);
                        
                }
                else if(trav->ele->tag1 == RNUM)
                {
                    fprintf(fp,"\tMOV dword [num1], __float32__(%s)\n", trav->ele->arg1);
                    fprintf(fp,"\tFLD dword [num1]\n");
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
                            fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n",arr->offset);
                        else
                            fprintf(fp,"\tMOV EAX, [EBP+%d]\n",arr->offset);
                            
                        if(tempo->isParameter == 0)
                            fprintf(fp,"\tMOV [EBP-8-%d], EAX\n",tempo->offset);
                        else
                            fprintf(fp,"\tMOV [EBP+%d], EAX\n",tempo->offset);
                    }
                    else
                    {
                        
                        fprintf(fp,"\tXOR EAX, EAX\n");
                        if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                            fprintf(fp,"\tMOV AX, 4d\n"); 
                        else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                            fprintf(fp,"\tMOV AX, 8d\n"); 
                        else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                            fprintf(fp,"\tMOV AX, 4d\n"); 
                        
                        if(trav->ele->tag2 == NUM)
                        {
                            fprintf(fp,"\tMOV EBX, %sd\n", trav->ele->arg2);              
                        }
                        else
                        {
                            symbolTableNode* index = searchScopeIRcode(tbStack, trav->ele->arg2);
                            
                            if(index->isParameter==0)
                                fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n", index->offset);
                            else
                                fprintf(fp,"\tMOV EBX, [EBP+%d]\n", index->offset);
                                
                        }
                        if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
                        {
                            fprintf(fp,"\tSUB EBX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                        }
                        else
                        {
                            //it is an ID
                            symbolTableNode *lowerbound = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                            
                            if(lowerbound->isParameter==0)
                                fprintf(fp,"\tMOV ECX, [EBP-8-%d]\n", lowerbound->offset);
                            else
                                fprintf(fp,"\tMOV ECX, [EBP+%d]\n", lowerbound->offset);
                            
                            
                            fprintf(fp,"\tSUB EBX, ECX\n");
                        }
                        
                        fprintf(fp,"\tMUL EBX\n");
                        
                        if(arr->isParameter == 0)
                            fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n", arr->offset);
                        else
                            fprintf(fp,"\tMOV EBX, [EBP+%d]\n", arr->offset);
                        
                        
                        fprintf(fp,"\tSUB EBX, EAX\n");
                        //EBX contains the address of the array element on RHS
                        
                        if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                        {
                            fprintf(fp,"\tMOV EAX, [EBX]\n");
                            if(tempo->isParameter == 0)
                                fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", tempo->offset);
                            else
                                fprintf(fp,"\tMOV [EBP+%d], EAX\n", tempo->offset);
                        }
                        else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                        {
                            // fprintf(fp,"MOV EBX, [EAX]\n");
                            // fprintf(fp,"MOV [EBP-8-%d], EBX\n", tempo->offset) 
                            
                            fprintf(fp, "\tFLD qword [EBX]\n");
                        
                            if(tempo->isParameter == 0)
                                fprintf(fp, "\tFSTP qword [EBP-8-%d]\n", tempo->offset);
                            else
                                fprintf(fp, "\tFSTP qword [EBP+%d]\n", tempo->offset);
                            
                        }
                        else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                        {   
                            fprintf(fp,"\tMOV EAX, [EBX]\n");

                            if(tempo->isParameter==0)
                                fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", tempo->offset); 
                            else 
                                fprintf(fp,"\tMOV [EBP+%d], EAX\n", tempo->offset); 
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

                if(result->ele.tag == Array)
                {
                    symbolTableNode * right = searchScopeIRcode(tbStack, trav->ele->arg1);
                    if(right->isParameter == 0)
                        fprintf(fp, "\tMOV EAX, [EBP-8-%d]\n", right->offset);
                    else
                        fprintf(fp, "\tMOV EAX, [EBP+%d]\n", right->offset);
                    
                    if(result->isParameter == 0)
                        fprintf(fp, "\tMOV [EBP-8-%d], EAX\n", result->offset);
                    else
                        fprintf(fp, "\tMOV [EBP+%d], EAX\n", result->offset);
                }
                else
                {
                    if(!strcmp(result->ele.data.id.type,"INTEGER"))
                    {
                        if(trav->ele->tag1 == NUM)
                        {
                            fprintf(fp,"\tMOV EAX, %sd\n", trav->ele->arg1);
                            if(result->isParameter == 0)
                                fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", result->offset);
                            else
                                fprintf(fp,"\tMOV [EBP+%d], EAX\n", result->offset);
                        }
                        else
                        {
                            // it is an ID
                            symbolTableNode *ret = searchScopeIRcode(tbStack, trav->ele->arg1);

                            if(ret->isParameter == 0)
                                fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", ret->offset);
                            else
                                fprintf(fp,"\tMOV EAX, [EBP+%d]\n", ret->offset);

                            if(result->isParameter == 0)
                                fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", result->offset);
                            else
                                fprintf(fp,"\tMOV [EBP+%d], EAX\n", result->offset);
                        }
                    }
                    else if(!strcmp(result->ele.data.id.type,"REAL"))
                    {
                        if(trav->ele->tag1 == RNUM)
                        {
                            // unknown instructions
                            fprintf(fp,"\tFINIT\n");
                            fprintf(fp,"\tMOV dword [num1], __float32__(%s)\n",trav->ele->arg1); 
                            fprintf(fp,"\tFLD dword [num1]\n");
                            if(result->isParameter == 0)
                                fprintf(fp,"\tFSTP qword [EBP-8-%d]\n", result->offset);
                            else
                                fprintf(fp,"\tFSTP qword [EBP+%d]\n", result->offset);
                        }
                        else
                        {
                            symbolTableNode *ret = searchScopeIRcode(tbStack, trav->ele->arg1);
                            fprintf(fp,"\tFINIT\n");
                            
                            if(ret->isParameter == 0)
                                fprintf(fp,"\tFLD qword [EBP-8-%d]\n",ret->offset);
                            else
                                fprintf(fp,"\tFLD qword [EBP+%d]\n",ret->offset);
                            
                            if(result->isParameter == 0)
                                fprintf(fp,"\tFSTP qword [EBP-8-%d]\n", result->offset);
                            else
                                fprintf(fp,"\tFSTP qword [EBP+%d]\n", result->offset);
                        }
                    }
                    else if(!strcmp(result->ele.data.id.type,"BOOLEAN"))
                    {
                        if(trav->ele->tag1 == BOOL)
                        {
                            if(!strcmp(trav->ele->arg1, "true"))
                                fprintf(fp,"\tMOV EAX, 1b\n");
                            else
                                fprintf(fp,"\tMOV EAX, 0b\n");
                            
                            if(result->isParameter == 0)
                                fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", result->offset);
                            else
                                fprintf(fp,"\tMOV [EBP+%d], EAX\n", result->offset);
                        }
                        else
                        {
                            symbolTableNode *ret = searchScopeIRcode(tbStack, trav->ele->arg1);

                            if(ret->isParameter == 0)
                                fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", ret->offset);
                            else
                                fprintf(fp,"\tMOV EAX, [EBP+%d]\n", ret->offset);

                            if(result->isParameter==0)
                                fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", result->offset);
                            else 
                                fprintf(fp,"\tMOV [EBP+%d], EAX\n", result->offset);
                        }
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
                if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    fprintf(fp,"\tFINIT\n");
                

                fprintf(fp,"\tXOR EAX, EAX\n");
                if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    fprintf(fp,"\tMOV AX, 4d\n"); 
                else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    fprintf(fp,"\tMOV AX, 8d\n"); 
                else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    fprintf(fp,"\tMOV AX, 4d\n"); 
                
                if(trav->ele->tag2 == NUM)
                {
                    fprintf(fp,"\tMOV EBX, %sd\n", trav->ele->arg2);              
                }
                else
                {
                    symbolTableNode* index = searchScopeIRcode(tbStack, trav->ele->arg2);

                    if(index->isParameter == 0)
                        fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n", index->offset);
                    else
                        fprintf(fp,"\tMOV EBX, [EBP+%d]\n", index->offset);
                }


                if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
                {
                    fprintf(fp,"\tSUB EBX, %dd\n", *(int *)arr->ele.data.arr.lowerIndex->value);
                }
                else
                {
                    //it is an ID
                    symbolTableNode *lowerbound = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);
                    
                    if(lowerbound->isParameter==0)
                    fprintf(fp,"\tMOV ECX, [EBP-8-%d]\n", lowerbound->offset);
                    else 
                    fprintf(fp,"\tMOV ECX, [EBP+%d]\n", lowerbound->offset);
                    
                    
                    fprintf(fp,"\tSUB EBX, ECX\n");
                }


                fprintf(fp,"\tMUL EBX\n");

                if(arr->isParameter == 0)
                    fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n", arr->offset);
                else
                    fprintf(fp,"\tMOV EBX, [EBP+%d]\n", arr->offset);
                
                fprintf(fp,"\tSUB EBX, EAX\n");
                //AX contains the address of the array element on LHS

                if(tempo != NULL)
                {
                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    {
                        if(tempo->isParameter==0)
                            fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", tempo->offset);
                        else
                            fprintf(fp,"\tMOV EAX, [EBP+%d]\n", tempo->offset);
                        
                        
                        fprintf(fp,"\tMOV [EBX], EAX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    {
                        // fprintf(fp,"MOV EBX, [EBP-8-%d]\n", tempo->offset);
                        // fprintf(fp,"MOV [EAX], EBX\n");
                        
                        if(tempo->isParameter == 0)
                            fprintf(fp, "\tFLD qword [EBP-8-%d]\n", tempo->offset);
                        else
                            fprintf(fp, "\tFLD qword [EBP+%d]\n", tempo->offset);
                        
                        fprintf(fp, "\tFSTP qword [EBX]\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    {   
                        if(tempo->isParameter == 0)
                            fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", tempo->offset);
                        else
                            fprintf(fp,"\tMOV EAX, [EBP+%d]\n", tempo->offset);
                            
                        fprintf(fp,"\tMOV [EBX], EAX\n"); 
                    }
                }
                else
                {
                    if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                    {
                        fprintf(fp,"\tMOV EBX, %sd\n", trav->ele->arg1);
                        fprintf(fp,"\tMOV [EAX], EBX\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                    {
                        // fprintf(fp,"MOV EBX, [EBP-8-%d]\n", tempo->offset);
                        // fprintf(fp,"MOV [EAX], EBX\n");
                        fprintf(fp, "\tMOV dword [num1], __float32__(%s)\n", trav->ele->arg1);
                        fprintf(fp, "\tFLD dword [num1]\n");
                        
                        fprintf(fp, "\tFSTP qword [EBX]\n");
                    }
                    else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                    {   
                        if(!strcmp(trav->ele->arg1, "true"))
                            fprintf(fp,"\tMOV EBX, 1d\n");
                        else
                            fprintf(fp,"\tMOV EBX, 0d\n");
                        fprintf(fp,"\tMOV [EAX], EBX\n"); 
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

                fprintf(fp, "\tPUSH dword _integerMsg\n");
                fprintf(fp, "\tCALL printf\n");
                fprintf(fp, "\tADD ESP, 4d\n");

                // fprintf(fp, "\tPUSH EBP\n");
                // fprintf(fp, "\tMOV EBP, ESP\n");
                
                fprintf(fp,"\tPUSH dword _integerMem\n");
                fprintf(fp,"\tPUSH dword _inpPercentD\n");
                fprintf(fp,"\tCALL scanf\n");
                fprintf(fp,"\tMOV EAX, [_integerMem]\n");
                fprintf(fp,"\tADD ESP, 8d\n");

                // fprintf(fp,"\tMOV ESP, EBP\n");
                // fprintf(fp,"\tPOP EBP\n");
                if(var->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", var->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], EAX\n", var->offset);
            }
            else if(!strcmp(var->ele.data.id.type, "REAL"))
            {
                fprintf(fp, "\tPUSH dword _realMsg\n");
                fprintf(fp, "\tCALL printf\n");
                fprintf(fp, "\tADD ESP, 4d\n");

                fprintf(fp,"\tFINIT\n");
                fprintf(fp,"\tPUSH dword _floatMem\n");
                fprintf(fp,"\tPUSH dword _inpPercentF\n");
                fprintf(fp,"\tCALL scanf\n");
                fprintf(fp,"\tFLD qword [_floatMem]\n");

                if(var->isParameter == 0)
                    fprintf(fp,"\tFSTP qword [EBP-8-%d]\n", var->offset);
                else
                    fprintf(fp,"\tFSTP qword [EBP+%d]\n", var->offset);

                fprintf(fp,"\tADD ESP, 8d\n");
            }
            else if(!strcmp(var->ele.data.id.type, "BOOLEAN"))
            {
                fprintf(fp, "\tPUSH dword _booleanMsg\n");
                fprintf(fp, "\tCALL printf\n");
                fprintf(fp, "\tADD ESP, 4d\n");

                // fprintf(fp, "\tPUSH EBP\n");
                // fprintf(fp, "\tMOV EBP, ESP\n");
        
                fprintf(fp,"\tPUSH dword _booleanMem\n");
                fprintf(fp,"\tPUSH dword _inpPercentD\n");
                fprintf(fp,"\tCALL scanf\n");
                fprintf(fp,"\tMOV EAX, [_booleanMem]\n");
                fprintf(fp,"\tADD ESP, 8d\n");

                // fprintf(fp,"\tMOV ESP, EBP\n");
                // fprintf(fp,"\tPOP EBP\n");
                if(var->isParameter==0)
                    fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", var->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], EAX\n", var->offset);
            }
        }
        else if(!strcmp(trav->ele->op, "scanf_array"))
        {
            symbolTableNode * var = searchScopeIRcode(tbStack, trav->ele->arg1);
            if(!strcmp(var->ele.data.id.type, "INTEGER"))
            {
                // _integerMem dw ?                    
                // fprintf(fp, "\tPUSH EBP\n");
                // fprintf(fp, "\tMOV EBP, ESP\n");
                
                fprintf(fp,"\tPUSH dword _integerMem\n");
                fprintf(fp,"\tPUSH dword _inpPercentD\n");
                fprintf(fp,"\tCALL scanf\n");
                fprintf(fp,"\tMOV EAX, [_integerMem]\n");
                fprintf(fp,"\tADD ESP, 8d\n");
                
                // fprintf(fp,"\tMOV ESP, EBP\n");
                // fprintf(fp,"\tPOP EBP\n");
                
                if(var->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", var->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], EAX\n", var->offset);
            }
            else if(!strcmp(var->ele.data.id.type, "REAL"))
            {
                fprintf(fp,"\tFINIT\n");
                fprintf(fp,"\tPUSH dword _floatMem\n");
                fprintf(fp,"\tPUSH dword _inpPercentF\n");
                fprintf(fp,"\tCALL scanf\n");
                fprintf(fp,"\tFLD qword [_floatMem]\n");
                fprintf(fp,"\tADD ESP, 8d\n");

                if(var->isParameter == 0)
                    fprintf(fp,"\tFSTP qword [EBP-8-%d]\n", var->offset);
                else
                    fprintf(fp,"\tFSTP qword [EBP+%d]\n", var->offset);
            }
            else if(!strcmp(var->ele.data.id.type, "BOOLEAN"))
            {
                // fprintf(fp, "\tPUSH EBP\n");
                // fprintf(fp, "\tMOV EBP, ESP\n");

                fprintf(fp,"\tPUSH dword _booleanMem\n");
                fprintf(fp,"\tPUSH dword _inpPercentD\n");
                fprintf(fp,"\tCALL scanf\n");
                fprintf(fp,"\tMOV EAX, [_booleanMem]\n");
                fprintf(fp,"\tADD ESP, 8d\n");
                
                // fprintf(fp,"\tMOV ESP, EBP\n");
                // fprintf(fp,"\tPOP EBP\n");
                
                if(var->isParameter == 0)
                    fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", var->offset);
                else
                    fprintf(fp,"\tMOV [EBP+%d], EAX\n", var->offset);
            }
        }
        else if(!strcmp(trav->ele->op, "printf_array"))
        {
            //print (A)
            // Output : 2 3 4 5
            //print (x)
            // Output : x

            if(trav->ele->tag1 == NUM)
            {
                //fprintf(fp,"%d",);

                // fprintf(fp, "\tPUSH EBP\n");
                // fprintf(fp, "\tMOV EBP, ESP\n");

                fprintf(fp,"\tMOV EAX, %sd\n", trav->ele->arg1);
                fprintf(fp,"\tPUSH EAX\n");
                fprintf(fp,"\tPUSH dword _percentD_array\n");
                fprintf(fp,"\tCALL printf\n");
                fprintf(fp,"\tADD ESP, 8d\n");
                // fprintf(fp, "\tMOV ESP, EBP\n");
                // fprintf(fp, "\tPOP EBP\n");
            }
            else if(trav->ele->tag1 == RNUM)
            {
                fprintf(fp,"\tFINIT\n");
                fprintf(fp,"\tMOV dword [num1], __float32__(%s)\n", trav->ele->arg1);
                fprintf(fp,"\tFLD dword [num1]\n");
                fprintf(fp,"\tSUB ESP, 8d\n");
                fprintf(fp,"\tFSTP qword [ESP]\n");
                fprintf(fp,"\tPUSH dword _percentF_array\n");
                fprintf(fp,"\tCALL printf\n");
                fprintf(fp,"\tADD ESP, 12d\n");
            }
            else if(trav->ele->tag1 == BOOL)
            {
                // fprintf(fp, "\tPUSH EBP\n");
                // fprintf(fp, "\tMOV EBP, ESP\n"); 

                if(!strcmp(trav->ele->arg1, "true"))
                    fprintf(fp,"\tPUSH dword _true\n");
                else
                    fprintf(fp,"\tPUSH dword _false\n");
                fprintf(fp,"\tPUSH dword _percentS_array\n");
                fprintf(fp,"\tCALL printf\n");
                fprintf(fp,"\tADD ESP, 8d\n");

                // fprintf(fp, "\tMOV ESP, EBP\n");
                // fprintf(fp, "\tPOP EBP\n");
            }
            else
            {
                symbolTableNode *id =  searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(id->ele.data.id.type, "INTEGER"))
                {
                    //fprintf(fp,"%d", );
                    if(id->isParameter==0)
                        fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", id->offset);
                    else
                        fprintf(fp,"\tMOV EAX, [EBP+%d]\n", id->offset);
                    
                    // fprintf(fp, "\tPUSH EBP\n");
                    // fprintf(fp, "\tMOV EBP, ESP\n");
                    
                    fprintf(fp,"\tPUSH EAX\n");
                    fprintf(fp,"\tPUSH dword _percentD_array\n");
                    fprintf(fp,"\tCALL printf\n");
                    fprintf(fp,"\tADD ESP, 8d \n");

                    // fprintf(fp, "\tMOV ESP, EBP\n");
                    // fprintf(fp, "\tPOP EBP\n");
                }
                else if(!strcmp(id->ele.data.id.type, "REAL"))
                {
                    //fprintf(fp,"%f", );
                    fprintf(fp,"\tFINIT\n");
                    if(id->isParameter==0)
                    {
                        fprintf(fp,"\tFLD qword [EBP-8-%d]\n", id->offset);
                    }
                    else
                    {
                        fprintf(fp,"\tFLD qword [EBP+%d]\n", id->offset);
                    }

                    fprintf(fp,"\tSUB ESP, 8d\n");
                    fprintf(fp,"\tFSTP qword [ESP]\n");

                    fprintf(fp,"\tPUSH dword _percentF_array\n");
                    fprintf(fp,"\tCALL printf\n");
                    fprintf(fp,"\tADD ESP, 12d \n");
                }
                else if(!strcmp(id->ele.data.id.type, "BOOLEAN"))
                {
                    //fprintf(fp,"%s", );
                    
                    if(id->isParameter == 0)
                        fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", id->offset);
                    else
                        fprintf(fp,"\tMOV EAX, [EBP+%d]\n", id->offset);
                    
                    char label1[21], label2[21], label3[21];
                    getLabel(label1);
                    getLabel(label2);
                    getLabel(label3);
                    fprintf(fp,"\tCMP EAX, 1d\n");
                    fprintf(fp,"\tJE %s\n", label1);
                    fprintf(fp,"\tJMP %s\n", label2);

                    // fprintf(fp, "\tPUSH EBP\n");
                    // fprintf(fp, "\tMOV EBP, ESP\n");
                    
                    fprintf(fp,"\t\t%s: PUSH dword _true\n\t\t\tJMP %s\n", label1, label3);
                    fprintf(fp,"\t\t%s: PUSH dword _false\n", label2);
                    fprintf(fp,"\t%s:\n", label3);
                    fprintf(fp,"\tPUSH dword _percentS_array\n");
                    fprintf(fp,"\tCALL printf\n");
                    fprintf(fp,"\tADD ESP, 8d\n");
                    
                    // fprintf(fp, "\tMOV ESP, EBP\n");
                    // fprintf(fp, "\tPOP EBP\n");
                }       
            }
        }
        else if(!strcmp(trav->ele->op, "printf"))
        {
            //print (A)
            // Output : 2 3 4 5
            //print (x)
            // Output : x

            fprintf(fp, "\tPUSH dword _output\n");
            fprintf(fp, "\tCALL printf\n");
            fprintf(fp, "\tADD ESP, 4d\n");
            
            if(trav->ele->tag1 == NUM)
            {
                //fprintf(fp,"%d",);

                // fprintf(fp, "\tPUSH EBP\n");
                // fprintf(fp, "\tMOV EBP, ESP\n");

                fprintf(fp,"\tMOV EAX, %sd\n", trav->ele->arg1);
                fprintf(fp,"\tPUSH EAX\n");
                fprintf(fp,"\tPUSH dword _percentD\n");
                fprintf(fp,"\tCALL printf\n");
                
                fprintf(fp,"\tADD ESP, 8d\n");
                // fprintf(fp, "\tMOV ESP, EBP\n");
                // fprintf(fp, "\tPOP EBP\n");
            }
            else if(trav->ele->tag1 == RNUM)
            {
                fprintf(fp,"\tFINIT\n");
                fprintf(fp,"\tMOV dword [num1], __float32__(%s)\n", trav->ele->arg1);
                fprintf(fp,"\tFLD dword [num1]\n");
                fprintf(fp,"\tSUB ESP, 8d\n");
                fprintf(fp,"\tFSTP qword [ESP]\n");
                fprintf(fp,"\tPUSH dword _percentF\n");
                fprintf(fp,"\tCALL printf\n");
                fprintf(fp,"\tADD ESP, 12d\n");
            }
            else if(trav->ele->tag1 == BOOL)
            {
                // fprintf(fp, "\tPUSH EBP\n");
                // fprintf(fp, "\tMOV EBP, ESP\n"); 

                if(!strcmp(trav->ele->arg1, "true"))
                    fprintf(fp,"\tPUSH dword _true\n");
                else
                    fprintf(fp,"\tPUSH dword _false\n");
                fprintf(fp,"\tPUSH dword _percentS\n");
                fprintf(fp,"\tCALL printf\n");
                fprintf(fp,"\tADD ESP, 8d\n");

                // fprintf(fp, "\tMOV ESP, EBP\n");
                // fprintf(fp, "\tPOP EBP\n");
            }
            else
            {
                symbolTableNode *id =  searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(id->ele.data.id.type, "INTEGER"))
                {
                    //fprintf(fp,"%d", );
                    if(id->isParameter==0)
                        fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", id->offset);
                    else
                        fprintf(fp,"\tMOV EAX, [EBP+%d]\n", id->offset);
                    
                    // fprintf(fp, "\tPUSH EBP\n");
                    // fprintf(fp, "\tMOV EBP, ESP\n");
                    
                    fprintf(fp,"\tPUSH EAX\n");
                    fprintf(fp,"\tPUSH dword _percentD\n");
                    fprintf(fp,"\tCALL printf\n");
                    fprintf(fp,"\tADD ESP, 8d \n");

                    // fprintf(fp, "\tMOV ESP, EBP\n");
                    // fprintf(fp, "\tPOP EBP\n");
                }
                else if(!strcmp(id->ele.data.id.type, "REAL"))
                {
                    //fprintf(fp,"%f", );
                    fprintf(fp,"\tFINIT\n");
                    if(id->isParameter==0)
                        fprintf(fp,"\tFLD qword [EBP-8-%d]\n", id->offset);
                    else
                        fprintf(fp,"\tFLD qword [EBP+%d]\n", id->offset);
                    
                    // fprintf(fp, "\tPUSH EBP\n");
                    // fprintf(fp, "\tMOV EBP, ESP\n");
                    
                    fprintf(fp,"\tSUB ESP, 8d\n");
                    fprintf(fp,"\tFSTP qword [ESP]\n");
                    fprintf(fp,"\tPUSH dword _percentF\n");
                    fprintf(fp,"\tCALL printf\n");
                    fprintf(fp,"\tADD ESP, 12d \n");
                }
                else if(!strcmp(id->ele.data.id.type, "BOOLEAN"))
                {
                    //fprintf(fp,"%s", );
                    
                    if(id->isParameter == 0)
                        fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", id->offset);
                    else
                        fprintf(fp,"\tMOV EAX, [EBP+%d]\n", id->offset);
                    
                    char label1[21], label2[21], label3[21];
                    getLabel(label1);
                    getLabel(label2);
                    getLabel(label3);
                    fprintf(fp,"\tCMP EAX, 1d\n");
                    fprintf(fp,"\tJE %s\n", label1);
                    fprintf(fp,"\tJMP %s\n", label2);

                    // fprintf(fp, "\tPUSH EBP\n");
                    // fprintf(fp, "\tMOV EBP, ESP\n");
                    
                    fprintf(fp,"\t\t%s: PUSH dword _true\n\t\t\tJMP %s\n", label1, label3);
                    fprintf(fp,"\t\t%s: PUSH dword _false\n", label2);
                    fprintf(fp,"\t%s:\n", label3);
                    fprintf(fp,"\tPUSH dword _percentS\n");
                    fprintf(fp,"\tCALL printf\n");
                    fprintf(fp,"\tADD ESP, 8d\n");
                    
                    // fprintf(fp, "\tMOV ESP, EBP\n");
                    // fprintf(fp, "\tPOP EBP\n");
                }       
            }
        }
        else if(!strcmp(trav->ele->op, "printf_output"))
        {
            // fprintf(fp, "\tPUSH EBP\n");
            // fprintf(fp, "\tMOV EBP, ESP\n");

            fprintf(fp,"\tPUSH dword _output\n");
            fprintf(fp,"\tCALL printf\n");
            fprintf(fp,"\tADD ESP, 4d\n");

            // fprintf(fp, "\tMOV ESP, EBP\n");
            // fprintf(fp, "\tPOP EBP\n");
        }
        else if(!strcmp(trav->ele->op, "printf_output_end"))
        {
            // fprintf(fp, "\tPUSH EBP\n");
            // fprintf(fp, "\tMOV EBP, ESP\n");

            fprintf(fp,"\tPUSH dword _newline\n");
            fprintf(fp,"\tCALL printf\n");
            fprintf(fp,"\tADD ESP, 4d\n");

            // fprintf(fp, "\tMOV ESP, EBP\n");
            // fprintf(fp, "\tPOP EBP\n");
        }
        else if(!strcmp(trav->ele->op, "scanf_output"))
        {
            //Input: Enter 5 array elements of integer type for range 6 to 10

            //fprintf(fp,"Input: Enter %d array elements of %s type for range %d to %d");
            symbolTableNode * arr = searchScopeIRcode(tbStack, trav->ele->arg1);

            if(!strcmp(arr->ele.data.arr.lowerIndex->type, "NUM"))
            {
                fprintf(fp,"\tMOV EBX, %dd\n", *(int*)arr->ele.data.arr.lowerIndex->value);
            }
            else
            {
                symbolTableNode * lower = searchScopeIRcode(tbStack, arr->ele.data.arr.lowerIndex->lexeme);

                if(lower->isParameter == 0)
                    fprintf(fp,"\tMOV EBX, [EBP-8-%d]\n", lower->offset);
                else
                    fprintf(fp,"\tMOV EBX, [EBP+%d]\n", lower->offset);
            }
            
            if(!strcmp(arr->ele.data.arr.upperIndex->type, "NUM"))
            {
                fprintf(fp,"\tMOV EAX, %dd\n", *(int*)arr->ele.data.arr.upperIndex->value);
            }
            else
            {
                symbolTableNode * upper = searchScopeIRcode(tbStack, arr->ele.data.arr.upperIndex->lexeme);
                if(upper->isParameter==0)
                    fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", upper->offset);
                else 
                    fprintf(fp,"\tMOV EAX, [EBP+%d]\n", upper->offset);
            }

            // fprintf(fp, "\tPUSH EBP\n");
            // fprintf(fp, "\tMOV EBP, ESP\n");

            fprintf(fp,"\tPUSH EAX\n");
            fprintf(fp,"\tPUSH EBX\n");
            if(!strcmp(arr->ele.data.arr.type, "INTEGER"))
                fprintf(fp,"\tPUSH dword _integerType\n");
            else if(!strcmp(arr->ele.data.arr.type, "REAL"))
                fprintf(fp,"\tPUSH dword _realType\n");
            else if(!strcmp(arr->ele.data.arr.type, "BOOLEAN"))
                fprintf(fp,"\tPUSH dword _booleanType\n");
            fprintf(fp,"\tINC EAX\n");
            fprintf(fp,"\tSUB EAX, EBX\n");
            fprintf(fp,"\tPUSH EAX\n");
            fprintf(fp,"\tPUSH dword _arrayInputString\n");
            fprintf(fp,"\tCALL printf\n");
            fprintf(fp,"\tADD ESP, 20d\n");

            // fprintf(fp, "\tMOV ESP, EBP\n");
            // fprintf(fp, "\tPOP EBP\n");
        }
        else if(!strcmp(trav->ele->op,":"))
        {
            fprintf(fp,"\n%s:\n",trav->ele->arg1);
            if(!strcmp(trav->ele->arg1, "main"))
            {
                fprintf(fp,"\tMOV EBP, ESP\n");
                fprintf(fp,"\tPUSH EAX\n");
                fprintf(fp,"\tPUSH EAX\n");
                fprintf(fp, "\tSUB ESP, %dd\n", symT->currentOffset);
            }
            else
            {
                // symbolTableNode * func = searchScopeIRcode(tbStack, trav->ele->arg1);
                symbolTableNode * func = sym_hash_find(trav->ele->arg1, &(symbolTableRoot->hashtb), 0, NULL);
                if(func != NULL && func->ele.tag == Module)
                {
                    fprintf(fp, "\tSUB ESP, %dd\n", symT->currentOffset);
                }
            }
        }

        else if(!strcmp(trav->ele->op,"param"))
        {
            // insert the parameter on the stack
            // leave those many spaces and push the value
            if(trav->ele->tag1 == NUM)
            {
                fprintf(fp, "\tMOV EAX, %sd\n", trav->ele->arg1);
                fprintf(fp, "\tPUSH EAX\n");
            }
            else
            {
                symbolTableNode* var = searchScopeIRcode(tbStack,trav->ele->arg1);
                if(var->ele.tag == Array)
                {
                    // TODO: Checks for the case when 
                    // actual is dynamic and the formal 
                    // parameter in the called function is static

                    if(var->isParameter == 0)
                        fprintf(fp,"\tMOV EAX, [EBP-8-%d]\n", var->offset);
                    else
                        fprintf(fp,"\tMOV EAX, [EBP+%d]\n", var->offset);

                    fprintf(fp,"\tXOR EBX, EBX\n");
                    fprintf(fp,"\tPUSH EBX\n");
                    fprintf(fp,"\tPUSH EAX\n");                
                }
                else if(var->ele.tag == Identifier)
                {
                    fprintf(fp,"\tXOR EAX, EAX\n");
                    fprintf(fp,"\tXOR EBX, EBX\n");
                    
                    if(!strcmp(var->ele.data.id.type, "INTEGER"))
                    {
                        // fprintf(fp,"MOV AX, 2d\n");
                        if(var->isParameter == 0)
                            fprintf(fp,"\tMOV EBX,[EBP-8-%d]\n",var->offset); 
                        else
                            fprintf(fp,"\tMOV EBX,[EBP+%d]\n",var->offset); 
                        // fprintf(fp,"MOV [ESP],BX\n");
                        fprintf(fp,"\tPUSH EBX\n");
                    }
                    else if(!strcmp(var->ele.data.id.type,"REAL"))
                    {
                        // fprintf(fp,"MOV AX, 4d");
                        // fprintf(fp,"MOV EBX, %fd", *(float*)(var->ele.data.id.value));
                        fprintf(fp,"\tFINIT\n");
                        fprintf(fp,"\tSUB ESP,8\n");
                        if(var->isParameter == 0)
                            fprintf(fp,"\tFLD qword [EBP-8-%d]\n",var->offset); 
                        else
                            fprintf(fp,"\tFLD qword [EBP+%d]\n",var->offset);

                        fprintf(fp,"\tFSTP qword [ESP]\n");     
                        //dont know how to load
                    }    
                    else if(!strcmp(var->ele.data.id.type, "BOOLEAN"))
                    { 
                        // fprintf(fp,"MOV AX, 1d\n");
                        if(var->isParameter == 0)
                            fprintf(fp,"\tMOV EBX,[EBP-8-%d]\n",var->offset); 
                        else
                            fprintf(fp,"\tMOV EBX,[EBP+%d]\n",var->offset); 
                        // fprintf(fp,"MOV [ESP],BL\n");          
                        fprintf(fp,"\tPUSH EBX\n");
                    }    
                    // fprintf(fp,"SUB ESP,EAX\n");
                }
            }
        }
        else if(!strcmp(trav->ele->op,"call"))
        {
            //arg1 : name of the module
            //arg2 : number of parameters
            //call : op

            fprintf(fp, "\tMOV EAX, ESP\n");
            fprintf(fp, "\tPUSH EBP\n");
            fprintf(fp, "\tMOV EBP, EAX\n");
            // fprintf(fp,"\tMOV EBP,ESP\n");
            fprintf(fp,"\tCALL %s\n",trav->ele->arg1);
        }
        else if(!strcmp(trav->ele->op,"inp"))
        {
            // simply pop it
            if(trav->ele->tag1 == NUM)
            {
                fprintf(fp, "\tPOP EAX\n");
            }
            else
            {
                symbolTableNode* var = searchScopeIRcode(tbStack, trav->ele->arg1);
                
                if(var->ele.tag==Array)
                {
                    fprintf(fp,"\tPOP EAX\n");
                    fprintf(fp,"\tPOP EAX\n");
                    // fprintf(fp,"\tPOP EAX\n");
                    // fprintf(fp,"\tPOP EAX\n");
                }
                else if(var->ele.tag==Identifier)
                {
                    if(!strcmp(var->ele.data.id.type, "INTEGER"))
                    {
                        fprintf(fp,"\tPOP EAX\n");
                    }
                    else if(!strcmp(var->ele.data.id.type, "REAL"))
                    {
                        fprintf(fp,"\tPOP EAX\n");
                        fprintf(fp,"\tPOP EAX\n");
                    }
                    else if(!strcmp(var->ele.data.id.type, "BOOLEAN"))
                    {
                        fprintf(fp,"\tPOP EAX\n");
                    }
                }
            }
        }
        else if(!strcmp(trav->ele->op,"out"))
        {
            if(trav->ele->tag1 == NUM)
            {
                fprintf(fp, "\tPOP EAX\n");
            }
            else
            {
                symbolTableNode * var = searchScopeIRcode(tbStack, trav->ele->arg1);
                if(!strcmp(var->ele.data.id.type, "INTEGER"))
                {
                    fprintf(fp,"\tPOP EAX\n");
                    if(var->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", var->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], EAX\n", var->offset);
                }
                else if(!strcmp(var->ele.data.id.type, "REAL"))
                {
                    //TODO
                    fprintf(fp,"\tFINIT\n");
                    fprintf(fp, "\tFLD qword [ESP]\n");
                    if(var->isParameter == 0)
                        fprintf(fp,"\tFSTP qword [EBP-8-%d]\n", var->offset);
                    else
                        fprintf(fp,"\tFSTP qword [EBP+%d]\n", var->offset);  
                    fprintf(fp,"\tADD ESP, 8d\n");
                }
                else if(!strcmp(var->ele.data.id.type, "BOOLEAN"))
                {
                    fprintf(fp,"\tPOP EAX\n");

                    if(var->isParameter == 0)
                        fprintf(fp,"\tMOV [EBP-8-%d], EAX\n", var->offset);
                    else
                        fprintf(fp,"\tMOV [EBP+%d], EAX\n", var->offset);
                }
            }
        }
        else if(!strcmp(trav->ele->op, "goto"))
        {
            fprintf(fp, "\tJMP %s\n", trav->ele->arg1);
        }
        else if(!strcmp(trav->ele->op,"RET"))
        {
            fprintf(fp,"\tMOV ESP, EBP\n");
            fprintf(fp,"\tSUB ESP, 8d\n");
            // fprintf(fp,"\tMOV EBP, [EBP]\n");
            fprintf(fp,"\tRET\n");
        }
        else if(!strcmp(trav->ele->op, "trigger"))
        {
            fprintf(fp, "\tPOP EBP\n");
            // fprintf(fp,"\tMOV ESP, EAX\n");
        }
        trav = trav->next;
    }
}

