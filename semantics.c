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

symbolTable *searchScope(tableStack *tbStack, char*lexeme)
{
    tableStack *tempStack = (tableStack *)malloc(sizeof(tableStack));
    symbolTableNode *ret = NULL;
    tableStackEle *tmp = NULL;
    while((!strcmp(tbStack->top->ele->symLexeme, "Program")) && (ret = sym_hash_find(lexeme, &(tbStack->top->ele->hashtb), 0, NULL)) == NULL)
    {
        tmp = sympop(tbStack);
        sympush(tempStack, tmp);
    }

    if((!strcmp(tbStack->top->ele->symLexeme, "Program")))
    {
        //Not defined, ERROR
        return NULL;
    }
    else
    {
        while(tempStack->size  != 0)
        {
            sympush(tbStack, tempStack->top);
            tmp = sympop(tempStack);
            free(tmp);
        }
        return &(tbStack->top->ele->hashtb);
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
        symbolTable *apprSym = searchScope(tbStack, currentNode->child->node->ele.leafNode->lexeme);
        
        if(apprSym == NULL) //Error as LHS not defined
            return NULL;

        // Make semantic error node here    
        type *rightType = typeChecker(currentNode->child->sibling, tbStack);
        symbolTableNode * ret = sym_hash_find(currentNode->child->node->ele.leafNode->lexeme, &(apprSym->top->ele->hashtb), 0, NULL);
        
        //LHS ID is defined, check that 

        //1) LHS var can't be an array
        if(ret->ele.tag == Array)
        {
            //LHS can't be an array, ERROR
        }
        //2) Its type must match RHS type
        else if(strcmp(ret->ele.data.id.type, rightType->tp.type))
        {
            //Type mismatch, ERROR
        }
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "GET_VALUE"))
    {
        symbolTable *apprSym = searchScope(tbStack, currentNode->child->node->ele.leafNode->lexeme);
        if(apprSym)
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "ASSIGNOPARR"))
    {
        type * leftType_id = typeChecker(currentNode->child,tbStack);
        
        //the left side id should be an array type
        if(leftType_id->tag!=1)
        {
            //error
        }
                     
        type * leftType_index = typeChecker(currentNode->child->sibling,tbStack);
        //the left side index should be an integer 
        if(leftType_index->tag!=0)
        {
            //error
        }
        //the left side index should be within bounds of the array
        if()
        {
            //error
        }

        type * rightType = typeChecker(currentNode->child->sibling->sibling,tbStack);
        //right and left type should match finally
        if()
        {
            //error
        }        
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULEASSIGNOP"))
    {
        
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULECALL"))
    {
        
    } 
}