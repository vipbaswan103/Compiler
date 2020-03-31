/*  AST Node to be used for type checking/semantic rule verification
    
    ASSIGNOP
        - if left side is some var from output_list, update isassigned
        - especifically handle if both operands are arrays
    GET_VAL 
        - if the var is some var from output_list, update isassigned
    ASSIGNOPARR 
        - type, lower, uppper
    MODULEASSIGNOP and MODULECALL
        - left side matches with the output_list (type, number)
        - is the function declared/defined above the use
    FOR 
        - flag with the index variable
    WHILE
        - expression must be boolean type
    PLUS, MINUS, MUL, DIV, AND, OR, LT, LE, GT, GE, EQ, NE
        - Type checking (Both operands must be same)
        - No operand can be array
    
    SWITCH 
        - the switch var can't be real
        - check the default clause (shouldn't exist for boolean switch var, must exist for integer switch var)

    Stack updates in scope !
        PROGRAM
        MODULE
        MODULEDEF
        MODULEDEC
        DRIVER
        WHILE
        SWITCH
        FOR
*/    
#include "semanticsDef.h"
#include "ast.h"
#include "symbolTable.h"

int listCount(astNode* head)
{
    int count=0;
    while(head!=NULL)
    {
        count++;
        head=head->sibling;
    }
    return count;
}


symbolTableNode *searchScope(tableStack *tbStack, astNode *key)
{
    //initialise the temporary stack
    tableStack *tempStack = (tableStack *)malloc(sizeof(tableStack));
    symbolTableNode *ret = NULL;
    tableStackEle *temp = NULL;

    
    //For, while, switch
    //ModuleDef
    //Module
    //Module Dec
    //Program
    
    //its not the scope of module declarations and I did not find anything, put it on the other stack
    while((strcmp(tbStack->top->ele->symLexeme, "Module Declarations"))
    && (ret = sym_hash_find(key->node->ele.leafNode->lexeme, &(tbStack->top->ele->hashtb), 0, NULL)) == NULL)
    {
        temp = sympop(tbStack);
        sympush(tempStack, temp);
    }

    //We reached the topmost symbol table (broadest scope), key doesn't exist in the symbol table
    if((!strcmp(tbStack->top->ele->symLexeme, "Module Declarations")))
    {
        //Not declared, ERROR
        
        //TODO
        //TODO
        //TODO
        //TODO

        return NULL;
    }
    else
    {
        //Check whether declaration is above or below the definition (using lineNumbers)
        //TODO check if line number is the same for usage and declaration in the same/different scope, considering
        //everything is written in the same line 
        if(ret->lineNum >= key->node->ele.leafNode->lineNum)
        {
            //Declaration is below definition
            //Not declared, ERROR
            return NULL;
        }
        while(tempStack->size  != 0)
        {
            temp = sympop(tempStack);
            sympush(tbStack, temp);
        }
        free(tempStack);
        return ret;
    }
}

type * typeChecker(astNode * currentNode, tableStack * tbStack)
{
    if(!strcmp(currentNode->node->ele.internalNode->label, "ASSIGNOP"))
    {
        /*2 children:
            1) ID_node
            2) expression
        */ 

        //find which scope is the LHS of the assignment
        symbolTableNode *ret = searchScope(tbStack, currentNode->child);
        
        if(ret == NULL) //Error as ID_node not defined
            return NULL;
        
        if(ret->ele.tag == Array)
        {
            //ID_node can't be an array, ERROR
        }

        ret->ele.data.id.isAssigned = 1;    //ID_node has been assigned something
        type *rightType = typeChecker(currentNode->child->sibling, tbStack);

        if((rightType->tag == ArrayType) || strcmp(ret->ele.data.id.type, rightType->tp.type))
        {
            //Type mismatch, ERROR
        }
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "GET_VALUE"))
    {
        /*  1 Child:
                1) ID_node
        */
        symbolTableNode *ret= searchScope(tbStack, currentNode->child);
        if(ret == NULL)
            return NULL;
        
        if(ret->ele.tag == Array)
        {
            //can't be array varible, ERROR
            return NULL;
        }
        ret->ele.data.id.isAssigned = 1;
        return NULL;
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "ASSIGNOPARR"))
    {
        /*  2 Children
                1) ID_node
                2) Index_node
                3) RHS_expression
        */
        symbolTableNode *leftType_id = searchScope(tbStack, currentNode->child);
        
        //the left side id should be an array types
        if(leftType_id->ele.tag != Array)
        {
            //error
        }
        
        
        //the left side index should be an integer 
        if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "RNUM"))
        {
            //Index can't be a real constant, ERROR
        }
        else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "NUM"))
        {
            //Its OK to have a NUM as index, but still check the bounds if the array is static
            astNode *num = currentNode->child->sibling;
            if(leftType_id->ele.data.arr.isDynamic == 0)
            {
                //bound checking
                if(num->node->ele.leafNode->value < leftType_id->ele.data.arr.lowerIndex->value
                    || num->node->ele.leafNode->value > leftType_id->ele.data.arr.upperIndex->value)
                {
                    //out of bounds error
                }
            }
            
        }
        else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "ID"))
        {
            //the index is not a RNUM, or a NUM -> Its an ID, so search in the symbol Table
            symbolTableNode * index = searchScope(tbStack, currentNode->child->sibling);
            if(index == NULL)
            {
                //Undeclared index ID, ERROR
                return NULL;
            }
            if(index->ele.tag != Identifier)
            {
                //index can't be an array var, ERROR
            }
            //the left side index should be within bounds of the array, if the array is static
            if(leftType_id->ele.data.arr.isDynamic == 0)
            {
                if(index->ele.data.id.value < leftType_id->ele.data.arr.lowerIndex->value
                    || index->ele.data.id.value > leftType_id->ele.data.arr.upperIndex->value)
                {
                    //out  of bounds error
                }
            }
        }
        
        //Everything on LHS is fine
        
        type * rightType = typeChecker(currentNode->child->sibling->sibling,tbStack);
        
        if(rightType == NULL)
        {
            //Some problem/error on the RHS_expression, return NULL
            return NULL;
        }        
        
        if(rightType->tag == ArrayType)
        {
            //RHS_expression can't be of an array type, ERROR
            return NULL;
        }
        //right and left type should match finally
        if(strcmp(leftType_id->ele.data.arr.type, rightType))
        {
            //Type mismatch, ERROR
        }
        return NULL;
    } 
    // else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULEASSIGNOP"))
    // {
    //     /*  2 Children
    //             1) ID_list
    //             2) Modulecall
    //                 2.1) Func_name
    //                 2.2) Id_list_node
    //     */

    // } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULECALL"))
    {
        /*  3 Children
                1) ID_list_node (return)
                2) Func_name
                3) ID_list_node
        */

        /*Check if the func is already defined in program scope (defined functions)*/
        symbolTableNode *ret = sym_hash_find(currentNode->child->sibling->node->ele.leafNode->lexeme, &(symbolTableRoot->hashtb), 0, NULL);

        //Function is not defined
        if(ret == NULL)
        {
            //Check whether it is declared in declaration scope
            ret = sym_hash_find(currentNode->child->sibling->node->ele.leafNode->lexeme, &(symbolTableRoot->child->hashtb), 0, NULL);
            
            //It is not declared
            if(ret == NULL)
            {
                //Function neither declared nor defined, ERROR
                return NULL;
            }
        }
        
        /*Now compare the ID_list_node with input_list of the function*/
        int inputsize = listCount(currentNode->child->sibling->sibling);
        
        if(inputsize != ret->ele.data.mod.inputcount)
        {
            //No of input parameters mismatch, ERROR
            return NULL;
        }

        astNode * trav = currentNode->child->sibling->sibling;
        for(int i=0; i<inputsize; i++)
        {
            //its an ID
            symbolTableNode *id_arr = searchScope(tbStack, trav);

            //trav isn't declared
            if(id_arr == NULL)
            {
                //Not declared, ERROR
                return NULL;
            }
            
            //tags match
            if(ret->ele.data.mod.inputList[i].tag == id_arr->ele.tag)
            {
                // BOTH ARE IDs //
                if(id_arr->ele.tag == Identifier)
                {
                    if(strcmp(id_arr->ele.data.id.type, ret->ele.data.mod.inputList[i].data.id.type))
                    {
                        //Type mismatch, ERROR
                        return NULL;
                    }
                }

                // BOTH ARE ARRAYS //
                else
                {
                    int isError = 0;
                    //Check basic type
                    if(strcmp(id_arr->ele.data.arr.type, ret->ele.data.mod.inputList[i].data.arr.type))
                    {
                        isError = 1;
                        //Type mismatch, ERROR
                        //arrays are of different type
                    }
                    //Check bounds
                    if(isError = 0)
                    {
                        //check the lower index matches when they are static NUMS
                        if(!strcmp(ret->ele.data.mod.inputList[i].data.arr.lowerIndex->type, "NUM") && 
                        !strcmp(id_arr->ele.data.arr.lowerIndex, "NUM"))
                        {
                            if(ret->ele.data.mod.inputList[i].data.arr.lowerIndex->value != id_arr->ele.data.arr.lowerIndex->value)
                            {
                                //error
                                isError = 2;
                            }
                        }
                        //check the upper index matches when they are static NUMS
                        if(!strcmp(ret->ele.data.mod.inputList[i].data.arr.upperIndex->type, "NUM") && 
                        !strcmp(id_arr->ele.data.arr.upperIndex, "NUM"))
                        {
                            if(ret->ele.data.mod.inputList[i].data.arr.upperIndex->value != id_arr->ele.data.arr.upperIndex->value)
                            {
                                //error
                                isError = 2;
                            }
                        }
                    }

                    if(isError==1)
                    {
                        //Type mismatch, ERROR
                    }
                    else if(isError==2)
                    {
                        //bounds mismatch ERROR
                    }
                    
                }
            }
            else    //tags don't match, one is array and other is ID
            {
                //Type mismatch, ERROR
            }
        }

        /*Now compare the ID_list_node (return) with the output_list of the function*/
        int outputsize = listCount(currentNode->child);
        if(inputsize != ret->ele.data.mod.outputcount)
        {
            //No of input parameters mismatch, ERROR
            return NULL;
        }

        astNode * trav = currentNode->child;
        for(int i=0; i<outputsize; i++)
        {
            //its an ID
            symbolTableNode *id_arr = searchScope(tbStack, trav);

            //trav isn't declared
            if(id_arr == NULL)
            {
                //Not declared, ERROR
                return NULL;
            }
            //tags match
            if(ret->ele.data.mod.inputList[i].tag == id_arr->ele.tag)
            {
                // BOTH ARE IDs //
                if(id_arr->ele.tag == Identifier)
                {
                    if(strcmp(id_arr->ele.data.id.type, ret->ele.data.mod.inputList[i].data.id.type))
                    {
                        //Type mismatch, ERROR
                        return NULL;
                    }
                }

                // BOTH ARE ARRAYS //
                else
                {
                    //Error, as you can't return an array variable from the function
                }
            }
            else    //tags don't match, one is array and other is ID
            {
                //Type mismatch, ERROR
            }
        }
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "FOR"))
    {
        /*  3 Children
                1) ID_node
                2) Range_node
                3) stmts_list
        */
        /*New scope starts, push the corresponding symbol table on top of the stack*/

        symbolTable *currenSTNode = tbStack->top->ele;

        tableStackEle *newTable = (tableStackEle*)malloc(sizeof(tableStackEle));
        newTable->ele = currenSTNode->child;
        newTable->next = NULL;
        sympush(tbStack, newTable);
        currenSTNode = currenSTNode->child;
        
        astNode *trav = currentNode->child->sibling->sibling;
        while(trav != NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") || !strcmp(trav->node->ele.internalNode->label, "SWITCH") || !strcmp(trav->node->ele.internalNode->label, "WHILE"))
            {
                newTable = sympop(tbStack);
                free(newTable);
                newTable->ele = currenSTNode->sibling;
            }
            trav = trav->sibling;
        }
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "WHILE"))
    {
        
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "SWITCH"))
    {
        
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "PLUS") || 
    !strcmp(currentNode->node->ele.internalNode->label, "MINUS") || 
    !strcmp(currentNode->node->ele.internalNode->label, "MUL") || 
    !strcmp(currentNode->node->ele.internalNode->label, "DIV") || 
    !strcmp(currentNode->node->ele.internalNode->label, "LT") || 
    !strcmp(currentNode->node->ele.internalNode->label, "GT") ||
    !strcmp(currentNode->node->ele.internalNode->label, "LE") || 
    !strcmp(currentNode->node->ele.internalNode->label, "GE") || 
    !strcmp(currentNode->node->ele.internalNode->label, "EQ") || 
    !strcmp(currentNode->node->ele.internalNode->label, "NE") || 
    !strcmp(currentNode->node->ele.internalNode->label, "AND") || 
    !strcmp(currentNode->node->ele.internalNode->label, "OR"))
    {
        //If its a binary expression
        if(currentNode->child->sibling != NULL)
        {
            type *leftType = typeChecker(currentNode->child, tbStack);
            type *rightType = typeChecker(currentNode->child->sibling, tbStack);

            if(leftType->tag == ArrayType || rightType->tag == ArrayType)
            {
                //None of the operands can be of array type, ERROR
                return NULL;
            }

            type *retType = (type *)malloc(sizeof(type));
            if(!strcmp(currentNode->node->ele.internalNode->label, "PLUS") || 
            !strcmp(currentNode->node->ele.internalNode->label, "MINUS") || 
            !strcmp(currentNode->node->ele.internalNode->label, "MUL") || 
            !strcmp(currentNode->node->ele.internalNode->label, "DIV"))
            {
                if(!strcmp(leftType->tp.type, rightType->tp.type))
                {
                    if(!strcmp(leftType->tp.type, "INTEGER"))
                    {
                        retType->tag = IdentifierType;
                        retType->tp.type = "INTEGER\0";
                    }
                    else if(!strcmp(leftType->tp.type, "REAL"))
                    {
                        retType->tag = IdentifierType;
                        retType->tp.type = "REAL\0";
                    }
                    else if(!strcmp(leftType->tp.type, "BOOLEAN"))
                    {
                        //Type can't be boolean, ERROR
                        free(retType);
                        return  NULL;
                    }           
                    return retType;         
                }
                else
                {
                    //Type mismatch among operands, ERROR
                    free(retType);
                    return NULL;
                }
            }
            if(!strcmp(currentNode->node->ele.internalNode->label, "LT") || 
            !strcmp(currentNode->node->ele.internalNode->label, "GT") ||
            !strcmp(currentNode->node->ele.internalNode->label, "LE") || 
            !strcmp(currentNode->node->ele.internalNode->label, "GE") || 
            !strcmp(currentNode->node->ele.internalNode->label, "EQ") || 
            !strcmp(currentNode->node->ele.internalNode->label, "NE"))
            {
                if(!strcmp(leftType->tp.type, rightType->tp.type))
                {
                    if(!strcmp(leftType->tp.type, "INTEGER") || !strcmp(leftType->tp.type, "REAL"))
                    {
                        retType->tag = IdentifierType;
                        retType->tp.type = "BOOLEAN\0";
                    }
                    else if(!strcmp(leftType->tp.type, "BOOLEAN"))
                    {
                        //Type can't be boolean, ERROR
                        free(retType);
                        return  NULL;
                    }           
                    return retType;         
                }
                else
                {
                    //Type mismatch among operands, ERROR
                    free(retType);
                    return NULL;
                }
            }
            if(!strcmp(currentNode->node->ele.internalNode->label, "AND") || 
            !strcmp(currentNode->node->ele.internalNode->label, "OR"))
            {
                if(!strcmp(leftType->tp.type, rightType->tp.type))
                {
                    if(!strcmp(leftType->tp.type, "BOOLEAN"))
                    {
                        retType->tag = IdentifierType;
                        retType->tp.type = "BOOLEAN\0";
                    }
                    else
                    {
                        //Type can't be boolean, ERROR
                        free(retType);
                        return  NULL;
                    }           
                    return retType;         
                }
                else
                {
                    //Type mismatch among operands, ERROR
                    free(retType);
                    return NULL;
                }
            }
        }
        //If its a unary expression
        else
        {
            type *childType = typeChecker(currentNode->child, tbStack);
            if(childType->tag == ArrayType)
            {
                //Operand can't be of an array type, ERROR
                return NULL;
            }
            type *retType = (type *)malloc(sizeof(type));
            if(!strcmp(childType->tp.type, "INTEGER"))
            {
                retType->tag = IdentifierType;
                retType->tp.type = "INTEGER";
                return retType;
            }
            else if(!strcmp(childType->tp.type, "REAL"))
            {
                retType->tag = IdentifierType;
                retType->tp.type = "REAL";
                return retType;
            }
            else
            {
                //Type of operand is wrong, ERROR
                free(retType);
                return NULL;
            }
        }
    } 
}