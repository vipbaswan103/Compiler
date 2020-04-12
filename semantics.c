/*Check for cases where we need to search in a different scope (not in the current scope)*/
/*Testing for error conditions (when returned type is NULL)*/
/*Checking line numbers for declaration errors (everything in the same line)*/

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
#include "semantics.h"
/*Gives the count of elements in the list pointed by head*/
int listCount(astNode* head)
{
    int count=0;
    while(head != NULL)
    {
        count++;
        head = head->sibling;
    }
    return count;
}

void pushSemanticError(char *str)
{
    semanticErrorNode *errNode = (semanticErrorNode *)malloc(sizeof(semanticErrorNode));
    char * err = (char*)malloc(sizeof(char)*200);
    strcpy(err,str);
    errNode->errorMessage = err;
    errNode->next = NULL;
    insertSemError(errNode);
}

///only searches for variables/identifiers in the stack of scopes
//does not examine module declarations or programs
symbolTableNode *searchScope(tableStack *tbStack, astNode *key)
{
    //initialise the temporary stack
    tableStack *tempStack = (tableStack *)malloc(sizeof(tableStack));
    tempStack->top = NULL;
    tempStack->size = 0;
    tempStack->bottom = NULL;

    symbolTableNode *ret = NULL;
    tableStackEle *temp = NULL;
    
    //its not the scope of module declarations and I did not find anything, put it on the other stack
    while((strcmp(tbStack->top->ele->symLexeme, "Module Declarations"))
    && (ret = sym_hash_find(key->node->ele.leafNode->lexeme, &(tbStack->top->ele->hashtb), 0, NULL)) == NULL)
    {
        temp = sympop(tbStack);
        sympush(tempStack, temp);
    }
    char * err = (char *)malloc(sizeof(char)*200);
    memset(err, '\0', sizeof(char)*200);
    //We reached the topmost symbol table (broadest scope), key doesn't exist in the symbol table tree
    if((!strcmp(tbStack->top->ele->symLexeme, "Module Declarations")))
    {
        //Not declared, ERROR
        
        sprintf(err, "Line %d: %s variable is not declared.", 
        key->node->ele.leafNode->lineNum, key->node->ele.leafNode->lexeme);
        pushSemanticError(err);
        while(tempStack->size  != 0)
        {
            temp = sympop(tempStack);
            sympush(tbStack, temp);
        }
        // free(tempStack);
        return NULL;
    }
    
    // You had found the variable in some scope
    else
    {
        //Check whether declaration is above or below the definition (using lineNumbers)
        //TODO check if line number is the same for usage and declaration in the same/different scope, considering
        //everything is written in the same line 
    
        if(ret->aux == 0)   //Variable hasn't been decalared
        {
            //ERROR
            symbolTableNode *tmp = (symbolTableNode *)malloc(sizeof(symbolTableNode));
            *tmp = *ret;
            
            temp = sympop(tbStack);
            sympush(tempStack, temp);
            
            while((strcmp(tbStack->top->ele->symLexeme, "Module Declarations"))
            && (ret = sym_hash_find(key->node->ele.leafNode->lexeme, &(tbStack->top->ele->hashtb), 0, NULL)) == NULL)
            {
                temp = sympop(tbStack);
                sympush(tempStack, temp);
            }

            if((!strcmp(tbStack->top->ele->symLexeme, "Module Declarations")))
            {
                //Not declared, ERROR
                sprintf(err, "Line %d: %s variable not declared (Declaration at %d, should be done before use).", 
                key->node->ele.leafNode->lineNum, key->node->ele.leafNode->lexeme, tmp->lineNum);
                pushSemanticError(err);
                while(tempStack->size  != 0)
                {
                    temp = sympop(tempStack);
                    sympush(tbStack, temp);
                }
                // free(tempStack);
                return NULL;
            }
        }

        //Push everything in tempStack into the tbStack
        while(tempStack->size  != 0)
        {
            temp = sympop(tempStack);
            sympush(tbStack, temp);
        }
        // if(err!=NULL) free(err);
        // if(tempStack!=NULL) free(tempStack);
        return ret;
    }
}

type * typeChecker(astNode * currentNode, tableStack * tbStack)
{
    if(currentNode == NULL)
    {
        return NULL;
    }
    if(currentNode->node->tag == Leaf)
    {
        //TODO: Add code for leaf node   
        //DON NOT FORGET TO RETURN FROM FUNCTION
        //DO NOT MOVE ON
        
        type *typeLeaf = (type*)malloc(sizeof(type));
        if(!strcmp(currentNode->node->ele.leafNode->type, "TRUE") 
        || !strcmp(currentNode->node->ele.leafNode->type, "FALSE"))
        {
            typeLeaf->tag = IdentifierType;
            typeLeaf->tp.type = "BOOLEAN\0";
        }
        else if(!strcmp(currentNode->node->ele.leafNode->type, "NUM"))
        {
            typeLeaf->tag = IdentifierType;
            typeLeaf->tp.type = "INTEGER\0";
        }
        else if(!strcmp(currentNode->node->ele.leafNode->type, "RNUM"))
        {
            typeLeaf->tag = IdentifierType;
            typeLeaf->tp.type = "REAL\0";
        }
        else if(!strcmp(currentNode->node->ele.leafNode->type,"ID"))
        {
            symbolTableNode *idNode = searchScope(tbStack, currentNode);
            
            if(idNode == NULL)
            {
                // ERROR: Undeclared variable
                // if(typeLeaf!=NULL) free(typeLeaf);
                return NULL;    
            }
            if(idNode->ele.tag == Identifier)
            {
                typeLeaf->tag = IdentifierType;
                typeLeaf->tp.type = idNode->ele.data.id.type;
            }
            else if(idNode->ele.tag == Array)
            {
                typeLeaf->tag = ArrayType;
                typeLeaf->tp.arr.basicType = idNode->ele.data.arr.type;
                typeLeaf->tp.arr.lowerBound = idNode->ele.data.arr.lowerIndex;
                typeLeaf->tp.arr.upperBound = idNode->ele.data.arr.upperIndex;
            }
        }
        else
        {
            // ERROR : Invalid Leaf Node
            printf("Shouldn't have reached here\n");
            // if(typeLeaf!=NULL) free(typeLeaf);
            return NULL;
        }
        return typeLeaf;
    }

    
    //psuhes both program and module declarations scope tables together
    if(!strcmp(currentNode->node->ele.internalNode->label, "PROGRAM"))
    {
        /*
            4 Children:
                1) ModuleDec
                2) Othermodules
                3) driver modules
                4) othermodules
        */

        // creating and pushing the program table
        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->next = NULL;
        newTable->ele = symbolTableRoot;
        sympush(tbStack,newTable);
        
        // creating and pushing the moduledec table
        newTable= (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->next = NULL;
        newTable->ele = symbolTableRoot->child;
        sympush(tbStack, newTable);

        //Push
        astNode *trav = currentNode->child->sibling;
        while(trav != NULL)
        {
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }

        //Pop the ModuleDec symbol table
        newTable = sympop(tbStack);
        // if(newTable!=NULL) free(newTable);

        //Pop the Program symbol table
        newTable = sympop(tbStack);
        // if(newTable!=NULL) free(newTable);
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULES1"))
    {
        astNode * trav = currentNode->child;
        tableStackEle *newTable = NULL;

        symbolTable *st = symbolTableRoot->child->sibling;

        //Iterate over all the modules and push their corresponding scope
        // on the stack (and recursively call typechecker)

        int x = 0;
        while(strcmp(st->symLexeme, "Driver") && trav != NULL)
        {
            x++;
            newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
            newTable->ele = st;
            newTable->next = NULL;
            sympush(tbStack, newTable);
            st = st->sibling;
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }

        // while(x > 0)
        // {
        //     free(sympop(tbStack));
        //     x--;
        // }
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULES2"))
    {
        astNode * trav = currentNode->child;
        tableStackEle *newTable = NULL;

        //Driver must exist (syntactic correctness)
        symbolTable *st = symbolTableRoot->child->sibling;
        while(strcmp(st->symLexeme, "Driver"))
        {
            st = st->sibling;
        }
        st = st->sibling;

        // st is now pointing to the symbol table of these "othermodules2"

        //Iterate over all the modules and push their corresponding scope on 
        //the stack (and recursively call typechecker)
        int x = 0;
        while(trav != NULL)
        {
            x++;
            newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
            newTable->ele = st;
            newTable->next = NULL;
            //Always remember pushing this table on stack is responsibility of this parent node
            //popping will be responsibility of the node itself

            //Also remember that these are the symbol tables of the "module" level
            //They were made for the input list and output list storage (intermediate levels)
            sympush(tbStack, newTable);
            st = st->sibling;
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
        // while(x > 0)
        // {
        //     free(sympop(tbStack));
        //     x--;
        // }
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULE"))
    {
        //has four children
        //ModuleName, InputPlist, OutputPList, ModuleDef
        
        symbolTableNode *ret = sym_hash_find(tbStack->top->ele->symLexeme, &(symbolTableRoot->hashtb), 0, NULL);
        ret->aux = 1;
        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->ele = tbStack->top->ele->child;
        newTable->next = NULL;

        symbolTableNode *st = ret;
        symbolTableNode *tmp = NULL;
        
        astNode * trav = currentNode->child->sibling->sibling->child;
        while(trav != NULL)
        {
            tmp = sym_hash_find(trav->node->ele.leafNode->lexeme, &(tbStack->top->ele->hashtb), 0, NULL);
            tmp->aux = 1;   
            trav = trav->sibling->sibling;
        }
        
        trav = currentNode->child->sibling->child;
        
        while(trav != NULL)
        {
            tmp = sym_hash_find(trav->node->ele.leafNode->lexeme, &(tbStack->top->ele->hashtb), 0, NULL);
            tmp->aux = 1;
            trav = trav->sibling->sibling;
        }
        //Push the moduleDef's symbol table on the stack
        sympush(tbStack, newTable);
    
        //Call typeChecker on moduleDef
        typeChecker(currentNode->child->sibling->sibling->sibling, tbStack);
        
        char * err = (char *)malloc(sizeof(char)*200);
        memset(err, '\0', sizeof(char)*200);

        trav = currentNode->child->sibling->sibling->child;
        while(trav != NULL)
        {
            // if(tmp->ele.tag == Identifier)
            // {
            tmp = sym_hash_find(trav->node->ele.leafNode->lexeme, &(tbStack->top->ele->hashtb), 0, NULL);
            if(tmp->ele.data.id.isAssigned == 0)
            {
                //Some var in O/P is unassigned, ERROR.
                sprintf(err,"Line %d: %s (returned) variable not assigned in module %s", 
                tmp->lineNum, tmp->ele.data.id.lexeme, currentNode->child->node->ele.leafNode->lexeme);
                pushSemanticError(err);
                // newTable = sympop(tbStack);
                // free(newTable);
                // return NULL;
            }
            // }
            trav = trav->sibling->sibling;
        }
        //Pop this module's symbol table (moduleDef's symbol table will be popped in its own scope)
        // if(err!=NULL) free(err);
        newTable = sympop(tbStack);
        // if(newTable!=NULL) free(newTable);
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "DRIVER"))
    {
        symbolTable *trav = symbolTableRoot->child->sibling;
        while(strcmp(trav->symLexeme, "Driver"))
        {
            trav = trav->sibling;
        }

        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->ele = trav;
        newTable->next = NULL;
        //Push the driver's symbol table on the stack
        sympush(tbStack, newTable);
        
        newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->ele = trav->child;
        newTable->next = NULL;
        //Push the moduleDef's symbol table on the stack
        sympush(tbStack, newTable);

        //Call type checker on driver's moduleDef
        typeChecker(currentNode->child, tbStack);

        tableStackEle *table = sympop(tbStack);
        // if(table!=NULL) free(table);
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULEDEF"))
    {
        //traverse the statements inside the moduledef 
        astNode *trav = currentNode->child;
        tableStackEle *newTable = NULL;
        symbolTable *currentSTNode = tbStack->top->ele->child;
        
        //Iterate for all the statements in the moduleDef and recursively call typeChecker
        //If required, push the new scope on the stack
        while(trav != NULL)
        {
            //A special check for the "FOR" statement (bound check)
            if(!strcmp(trav->node->ele.internalNode->label, "FOR")) 
            {
                if(*(int*)trav->child->sibling->child->node->ele.leafNode->value > 
                *(int*)trav->child->sibling->child->sibling->node->ele.leafNode->value)
                {
                    // printf("Reached here \n");
                    char * err=  (char*)malloc(sizeof(char)*200);
                    memset(err, '\0', sizeof(char)*200);
                    sprintf(err,"Line %d: Lower bound of the FOR loop should be less then upper bound",
                    trav->child->sibling->child->node->ele.leafNode->lineNum);
                    pushSemanticError(err);
                    // free(err);
                }
            }
            
            //New scope starts for FOR, WHILE, SWITCH
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") 
            || !strcmp(trav->node->ele.internalNode->label, "WHILE") 
            || !strcmp(trav->node->ele.internalNode->label, "SWITCH"))
            {
                newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
                newTable->ele = currentSTNode;
                newTable->next = NULL;
                //Always remember pushing this table on stack is responsibility of this parent node
                //popping will be responsibility of the node itself
                sympush(tbStack, newTable);
                
                //move to the right in the n-ary tree of the tables also
                currentSTNode = currentSTNode->sibling;
            }
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
        
        //Pop this scope from the stack (as this moduleDef is over)
        newTable = sympop(tbStack);
        // if(newTable !=NULL) free(newTable);
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "DECLARE"))
    {
        //Declare AST node has 2 children
        // 1) Id List
        // 2) datatype
        astNode *trav = currentNode->child->child;
        astNode *type = currentNode->child->sibling;
        symbolTableNode *newNode = NULL;
        while(trav != NULL)
        {
            newNode = sym_hash_find(trav->node->ele.leafNode->lexeme, &(tbStack->top->ele->hashtb), 0, NULL);
            if(type->node->tag == Internal)
            {
                newNode->ele.tag = Array;
                newNode->ele.data.arr.lexeme = trav->node->ele.leafNode->lexeme;
                newNode->ele.data.arr.type = type->child->sibling->node->ele.leafNode->type;

                newNode->ele.data.arr.lowerIndex = (identifier*)malloc(sizeof(identifier));
                newNode->ele.data.arr.lowerIndex->lexeme = type->child->child->node->ele.leafNode->lexeme;
                newNode->ele.data.arr.lowerIndex->type = type->child->child->node->ele.leafNode->type;
                newNode->ele.data.arr.lowerIndex->value = type->child->child->node->ele.leafNode->value;

                newNode->ele.data.arr.upperIndex = (identifier*)malloc(sizeof(identifier));
                newNode->ele.data.arr.upperIndex->lexeme = type->child->child->sibling->node->ele.leafNode->lexeme;
                newNode->ele.data.arr.upperIndex->type = type->child->child->sibling->node->ele.leafNode->type;
                newNode->ele.data.arr.upperIndex->value = type->child->child->sibling->node->ele.leafNode->value;
                
                newNode->lineNum = trav->node->ele.leafNode->lineNum;

                int tmp = 0;
                if(!strcmp(type->child->sibling->node->ele.leafNode->type,"INTEGER"))
                    tmp = INTEGER_SIZE;
                else if(!strcmp(type->child->sibling->node->ele.leafNode->type,"REAL"))
                    tmp = REAL_SIZE;
                else if(!strcmp(type->child->sibling->node->ele.leafNode->type,"BOOLEAN"))
                    tmp = BOOLEAN_SIZE;
                
                if( (!strcmp(newNode->ele.data.arr.lowerIndex->type,"ID")) || (!strcmp(newNode->ele.data.arr.upperIndex->type,"ID")) )
                {
                    //dynamic array
                    // newNode->width = POINTER_SIZE;
                    // newNode->offset = current->currentOffset;
                    // current->currentOffset += POINTER_SIZE;
                    newNode->ele.data.arr.isDynamic = 1;   
                }
                else    
                {
                    //static array
                    newNode->ele.data.arr.isDynamic = 0;
                    // int size = *(int *)newNode->ele.data.arr.upperIndex->value - *(int*)newNode->ele.data.arr.lowerIndex->value + 1;
                    // newNode->offset = current->currentOffset;
                    // newNode->width = tmp * size + POINTER_SIZE;
                    // current->currentOffset += newNode->width;
                }
                // newNode->next = NULL;
            }
            //Its an ID
            else
            {
                newNode->ele.tag = Identifier;

                newNode->ele.data.id.lexeme = trav->node->ele.leafNode->lexeme;
                newNode->ele.data.id.type = type->node->ele.leafNode->type;
                newNode->ele.data.id.value = trav->node->ele.leafNode->value;
                newNode->lineNum = trav->node->ele.leafNode->lineNum;
                
                // if(!strcmp(type->node->ele.leafNode->type,"INTEGER"))
                //     newNode->width = INTEGER_SIZE;
                // else if(!strcmp(type->node->ele.leafNode->type,"REAL"))
                //     newNode->width = REAL_SIZE;
                // else if(!strcmp(type->node->ele.leafNode->type,"BOOLEAN"))
                //     newNode->width = BOOLEAN_SIZE;
                    
                // newNode->next = NULL;
                // newNode->offset = current->currentOffset;
                // current->currentOffset += newNode->width;
            }
            newNode->aux = 1;
            trav = trav->sibling;   
        }

        //trav is now datatype
        trav = currentNode->child->sibling;
        symbolTableNode * st = NULL;
        if (!strcmp(trav->node->ele.internalNode->label,"ARRAY"))
        {
            //Check whether the lower/upper bounds of the array are of ID type,
            //If yes, then search for their declaration. If not found, give error
            if(!strcmp(trav->child->child->node->ele.leafNode->type, "ID"))
            {
                st = searchScope(tbStack, trav->child->child);

                if(st==NULL)
                {
                    //DO NOTHING 
                    //searchscope has caught the error
                }
                else
                if(st->ele.tag == Array)
                {
                    char * err=  (char*)malloc(sizeof(char)*200);
                    memset(err, '\0', sizeof(char)*200);
                    sprintf(err, "Line  %d: Lowerbound %s of dynamic array(s) can't be of Array type.",
                    trav->child->child->node->ele.leafNode->lineNum,
                    trav->child->child->node->ele.leafNode->lexeme);
                    pushSemanticError(err);
                    // free(err);
                }
                else if(st->ele.tag == Identifier)
                {
                    if(strcmp(st->ele.data.id.type, "INTEGER"))
                    {
                        char * err=  (char*)malloc(sizeof(char)*200);
                        memset(err, '\0', sizeof(char)*200);
                        sprintf(err, "Line  %d: Lowerbound %s of dynamic array(s) is of type %s (Bounds can only be of Integer type).",
                        trav->child->child->node->ele.leafNode->lineNum,
                        trav->child->child->node->ele.leafNode->lexeme,
                        st->ele.data.id.type);
                        pushSemanticError(err);
                        // free(err);
                    }
                }
            }

            if(!strcmp(trav->child->child->sibling->node->ele.leafNode->type, "ID"))
            {
                st = searchScope(tbStack, trav->child->child->sibling);

                if(st==NULL)
                {
                    //DO NOTHING
                    //searchscope has caught the error
                }
                else
                if(st->ele.tag == Array)
                {
                    char * err=  (char*)malloc(sizeof(char)*200);
                    memset(err, '\0', sizeof(char)*200);
                    sprintf(err, "Line  %d: Upperbound %s of dynamic array(s) can't be of Array type.",
                    trav->child->child->sibling->node->ele.leafNode->lineNum,
                    trav->child->child->sibling->node->ele.leafNode->lexeme);
                    pushSemanticError(err);
                    // free(err);
                }
                else if(st->ele.tag == Identifier)
                {
                    if(strcmp(st->ele.data.id.type, "INTEGER"))
                    {
                        char * err=  (char*)malloc(sizeof(char)*200);
                        memset(err, '\0', sizeof(char)*200);
                        sprintf(err, "Line  %d: Upperbound %s of dynamic array(s) is of type %s (Bounds can only be of Integer type).",
                        trav->child->child->sibling->node->ele.leafNode->lineNum,
                        trav->child->child->sibling->node->ele.leafNode->lexeme,
                        st->ele.data.id.type);
                        pushSemanticError(err);
                        // free(err);
                    }
                }
            }
            if((!strcmp(trav->child->child->node->ele.leafNode->type,"NUM")) 
            && (!strcmp(trav->child->child->sibling->node->ele.leafNode->type,"NUM"))
            && (*(int*)trav->child->child->node->ele.leafNode->value > *(int*)trav->child->child->sibling->node->ele.leafNode->value))
            {
                char * err=  (char*)malloc(sizeof(char)*200);
                memset(err, '\0', sizeof(char)*200);
                sprintf(err,"Line %d: Lower bound of the array(s) should be less then upper bound in declaration.",
                trav->child->child->node->ele.leafNode->lineNum);
                pushSemanticError(err);
                // free(err);
            }
        }

        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "ASSIGNOP"))
    {
        /*2 children:
            1) ID_node
            2) expression
        */ 

        //Search for ID_node in the symbol table tree
        symbolTableNode *ret = searchScope(tbStack, currentNode->child);

        //Find the rightExpression's type
        type *rightType = typeChecker(currentNode->child->sibling, tbStack);

        //ID_node is not declared, stop right here
        //searchscope function has already caught the error
        if(ret == NULL) 
        {
            return NULL;
        }    
        
        char * err = (char *)malloc(sizeof(char)*200);
        memset(err, '\0', sizeof(char)*200);
        //ID_node var is of type array
        if(ret->ele.tag != Identifier && ret->ele.tag != Array)
        {
            //ID_node can't be an array var or module's name, ERROR
            sprintf(err, "Line %d: '%s' variable can't be on LHS of an assignment because it isn't an ID.", 
            currentNode->child->node->ele.leafNode->lineNum, ret->ele.data.arr.lexeme);
            pushSemanticError(err);
            // if(err!=NULL) free(err);
            return NULL;
        }

        if(ret->ele.tag == Identifier && ret->ele.data.id.isIndex==1)
        {
            //Error catch
            //It's an index and I am in the scope of the corresponding for loop
            sprintf(err,"Line %d: '%s' variable can not be modified during FOR loop (index variable).", 
            currentNode->child->node->ele.leafNode->lineNum, ret->ele.data.id.lexeme); 
            pushSemanticError(err);
            // if(err!=NULL) free(err);
            return NULL;
        }
        
        if(ret->ele.tag == Identifier)
            ret->ele.data.id.isAssigned = 1;    //ID_node has been assigned something

        //Either there was an error in type calculation of rightExpression or the type is an arrayType or type doesn't match
        if(rightType == NULL)
        {
            // if(err!=NULL) free(err);
            // free(rightType);
            return NULL;
        }
        if(rightType->tag == ArrayType)
        {
            if(ret->ele.tag == Array)
            {
                if(strcmp(ret->ele.data.arr.type, rightType->tp.arr.basicType))
                {
                    //Type mismatch, ERROR
                    sprintf(err,"Line %d: '%s' variable's type (Array, %s) does not match the expression type (Array, %s).", 
                    currentNode->child->node->ele.leafNode->lineNum, ret->ele.data.arr.lexeme, ret->ele.data.id.type, rightType->tp.type);
                    pushSemanticError(err);
                    // if(rightType!=NULL) free(rightType);
                    // if(err!=NULL) free(err);
                    return NULL;
                }

                int isStatic = 0;

                if(!strcmp(rightType->tp.arr.lowerBound->type, "NUM") && !strcmp(rightType->tp.arr.upperBound->type, "NUM"))
                    isStatic = 1;
                if(ret->ele.data.arr.isDynamic == 0 && isStatic == 1)
                {
                    if(*(int *)ret->ele.data.arr.lowerIndex->value != *(int *)rightType->tp.arr.lowerBound->value ||
                    *(int *)ret->ele.data.arr.upperIndex->value != *(int *)rightType->tp.arr.upperBound->value)
                    {
                        sprintf(err, "Line %d: Bounds of array '%s' (%d - %d) doesn't match the bounds of array '%s' (%d - %d).",
                        currentNode->child->node->ele.leafNode->lineNum, ret->ele.data.arr.lexeme, 
                        *(int *)ret->ele.data.arr.lowerIndex->value, *(int *)ret->ele.data.arr.upperIndex->value,
                        currentNode->child->sibling->child->node->ele.leafNode->lexeme,
                        *(int *)rightType->tp.arr.lowerBound->value, *(int *)rightType->tp.arr.upperBound->value);

                        pushSemanticError(err);
                        // if(rightType!=NULL) free(rightType);
                        // if(err!=NULL) free(err);
                        return NULL;
                    }
                }
                // if(rightType!=NULL) free(rightType);
                // if(err!=NULL) free(err);
                return NULL;
            }
            else
            {
                sprintf(err,"Line %d: '%s' variable's type (%s) does not match the expression type (Array).", 
                currentNode->child->node->ele.leafNode->lineNum, ret->ele.data.id.lexeme, ret->ele.data.id.type);
                pushSemanticError(err);
                // if(rightType!=NULL) free(rightType);
                // if(err!=NULL) free(err);
                return NULL;
            }
        }
        if(strcmp(ret->ele.data.id.type, rightType->tp.type))
        {
            //Type mismatch, ERROR
            sprintf(err,"Line %d: '%s' variable's type (%s) does not match the expression type (%s).", 
            currentNode->child->node->ele.leafNode->lineNum,ret->ele.data.id.lexeme, ret->ele.data.id.type, rightType->tp.type);
            pushSemanticError(err);
            // if(rightType!=NULL) free(rightType);
            // if(err!=NULL) free(err);
            return NULL;
        }
        // if(err!=NULL) free(err);
        // if(rightType!=NULL) free(rightType);
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "GET_VAL"))
    {
        /*  1 Child:
                1) ID_node
        */

        //Search for ID_node in the symbol table tree
        symbolTableNode *ret= searchScope(tbStack, currentNode->child);

        //ID_node isn't declared, stop right here, searchscope must have inserted error
        if(ret == NULL)
            return NULL;
        
        char * err = (char *)malloc(sizeof(char)*200);
        memset(err, '\0', sizeof(char)*200);
        //ID_node var is of Array type

        

        if(ret->ele.tag == Module)
        {
            //Can't be array varible, ERROR
            sprintf(err,"Line %d: %s variable is of Module type (Not Allowed).", 
            currentNode->child->node->ele.leafNode->lineNum, 
            currentNode->child->node->ele.leafNode->lexeme);
            pushSemanticError(err);
            // if(err!=NULL) free(err);
            return NULL;
        }

        //ID_node has been assigned something
        if((ret->ele.tag == Identifier && ret->ele.data.id.isIndex==1))
        {
            //Error catch
            //It's an index and I am in the scope of the corresponding for loop
            sprintf(err,"Line %d: %s variable can not be modified during FOR loop (index variable).", 
            currentNode->child->node->ele.leafNode->lineNum, 
            currentNode->child->node->ele.leafNode->lexeme); 
            pushSemanticError(err);
            // if(err!=NULL) free(err);
            return NULL;
        }
        // if(err!=NULL) free(err);
        if(ret->ele.tag == Identifier)
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

        //Search for ID_node in the symbol table tree
        symbolTableNode *leftType_id = searchScope(tbStack, currentNode->child);

        // Right Expr type checking also done
        type * rightType = typeChecker(currentNode->child->sibling->sibling,tbStack);
        
        if(leftType_id == NULL)
            return NULL;
        
        char * err = (char *)malloc(sizeof(char)*200);
        memset(err, '\0', sizeof(char)*200);
        //The ID_node should be of Array type
        if(leftType_id->ele.tag != Array)
        {
            //Error
            sprintf(err,"Line %d: %s variable (%s) can not be referenced like an Array.", 
            currentNode->child->node->ele.leafNode->lineNum, 
            currentNode->child->node->ele.leafNode->lexeme, 
            leftType_id->ele.data.id.type);
            pushSemanticError(err);
            // if(err!=NULL) free(err);
            return NULL;
        }
        
        //The index should be of Integer type or NUM
        if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "RNUM"))
        {
            //Index can't be a real constant, ERROR
            sprintf(err,"Line %d: Index (%s) is real (Index of an array can't be a real constant).", 
            currentNode->child->sibling->node->ele.leafNode->lineNum, 
            currentNode->child->sibling->node->ele.leafNode->lexeme);
            pushSemanticError(err);
            // if(err!=NULL) free(err);
            return NULL;
        }
        else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "NUM"))
        {
            //Its OK to have a as index, but still check the bounds if the array is static
            astNode *num = currentNode->child->sibling;

            //Check if the array is static or not
            if(leftType_id->ele.data.arr.isDynamic == 0)
            {
                //Static array, do bound checking
                if(*(int *)(num->node->ele.leafNode->value) < *(int*)(leftType_id->ele.data.arr.lowerIndex->value)
                    || *(int *)(num->node->ele.leafNode->value) > *(int *)(leftType_id->ele.data.arr.upperIndex->value))
                {
                    //out of bounds errors
                    sprintf(err,"Line %d: Index (%s) is out of bounds for this Array - %s (%d - %d).",num->node->ele.leafNode->lineNum, num->node->ele.leafNode->lexeme, leftType_id->ele.data.arr.lexeme, *(int*)(leftType_id->ele.data.arr.lowerIndex->value), *(int*)(leftType_id->ele.data.arr.upperIndex->value));
                    pushSemanticError(err);
                    // if(err!=NULL) free(err);
                    return NULL;
                }
            }
        }
        else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "ID"))
        {
            //the index is not a RNUM, or a NUM -> Its an ID, so search in the symbol Table

            //Check whether index is declared or not
            symbolTableNode * index = searchScope(tbStack, currentNode->child->sibling);

            //Index is not declared
            if(index == NULL)
            {
                //Undeclared index ID, ERROR
                // if(err!=NULL) free(err);
                return NULL;
            }

            //Index must be an Identifier var (not an Array var)
            if(index->ele.tag != Identifier)
            {
                //index can't be an array var, ERROR
                sprintf(err,"Line %d: Index variable (%s) is of Array type (Index variable of an array can't be of an Array type).", 
                currentNode->child->sibling->node->ele.leafNode->lineNum, 
                currentNode->child->sibling->node->ele.leafNode->lexeme);
                pushSemanticError(err);
                // if(err!=NULL) free(err);
                return NULL;
            }
            
            //Index ID isn't of integer type 
            if(strcmp(index->ele.data.id.type, "INTEGER"))
            {
                //Error
                sprintf(err,"Line %d: Index variable (%s) is of %s type (Index variable of an array must be of Integer type).", 
                currentNode->child->sibling->node->ele.leafNode->lineNum, 
                currentNode->child->sibling->node->ele.leafNode->lexeme, 
                index->ele.data.id.type);
                pushSemanticError(err);
                // if(err!=NULL) free(err);
                return NULL;
            }
            
        }
        
        //Error while calculating type of the right side expression
        if(rightType == NULL)
        {
            //Some problem/error on the RHS_expression, return NULL
            // free(rightType);
            // if(err!=NULL) free(err);
            return NULL;
        }        
        
        //RHS expression can't be of an array type
        if(rightType->tag == ArrayType)
        {
            //RHS_expression can't be of an array type, ERROR
            sprintf(err,"Line %d: Type of LHS (%s) does not match the expression type (Array).", 
            currentNode->child->node->ele.leafNode->lineNum, 
            leftType_id->ele.data.arr.type);
            pushSemanticError(err);
            // if(rightType!=NULL )free(rightType);
            // if(err!=NULL) free(err);
            return NULL;
        }

        //Right and left type should finally match
        if(strcmp(leftType_id->ele.data.arr.type, rightType->tp.type))
        {
            //Type mismatch, ERROR
            sprintf(err,"Line %d: %s array type (%s) does not match the expression type (%s).", 
            currentNode->child->node->ele.leafNode->lineNum, 
            currentNode->child->node->ele.leafNode->lexeme, 
            leftType_id->ele.data.arr.type,
            rightType->tp.type);
            pushSemanticError(err);
            // if(rightType!=NULL )free(rightType);
            // if(err!=NULL) free(err);
            return NULL;
        }
        // if(rightType!=NULL )free(rightType);
        // if(err!=NULL) free(err);
        return NULL;
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULECALL"))
    {
        /*  3 Children
                1) ID_list_node (return)
                2) Func_name
                3) ID_list_node
        */
        
        /*Check if the func is already defined in program scope (defined functions)*/
        symbolTableNode *ret = sym_hash_find(currentNode->child->sibling->node->ele.leafNode->lexeme, &(symbolTableRoot->hashtb), 0, NULL);
        char * err = (char *)malloc(sizeof(char)*200);
        memset(err, '\0', sizeof(char)*200);
        
         //Function is not defined
        if(ret == NULL)
        {
            sprintf(err,"Line %d: %s module is not defined.", 
            currentNode->child->sibling->node->ele.leafNode->lineNum,
            currentNode->child->sibling->node->ele.leafNode->lexeme);
            pushSemanticError(err);
            // if(err!=NULL) free(err);   
            return NULL;
        }

        if(!strcmp(ret->ele.data.mod.lexeme , tbStack->top->ele->symLexeme))
        {
            // ERROR : Recursion not allowed
            sprintf(err,"Line %d: %s module has recursive call.", 
            currentNode->child->sibling->node->ele.leafNode->lineNum,
            currentNode->child->sibling->node->ele.leafNode->lexeme);
            pushSemanticError(err);
            // if(err!=NULL) free(err);
            return NULL;
        }
        //Function is defined
        //Check if its definition has been traversed yet (in this pass or not)
        if(!ret->aux)
        {
            //Declaration hasn't been used even once
            //Search for it in the declaration table
            ret = sym_hash_find(currentNode->child->sibling->node->ele.leafNode->lexeme, &(symbolTableRoot->child->hashtb), 0, NULL);     
            
            //Function not declared (but defined)
            if(ret == NULL)
            {
                // Function defined but not declared and occurs after it
                sprintf(err,"Line %d: %s module is not declared.", 
                currentNode->child->sibling->node->ele.leafNode->lineNum,
                currentNode->child->sibling->node->ele.leafNode->lexeme);
                pushSemanticError(err);
                // free(err);
                // return NULL;
            }
            //Function declared (but defined)
            else
            {
                // Setting the aux field which signifies that the declaration is used atleast once
                ret->aux = 1;
            }
        }
        else
        {
            //Declaration has been used at least once
            ret = sym_hash_find(currentNode->child->sibling->node->ele.leafNode->lexeme, &(symbolTableRoot->child->hashtb), 0, NULL);     
            if(ret != NULL)
            {
                if(!ret->aux)
                {
                    // ERROR (Redundant Declaration) : Declaration not used for module
                    sprintf(err,"Line %d: %s module has redundant declaration.", 
                    ret->lineNum,currentNode->child->sibling->node->ele.leafNode->lexeme);
                    pushSemanticError(err);
                    // free(err); 
                    // return NULL;
                }
            }
        }   
    
        
        /***********************************************************************/

        /*Now compare the ID_list_node with input_list of the function*/
        ret = sym_hash_find(currentNode->child->sibling->node->ele.leafNode->lexeme, &(symbolTableRoot->hashtb), 0, NULL);
        int inputsize = listCount(currentNode->child->sibling->sibling->child);
        
        
        int inputCountError = 0;
        if(inputsize != ret->ele.data.mod.inputcount)
        {
            //No of input parameters mismatch, ERROR
            sprintf(err,"Line %d: %s module call has input parameters mismatch (Number of parameters are different).", 
            currentNode->child->sibling->sibling->node->ele.internalNode->lineNumStart,
            ret->ele.data.mod.lexeme);
            pushSemanticError(err);
            // if(err!=NULL) free(err);
            inputCountError = 1;
        }

        

        //trav is pointing to head of the input ID list
        astNode * trav = currentNode->child->sibling->sibling->child;

        if(inputCountError == 0)
        {
            for(int i=0; i<inputsize; i++)
            {
                // printf(" %s ",ret->ele.data.mod.inputList[i].data.id.lexeme);
                // getchar(); getchar();

                //Search for trav in the symbol table tree
                symbolTableNode *id_arr = searchScope(tbStack, trav);

                // printf(" %s ",id_arr->ele.data.id.lexeme);
                // getchar(); getchar();

                //trav isn't declared
                if(id_arr == NULL)
                {
                    //Not declared, ERROR
                    //searchscope has already inserted not found errors!
                    continue; 
                    // return NULL;
                }
                
                //tags match (either both are Identifier or Array)
                if(ret->ele.data.mod.inputList[i].tag == id_arr->ele.tag)
                {
                    // Both are IDs
                    if(id_arr->ele.tag == Identifier)
                    {
                        //Type should match
                        if(strcmp(id_arr->ele.data.id.type, ret->ele.data.mod.inputList[i].data.id.type))
                        {
                            //Type mismatch, ERROR
                            sprintf(err,"Line %d: Type of actual parameter %s (%s) does not match the type of formal parameter %s (%s).", 
                            trav->node->ele.leafNode->lineNum, 
                            trav->node->ele.leafNode->lexeme, 
                            id_arr->ele.data.id.type, 
                            ret->ele.data.mod.inputList[i].data.id.lexeme, 
                            ret->ele.data.mod.inputList[i].data.id.type);
                            pushSemanticError(err);
                            // return NULL;
                        }
                    }

                    // Both are arrays
                    else
                    {
                        int isError = 0;
                        //Check basic type, should match
                        if(strcmp(id_arr->ele.data.arr.type, ret->ele.data.mod.inputList[i].data.arr.type))
                        {
                            isError = 1;
                            //Type mismatch, ERROR
                            //arrays are of different type
                            // sprintf(err,"Line %d: Type mismatch: Type of %s (%s, Array) and %s (%s, Array) doesn't match", 
                            // currentNode->child->sibling->sibling->node->ele.internalNode->lineNumStart,
                            // id_arr->ele.data.arr.lexeme,
                            // id_arr->ele.data.arr.type,
                            // ret->ele.data.mod.inputList[i].data.arr.lexeme,
                            // ret->ele.data.mod.inputList[i].data.arr.type);
                            // pushSemanticError(err);
                        }
                        //Check bounds (check only if types match)
                        if(isError == 0)
                        {
                            //check the lower index matches when they are static NUMs
                            if(!strcmp(ret->ele.data.mod.inputList[i].data.arr.lowerIndex->type, "NUM") && 
                            !strcmp(id_arr->ele.data.arr.lowerIndex->type, "NUM"))
                            {
                                if(*(int*)(ret->ele.data.mod.inputList[i].data.arr.lowerIndex->value) != *(int*)id_arr->ele.data.arr.lowerIndex->value)
                                {
                                    //error
                                    isError = 2;
                                }
                            }
                            //check the upper index matches when they are static NUMS
                            if(!strcmp(ret->ele.data.mod.inputList[i].data.arr.upperIndex->type, "NUM") && 
                            !strcmp(id_arr->ele.data.arr.upperIndex->type, "NUM"))
                            {
                                if(*(int*)ret->ele.data.mod.inputList[i].data.arr.upperIndex->value != *(int *)id_arr->ele.data.arr.upperIndex->value)
                                {
                                    //error
                                    isError = 2;
                                }
                            }
                        }

                        if(isError==1)
                        {
                            //Type mismatch, ERROR
                            sprintf(err,"Line %d: Type of actual parameter %s (Array - %s) does not match the type of formal parameter %s (Array - %s).", 
                            trav->node->ele.leafNode->lineNum, 
                            trav->node->ele.leafNode->lexeme, 
                            id_arr->ele.data.arr.type, ret->ele.data.mod.inputList[i].data.id.lexeme,
                            ret->ele.data.mod.inputList[i].data.arr.type);
                            pushSemanticError(err);
                            // return NULL;
                        }
                        else if(isError==2)
                        {
                            //bounds mismatch ERROR
                            sprintf(err,"Line %d: Bounds of actual parameter %s (Array) do not match the bounds of formal parameter %s (Array).", 
                            trav->node->ele.leafNode->lineNum, 
                            trav->node->ele.leafNode->lexeme, 
                            ret->ele.data.mod.inputList[i].data.arr.lexeme);
                            pushSemanticError(err);
                            // return NULL;
                        }
                        
                    }
                }
                else    //tags don't match, one is array and other is ID
                {
                    //Type mismatch, ERROR
                    char *actual,*formal ;
                    if(ret->ele.data.mod.inputList[i].tag == Identifier)
                        actual = ret->ele.data.mod.inputList[i].data.id.type;
                    else
                        actual = ret->ele.data.mod.inputList[i].data.arr.type;
                    
                    if(id_arr->ele.tag == Identifier)
                        formal = id_arr->ele.data.id.type;
                    else
                        formal = id_arr->ele.data.arr.type;    
                    
                    if(ret->ele.data.mod.inputList[i].tag == Array)
                    {
                        sprintf(err,"Line %d: Type of actual parameter %s (%s) does not match the type of formal parameter %s (Array, %s).",
                        trav->node->ele.leafNode->lineNum, trav->node->ele.leafNode->lexeme, formal,
                        ret->ele.data.mod.inputList[i].data.id.lexeme, actual);
                    }
                    else
                    {
                        sprintf(err,"Line %d: Type of actual parameter %s (Array, %s) does not match the type of formal parameter %s (%s).",
                        trav->node->ele.leafNode->lineNum, trav->node->ele.leafNode->lexeme, formal,
                        ret->ele.data.mod.inputList[i].data.id.lexeme, actual);   
                    }  
                    pushSemanticError(err);
                    // return NULL;
                }
                trav = trav->sibling;
            }
        }

        /*Now compare the ID_list_node (return) with the output_list of the function*/
        int outputsize = listCount(currentNode->child->child);
        if(outputsize != ret->ele.data.mod.outputcount)
        {
            //No of input parameters mismatch, ERROR
            sprintf(err,"Line %d: %s module call has output parameters mismatch (Number of parameters are different).", 
            currentNode->child->sibling->sibling->node->ele.internalNode->lineNumStart, ret->ele.data.mod.lexeme);
            pushSemanticError(err);
            // if(err!=NULL) free(err);
            return NULL;
        }

        trav = currentNode->child->child;
        for(int i=0; i<outputsize; i++)
        {
            //its an ID
            symbolTableNode *id_arr = searchScope(tbStack, trav);

            //trav isn't declared
            if(id_arr == NULL)
            {
                //Not declared, ERROR
                continue;
                // return NULL;
            }
            //tags match
            if(ret->ele.data.mod.outputList[i].tag == id_arr->ele.tag)
            {
                // BOTH ARE IDs //
                if(id_arr->ele.tag == Identifier)
                {
                    if(strcmp(id_arr->ele.data.id.type, ret->ele.data.mod.outputList[i].data.id.type))
                    {
                        //Type mismatch, ERROR
                        sprintf(err,"Line %d: Type of receiving parameter %s (%s) for module call does not match the return type of %s (%s).", 
                        trav->node->ele.leafNode->lineNum, 
                        trav->node->ele.leafNode->lexeme, 
                        id_arr->ele.data.id.type, 
                        ret->ele.data.mod.outputList[i].data.id.lexeme, 
                        ret->ele.data.mod.outputList[i].data.id.type);
                        pushSemanticError(err);
                        id_arr->ele.data.id.isAssigned = 1;
                        // return NULL;
                    }

                    // Checking for index variable being assigned
                    if(id_arr->ele.data.id.isIndex == 1)
                    {
                        // ERROR
                        sprintf(err,"Line %d: %s variable can not be modified during FOR loop (index variable).", 
                        trav->node->ele.leafNode->lineNum, trav->node->ele.leafNode->lexeme); 
                        pushSemanticError(err);
                        // return NULL;
                    }
                }

                // BOTH ARE ARRAYS //
                else
                {
                    //Error, as you can't return an array variable from the function
                    sprintf(err,"Line %d: %s variable (Array) can't be returned from a function.", 
                    trav->node->ele.leafNode->lineNum, trav->node->ele.leafNode->lexeme); 
                    pushSemanticError(err);
                    // return NULL;
                }
            }
            else    //tags don't match, one is array and other is ID
            {
                //Type mismatch, ERROR
                if(id_arr->ele.tag == Identifier && ret->ele.data.mod.outputList[i].tag == Array)
                {
                    sprintf(err,"Line %d: Type of receiving parameter %s (%s) does not match the return type of %s (%s, Array).", 
                    trav->node->ele.leafNode->lineNum, 
                    trav->node->ele.leafNode->lexeme, 
                    id_arr->ele.data.id.type,
                    ret->ele.data.mod.outputList[i].data.arr.lexeme, 
                    ret->ele.data.mod.outputList[i].data.arr.type);
                    pushSemanticError(err);
                }
                else if(id_arr->ele.tag == Array && ret->ele.data.mod.outputList[i].tag == Identifier)
                {
                    sprintf(err,"Line %d: Type of receiving parameter %s (%s, Array) does not match the return type of %s (%s).", 
                    trav->node->ele.leafNode->lineNum, 
                    trav->node->ele.leafNode->lexeme, 
                    id_arr->ele.data.arr.type, 
                    ret->ele.data.mod.outputList[i].data.id.lexeme,
                    ret->ele.data.mod.outputList[i].data.arr.type);
                    pushSemanticError(err);
                }
                // return NULL;
            }
            trav = trav->sibling;
        }
        // if(err!=NULL) free(err);
        return NULL;
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "FOR"))
    {
        /*  3 Children
                1) ID_node
                2) Range_node
                3) stmts_list
        */
        /*New scope starts, push the corresponding symbol table on top of the stack*/
        tableStackEle *temp = sympop(tbStack);
        symbolTableNode *id = searchScope(tbStack, currentNode->child);//sym_hash_find(currentNode->child->node->ele.leafNode->lexeme, &(tbStack->top->ele->hashtb),0,NULL);
        sympush(tbStack,temp);
        
        char * err = (char *)malloc(sizeof(char)*200);
        memset(err, '\0', sizeof(char)*200);
        
        int isNull = 0;
        if(id == NULL)
        {
            isNull = 1;
           //Index variable not declared, ERROR
        //    free(sympop(tbStack));
        //    return NULL; 
        }
        
        if(isNull == 0 && id->ele.tag == Array)
        {
            //Can't be an array variable, ERROR
            sprintf(err,"Line %d: %s variable is of Array type (Index of FOR loop can't be of Array type).", 
            currentNode->child->node->ele.leafNode->lineNum, 
            currentNode->child->node->ele.leafNode->lexeme);
            pushSemanticError(err);
            // free(sympop(tbStack));
            // return NULL;
        }
        else if(isNull == 0  && (!strcmp(id->ele.data.id.type,"REAL") || !strcmp(id->ele.data.id.type,"BOOLEAN")))
        {
            //Type invalid, ERROR
            sprintf(err,"Line %d: %s variable is of invalid type (%s).", 
            currentNode->child->node->ele.leafNode->lineNum, 
            currentNode->child->node->ele.leafNode->lexeme,
            id->ele.data.id.type);
            pushSemanticError(err);
            // free(sympop(tbStack));
            // return NULL;
        }
        
        if(isNull == 0)
            id->ele.data.id.isIndex = 1;    //Mark this variable as index variable

        symbolTable *currentSTNode = tbStack->top->ele->child;
        tableStackEle *newTable = NULL;
        
        astNode *trav = currentNode->child->sibling->sibling;
        while(trav != NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") 
            || !strcmp(trav->node->ele.internalNode->label, "SWITCH") 
            || !strcmp(trav->node->ele.internalNode->label, "WHILE"))
            {
                // newTable = sympop(tbStack);
                newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
                newTable->ele = currentSTNode;
                newTable->next = NULL;
                sympush(tbStack,newTable);
                currentSTNode = currentSTNode->sibling;     
            }
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
        
        sympop(tbStack);
        // free(sympop(tbStack));
        // free(newTable);
        // if(err!=NULL) free(err);
        if(isNull == 0)
            id->ele.data.id.isIndex = 0; //isIndex restored back. Now it can change!

        return NULL;
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "WHILE"))
    {
        /*  2 Children
                1) Epxr_node
                2) stmts_node
        */
       
        //make a function to take a astNode and that marks all its leaves to be "while" variables
        symbolTable *currentSTNode = tbStack->top->ele->child;
        tableStackEle *newTable = NULL;

        tableStackEle *temp = sympop(tbStack);
        type *exprType = typeChecker(currentNode->child, tbStack);
        sympush(tbStack,temp);

        char * err = (char *)malloc(sizeof(char)*200);
        memset(err, '\0', sizeof(char)*200);
        int isNull = 0;
        
        if(exprType == NULL)
        {
            // free(err);
            // free(sympop(tbStack));
            // return NULL;
            isNull = 1;
        }
        if(isNull == 0 && exprType->tag == ArrayType)
        {
            if(!strcmp(currentNode->child->node->ele.internalNode->label, "ID_ARR"))
            {
                sprintf(err,"Line %d: Variable %s in while condition is of Array type (It should be Boolean).", 
                currentNode->child->child->node->ele.leafNode->lineNum,
                currentNode->child->child->node->ele.leafNode->lexeme);
            }
            // else
            // {
            //     sprintf(err,"Line %d: RHS type can't be Array.", 
            //     currentNode->child->node->ele.internalNode->lineNumStart);
            // }
            pushSemanticError(err);
            
            // free(exprType);
            // free(sympop(tbStack));
            // return NULL;
        }
        else if(isNull == 0 && strcmp(exprType->tp.type,"BOOLEAN"))
        {
            // ERROR - Expr in While is either has error or of wrong type.
            if(currentNode->child->node->tag == Leaf)
            {
                sprintf(err,"Line %d: Expression in while condition must be boolean (Here, it is %s).", 
                currentNode->child->node->ele.leafNode->lineNum, exprType->tp.type);
            }
            else
            {
                sprintf(err,"Line %d: Expression in while condition must be boolean (Here, it is %s).", 
                currentNode->child->node->ele.internalNode->lineNumStart, exprType->tp.type);
            }
            // free(exprType);
            pushSemanticError(err);
            // free(sympop(tbStack));
            // return NULL;
        }
        //Mark all vars in the Expr_node as unassigned
        int size = 100, index = 0, isConst = 1;
        // checkConstant(currentNode->child, isConst);
        int *prevValues = (int *)malloc(sizeof(int)*size);
        traverseAndMark(currentNode->child, tbStack, prevValues, &size, &index, &isConst);

        if(isConst == 1)
        {
            if(currentNode->child->node->tag == Leaf)
            {
                sprintf(err, "Line %d: The while condition has no varible. Possibly infinite loop.",
                currentNode->child->node->ele.leafNode->lineNum);
            }
            else
            {
                sprintf(err, "Line %d: The while condition has no varible. Possibly infinite loop.",
                currentNode->child->node->ele.internalNode->lineNumStart);
            } 
            pushSemanticError(err);
            // if(free != NULL)    free(err);
        }

        astNode *trav = currentNode->child->sibling;
        while(trav != NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") 
            || !strcmp(trav->node->ele.internalNode->label, "SWITCH") 
            || !strcmp(trav->node->ele.internalNode->label, "WHILE"))
            {
                // newTable = sympop(tbStack);
                newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
                newTable->ele = currentSTNode;
                newTable->next = NULL;
                sympush(tbStack,newTable);
                currentSTNode = currentSTNode->sibling;
            }
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
        

        //Check whether some var in Expr_node is marked as assigned
        int error = 1;

        //If some var in Expr_node is assigned, error = 0
        index = 0;

        if(isConst == 1)
        {
            error = 0;
        }
        else
        {
            checkAssignment(currentNode->child, tbStack, &error, prevValues, &index);
            // if(prevValues!=NULL) free(prevValues);
        }

        //No variable in Expr_node has been assigned in the body of while
        if(error == 1)
        {
            //Error
            if(currentNode->child->node->tag == Leaf)
            {
                sprintf(err,"Line %d: No variable in while's condition changes inside the loop.", 
                currentNode->child->node->ele.leafNode->lineNum);
            }
            else
            {
                sprintf(err,"Line %d: No variable in while's condition changes inside the loop.", 
                currentNode->child->node->ele.internalNode->lineNumStart);
            }
            // if(isNull == 0)
                // free(exprType);
            pushSemanticError(err);
            // if(err!=NULL) free(err);
            sympop(tbStack);
            // free(sympop(tbStack));
            return NULL;
        }

        // if(err!=NULL) free(err);
        // if(isNull == 0)
            // free(exprType);
        sympop(tbStack);
        // free(sympop(tbStack));
        // free(newTable);
        return NULL;
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "SWITCH"))
    {
        /*  3 Children
                1) ID
                2) Case
                3) Default
        */
        tableStackEle *temp = sympop(tbStack);
        symbolTableNode *id = searchScope(tbStack, currentNode->child); //sym_hash_find(currentNode->child->node->ele.leafNode->lexeme, &(tbStack->top->ele->hashtb),0,NULL);
        sympush(tbStack,temp);

        char * err = (char *)malloc(sizeof(char)*200);
        memset(err, '\0', sizeof(char)*200);
        int isNull = 0;
        
        if(id == NULL)
        {
           //Index variable not declared, ERROR
           //searchscope has already inserted the error of "not found" variable
        //    free(sympop(tbStack));
        //    return NULL;
            isNull = 1;
        }
        
        if(isNull == 0 && id->ele.tag == Array)
        {
            //Can't be an array variable, ERROR
            sprintf(err,"Line %d: %s variable used in the switch construct (Switch varible can't be an array).", 
            id->lineNum,
            id->ele.data.arr.lexeme);
            pushSemanticError(err);
            // free(sympop(tbStack));
            // return NULL;
        }
        else if(isNull == 0 && !strcmp(id->ele.data.id.type,"REAL"))
        {
            //Type invalid, ERROR
            sprintf(err,"Line %d: %s variable is of invalid type (REAL).", 
            currentNode->child->node->ele.leafNode->lineNum, 
            currentNode->child->node->ele.leafNode->lexeme);
            pushSemanticError(err);
            // free(sympop(tbStack));
            // return NULL;
        }
        
        //trav is the head of the CASE statements        
        astNode * trav = currentNode->child->sibling;
        
        if(isNull == 0 && !strcmp(id->ele.data.id.type,"INTEGER"))
        {
            astNode* findDefault = currentNode->child;
            while(findDefault->sibling!=NULL)
            {
                findDefault = findDefault->sibling;
            }
            if(strcmp(findDefault->node->ele.internalNode->label,"DEFAULT"))
            {
                //error DEFAULT IS MISSING
                sprintf(err, "Line %d: Default is missing though switch variable %s is of type %s.",
                currentNode->child->node->ele.leafNode->lineNum, 
                currentNode->child->node->ele.leafNode->lexeme, 
                id->ele.data.id.type);
                pushSemanticError(err);
                findDefault = findDefault->sibling;
                // free(sympop(tbStack));
                // return NULL;
            }
            
            //trav is the firt case (head)
            while(trav != findDefault)
            {
            
                if(!strcmp(trav->child->node->ele.leafNode->type, "RNUM") 
                || !strcmp(trav->child->node->ele.leafNode->type, "TRUE") 
                || !strcmp(trav->child->node->ele.leafNode->type, "FALSE"))
                {   
                    if(!strcmp(trav->child->node->ele.leafNode->type, "TRUE") || !strcmp(trav->child->node->ele.leafNode->type, "FALSE"))
                    {
                        sprintf(err,"Line %d: Value variable (%s) in CASE cannot be of %s type, should be of NUM/ID(Integer) type.", 
                        trav->child->node->ele.leafNode->lineNum, 
                        trav->child->node->ele.leafNode->lexeme, 
                        "Boolean");
                        pushSemanticError(err);
                    }
                    else if(!strcmp(trav->child->node->ele.leafNode->type, "RNUM"))
                    {
                        sprintf(err,"Line %d: Value variable (%s) in CASE cannot be of %s type, should be of NUM/ID(Integer) type.", 
                        trav->child->node->ele.leafNode->lineNum, 
                        trav->child->node->ele.leafNode->lexeme, 
                        "Real");
                        pushSemanticError(err);
                    }
                    // free(sympop(tbStack));
                    // pushSemanticError(err);
                    // return NULL;
                }
                
                //Now value is either ID or NUM
                if(!strcmp(trav->child->node->ele.leafNode->type, "ID"))
                {
                    type *valType = typeChecker(trav->child, tbStack);

                    if(valType == NULL)
                    {
                        //ID isn't declared, ERROR
                        //searchscope must have inserted the error
                        // free(sympop(tbStack));
                        // return NULL;
                    }
                    else if(valType->tag == ArrayType)
                    {
                        //ID can't be a array var, ERROR
                        sprintf(err, "Line %d: Value variable (%s) in CASE is of Array type.",
                        trav->child->node->ele.leafNode->lineNum, 
                        trav->child->node->ele.leafNode->lexeme);
                        pushSemanticError(err);
                        // free(valType);
                        // free(sympop(tbStack));
                        // return NULL;
                    }
                    else if(strcmp(valType->tp.type, "INTEGER"))
                    {
                        //ID can't be of any type except integer, ERROR
                        sprintf(err, "Line %d: Value variable (%s) in CASE is of type %s (Expected Integer).",
                        trav->child->node->ele.leafNode->lineNum, 
                        trav->child->node->ele.leafNode->lexeme,
                        valType->tp.type);
                        pushSemanticError(err);
                        // free(valType);
                        // free(sympop(tbStack));
                        // return NULL;
                    }
                    
                }  
                trav = trav->sibling;
            }
        }
        trav = currentNode->child->sibling;

        
        if(isNull == 0 && !strcmp(id->ele.data.id.type,"BOOLEAN"))
        {
            // printf("Reached Here");
            // getchar();
            
            astNode* findDefault = currentNode->child;
            while(findDefault->sibling!=NULL)
            {
                findDefault = findDefault->sibling;
            }
            if(!strcmp(findDefault->node->ele.internalNode->label,"DEFAULT"))
            {
                //error DEFAULT IS PRESENT
                sprintf(err, "Line %d: Default is present though switch variable %s is of type %s.",
                currentNode->child->node->ele.leafNode->lineNum, 
                currentNode->child->node->ele.leafNode->lexeme, 
                id->ele.data.id.type);
                pushSemanticError(err);
                // free(sympop(tbStack));
                // return NULL;
            }
            else
            {
                findDefault = findDefault->sibling;
            }
            //switch with a BOOL type does not require a Default (should not be there)
            while(trav != NULL)
            {
                //If default exists, skip it. No need to perform semantic checks inside default
                if(findDefault != NULL && trav == findDefault)
                {
                    trav = trav->sibling;
                    continue;
                }
                if(!strcmp(trav->child->node->ele.leafNode->type, "RNUM"))
                {
                    sprintf(err, "Line %d: Value (%s) in CASE is of type %s (Expected Boolean).",
                    trav->child->node->ele.leafNode->lineNum, 
                    trav->child->node->ele.leafNode->lexeme, "REAL");
                    pushSemanticError(err);
                    // free(sympop(tbStack));
                    // return NULL;
                }
                if(!strcmp(trav->child->node->ele.leafNode->type, "NUM"))
                {
                    //Value can't be a real constant, ERROR
                    sprintf(err, "Line %d: Value (%s) in CASE is of type %s (Expected Boolean).",
                    trav->child->node->ele.leafNode->lineNum, 
                    trav->child->node->ele.leafNode->lexeme, "INTEGER");
                    pushSemanticError(err);
                    // free(sympop(tbStack));
                    // return NULL;
                }
                if(!strcmp(trav->child->node->ele.leafNode->type, "ID"))
                {
                    type *valType = typeChecker(trav->child, tbStack);
                    if(valType == NULL)
                    {
                        //ID isn't declared, ERROR
                        // free(sympop(tbStack));
                        // return NULL;
                    }
                    else if(valType->tag == ArrayType)
                    {
                        //ID can't be a array var, ERROR
                        sprintf(err, "Line %d: Value variable (%s) in CASE is of Array type.",
                        trav->child->node->ele.leafNode->lineNum, 
                        trav->child->node->ele.leafNode->lexeme);
                        pushSemanticError(err);
                        // free(valType);
                        // free(sympop(tbStack));
                        // return NULL;
                    }
                    else if(strcmp(valType->tp.type, "BOOLEAN"))
                    {
                        //ID can't be of any type except integer, ERROR
                        sprintf(err, "Line %d: Value variable (%s) in CASE is of type %s (Expected Boolean).",
                        trav->child->node->ele.leafNode->lineNum, 
                        trav->child->node->ele.leafNode->lexeme,
                        valType->tp.type);
                        pushSemanticError(err);
                        // free(valType);
                        // free(sympop(tbStack));
                        // return NULL;
                    }
                }
                trav = trav->sibling;
            }
        }   

        //assigntrav as ID_node 
        trav = currentNode->child->sibling;
        while(trav != NULL)
        {
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
        // if(err!=NULL) free(err);
        sympop(tbStack);
        // free(sympop(tbStack));
        return NULL;   
    }
    else if (!strcmp(currentNode->node->ele.internalNode->label, "CASE"))
    {
        /* 
            Structure of CASE
                Child 1 is Value
                Child 2 is Statements
        */
        
        
        symbolTable *currentSTNode = tbStack->top->ele->child;
        tableStackEle *newTable = NULL;
        
        astNode *trav = currentNode->child->sibling;
        while(trav != NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") 
            || !strcmp(trav->node->ele.internalNode->label, "SWITCH") 
            || !strcmp(trav->node->ele.internalNode->label, "WHILE"))
            {
                // newTable = sympop(tbStack);
                newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
                newTable->ele = currentSTNode;
                newTable->next = NULL;
                sympush(tbStack,newTable);
                currentSTNode = currentSTNode->sibling;
            }
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
        
        // free(sympop(tbStack));
        // free(newTable);
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label,"ID_ARR"))
    {
        // 2 Children
        //      1) ID
        //      2) WhichID

        char * err = (char *)malloc(sizeof(char)*200);
        memset(err, '\0', sizeof(char)*200);
    
        symbolTableNode * myNode = searchScope(tbStack, currentNode->child);
        
        if(myNode==NULL)
        {
            //ID not declared ERROR
            //searchscope  has inserted the error in the list
            // if(err!=NULL) free(err);
            return NULL;
        }

        type *answer = (type *) malloc(sizeof(type));

        // There is no index mentioned and the ID is an array
        if(currentNode->child->sibling == NULL && myNode->ele.tag == Array)
        {
            answer->tag = ArrayType;
            answer->tp.arr.basicType = myNode->ele.data.arr.type;
            answer->tp.arr.lowerBound = myNode->ele.data.arr.lowerIndex;
            answer->tp.arr.upperBound = myNode->ele.data.arr.upperIndex;
        }
        
        // There is no index mentioned and the ID is an identifier
        else if(currentNode->child->sibling == NULL && myNode->ele.tag == Identifier)
        {
            answer->tag = IdentifierType;
            answer->tp.type = myNode->ele.data.id.type; 
        }

        // The index is mentioned for sure now

        //index is mentioned and ID is an Identifier
        else if(currentNode->child->sibling !=NULL && myNode->ele.tag == Identifier)
        {
            // ERROR 
            // Identifier with an index is not valid
            sprintf(err,"Line %d: %s variable (%s) can not be referenced like an Array.", 
            currentNode->child->node->ele.leafNode->lineNum, 
            currentNode->child->node->ele.leafNode->lexeme, 
            myNode->ele.data.id.type);
            pushSemanticError(err);
            // if(err!=NULL) free(err);
            return NULL;
        }

        //index is mentioned and the ID is an Array
        else if(currentNode->child->sibling !=NULL && myNode->ele.tag==Array)
        {
            answer->tag = IdentifierType;
            answer->tp.type = myNode->ele.data.arr.type;
            //check boundaries and types of the whichid construct
            if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"ID"))
            {
                //the index is an ID
                symbolTableNode * st = searchScope(tbStack, currentNode->child->sibling);

                if(st == NULL)
                {
                    //ID isn't declared, ERROR
                    //searchscope must have inserted the error
                    // if(err!=NULL) free(err);
                    return NULL;
                }
                if(st->ele.tag != Identifier)
                {
                    //Index node is Array, ERROR
                    sprintf(err,"Line %d: %s variable (%s) can not be referenced like an Array.", 
                    currentNode->child->sibling->node->ele.leafNode->lineNum, 
                    currentNode->child->sibling->node->ele.leafNode->lexeme, 
                    st->ele.data.id.type);
                    pushSemanticError(err);
                    // if(err!=NULL) free(err);
                    return NULL;
                }
                
                if(strcmp(st->ele.data.id.type, "INTEGER"))
                {
                    //Type can't be anything except INTEGER, ERROR
                    sprintf(err,"Line %d: Index variable (%s) is of %s type (Index variable of an array must be of Integer type).", 
                    currentNode->child->sibling->node->ele.leafNode->lineNum, 
                    currentNode->child->sibling->node->ele.leafNode->lexeme, 
                    st->ele.data.id.type);
                    pushSemanticError(err);
                    // if(err!=NULL) free(err);
                    return NULL;
                }

                //Index node is an identifier of type Integer
            }

            else if (!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"NUM"))
            {
                //the index is a NUM
                astNode *num = currentNode->child->sibling;
                //boundary checks
                if(myNode->ele.data.arr.isDynamic == 0)
                {
                    if( ((*(int*)(currentNode->child->sibling->node->ele.leafNode->value)) < (*(int*)(myNode->ele.data.arr.lowerIndex->value)) )
                    || ((*(int*)(currentNode->child->sibling->node->ele.leafNode->value)) > (*(int*)(myNode->ele.data.arr.upperIndex->value)) ) )
                    {
                        sprintf(err,"Line %d: Index (%s) is out of bounds for this Array - %s (%d - %d).",
                        num->node->ele.leafNode->lineNum, num->node->ele.leafNode->lexeme, 
                        currentNode->child->node->ele.leafNode->lexeme, 
                        *(int*)(myNode->ele.data.arr.lowerIndex->value), 
                        *(int*)(myNode->ele.data.arr.upperIndex->value));
                        pushSemanticError(err);
                        // if(err!=NULL) free(err);
                        return NULL;
                    }
                }
            }                
        }
        // if(err!=NULL) free(err);
        return answer;
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
        char * err = (char *)malloc(sizeof(char)*200);
        memset(err, '\0', sizeof(char)*200);
        
        //If its a binary expression
        if(currentNode->child->sibling != NULL)
        {
            type *leftType = typeChecker(currentNode->child, tbStack);
            type *rightType = typeChecker(currentNode->child->sibling, tbStack);
			//There was some problem in type checking of the operands
            if(leftType == NULL || rightType == NULL)
                return NULL;
        
            int isArr = 0;
            //Note: This case will arise only at the second-last level of the expression
            if(leftType->tag == ArrayType)
            {
                isArr = 1;
                if(!strcmp(currentNode->child->node->ele.internalNode->label, "ID_ARR"))
                {
                    sprintf(err, "Line %d: Left operand (%s) of operator (%s) is of Array type.",
                    currentNode->node->ele.internalNode->lineNumStart,
                    currentNode->child->child->node->ele.leafNode->lexeme,
                    currentNode->node->ele.internalNode->label);
                    pushSemanticError(err);
                }
                // else
                // {
                //     sprintf(err, "Line %d: Left operand (%s) of operator (%s) is of Array type.",
                //     currentNode->node->ele.internalNode->lineNumStart,
                //     currentNode->child->node->ele.internalNode->label,
                //     currentNode->node->ele.internalNode->label);
                // }
            }
            if(rightType->tag == ArrayType)
            {
                isArr = 1;
                if(!strcmp(currentNode->child->sibling->node->ele.internalNode->label, "ID_ARR"))
                {
                    sprintf(err, "Line %d: Right operand (%s) of operator (%s) is of Array type.",
                    currentNode->node->ele.internalNode->lineNumStart,
                    currentNode->child->sibling->child->node->ele.leafNode->lexeme,
                    currentNode->node->ele.internalNode->label);
                    pushSemanticError(err);
                }
                // else
                // {
                //     sprintf(err, "Line %d: Right operand (%s) of operator (%s) is of Array type.",
                //     currentNode->node->ele.internalNode->lineNumStart,
                //     currentNode->child->sibling->node->ele.internalNode->label,
                //     currentNode->node->ele.internalNode->label);
                // }
                // if(leftType!=NULL)free(leftType);
                // if(rightType!=NULL)free(rightType);
                // if(err!=NULL) free(err);
                // return NULL;
            }
            
            if(isArr == 1)
            {
                // if(leftType!=NULL)free(leftType);
                // if(rightType!=NULL)free(rightType);
                // if(err!=NULL) free(err);
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
                        if(!strcmp(currentNode->child->node->ele.internalNode->label, "ID_ARR"))
                        {
                            if(!strcmp(currentNode->child->sibling->node->ele.internalNode->label, "ID_ARR"))
                            {
                                sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of Boolean type.",
                                currentNode->node->ele.internalNode->lineNumStart,
                                currentNode->child->child->node->ele.leafNode->lexeme,
                                currentNode->child->sibling->child->node->ele.leafNode->lexeme,
                                currentNode->node->ele.internalNode->label);
                            }
                            else
                            {
                                sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of Boolean type.",
                                currentNode->node->ele.internalNode->lineNumStart,
                                currentNode->child->child->node->ele.leafNode->lexeme,
                                currentNode->child->sibling->node->ele.internalNode->label,
                                currentNode->node->ele.internalNode->label);
                            } 
                        }
                        else
                        {
                            if(!strcmp(currentNode->child->sibling->node->ele.internalNode->label, "ID_ARR"))
                            {
                                sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of Boolean type.",
                                currentNode->node->ele.internalNode->lineNumStart,
                                currentNode->child->node->ele.internalNode->label,
                                currentNode->child->sibling->child->node->ele.leafNode->lexeme,
                                currentNode->node->ele.internalNode->label);
                            }
                            // else
                            // {
                            //     sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of Boolean type.",
                            //     currentNode->node->ele.internalNode->lineNumStart,
                            //     currentNode->child->node->ele.internalNode->label,
                            //     currentNode->child->sibling->node->ele.internalNode->label,
                            //     currentNode->node->ele.internalNode->label);
                            // }
                        }
                        pushSemanticError(err);
                        // if(retType!=NULL)free(retType);
                        // if(leftType!=NULL)free(leftType);
                        // if(rightType!=NULL)free(rightType);
                        // if(err!=NULL) free(err);
                        return  NULL;
                    } 

                    // if(leftType!=NULL)free(leftType);
                    // if(rightType!=NULL)free(rightType);
                    // if(err!=NULL) free(err);        
                    return retType;         
                }
                else
                {
                    //Type mismatch among operands, ERROR

                    sprintf(err,"Line %d: LHS (%s) and RHS (%s) of %s operator type mismatch.", 
                    currentNode->node->ele.internalNode->lineNumStart, 
                    leftType->tp.type,
                    rightType->tp.type,
                    currentNode->node->ele.internalNode->label);
                    pushSemanticError(err);

                    // if(retType!=NULL)free(retType);
                    // if(leftType!=NULL)free(leftType);
                    // if(rightType!=NULL)free(rightType);
                    // if(err!=NULL) free(err);
                    return NULL;
                }
                // if(err!=NULL) free(err);
            }
            
            else if(!strcmp(currentNode->node->ele.internalNode->label, "LT") || 
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
                        if(!strcmp(currentNode->child->node->ele.internalNode->label, "ID_ARR") 
                        && !strcmp(currentNode->child->sibling->node->ele.internalNode->label, "ID_ARR"))
                        {
                            sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of Boolean type.",
                            currentNode->node->ele.internalNode->lineNumStart,
                            currentNode->child->child->node->ele.leafNode->lexeme,
                            currentNode->child->sibling->child->node->ele.leafNode->lexeme,
                            currentNode->node->ele.internalNode->label);
                        }
                        else if(strcmp(currentNode->child->node->ele.internalNode->label, "ID_ARR") 
                        && !strcmp(currentNode->child->sibling->node->ele.internalNode->label, "ID_ARR"))
                        {
                            sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of Boolean type.",
                            currentNode->node->ele.internalNode->lineNumStart,
                            currentNode->child->node->ele.internalNode->label,
                            currentNode->child->sibling->child->node->ele.leafNode->lexeme,
                            currentNode->node->ele.internalNode->label);
                        }
						else if(!strcmp(currentNode->child->node->ele.internalNode->label, "ID_ARR") 
                        && strcmp(currentNode->child->sibling->node->ele.internalNode->label, "ID_ARR"))
                        {
                            sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of Boolean type.",
                            currentNode->node->ele.internalNode->lineNumStart,
                            currentNode->child->child->node->ele.leafNode->lexeme,
                            currentNode->child->sibling->node->ele.internalNode->label,
                            currentNode->node->ele.internalNode->label);
                        }
						// else
                        // {
                        //     sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of Boolean type.",
                        //     currentNode->node->ele.internalNode->lineNumStart,
                        //     currentNode->child->node->ele.internalNode->label,
                        //     currentNode->child->sibling->node->ele.internalNode->label,
                        //     currentNode->node->ele.internalNode->label);
                        // }
                        pushSemanticError(err);
                        // if(retType!=NULL)free(retType);
                        // if(leftType!=NULL)free(leftType);
                        // if(rightType!=NULL)free(rightType);
                        // if(err!=NULL) free(err);
                        return NULL;
                    }
                    
                    // if(leftType!=NULL)free(leftType);
                    // if(rightType!=NULL)free(rightType);
                    // if(err!=NULL) free(err);
                    return retType;         
                }
                else
                {
                    //Type mismatch among operands, ERROR

                    sprintf(err,"Line %d: LHS (%s) and RHS (%s) of %s operator type mismatch.", 
                    currentNode->node->ele.internalNode->lineNumStart, 
                    leftType->tp.type,
                    rightType->tp.type,
                    currentNode->node->ele.internalNode->label);
                    pushSemanticError(err);

                    // if(retType!=NULL)free(retType);
                    // if(leftType!=NULL)free(leftType);
                    // if(rightType!=NULL)free(rightType);
                    // if(err!=NULL) free(err);
                    return NULL;
                }
            }
            else if(!strcmp(currentNode->node->ele.internalNode->label, "AND") || 
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
                        //Type can't be anything except boolean, ERROR
                        if(!strcmp(currentNode->child->node->ele.internalNode->label, "ID_ARR"))
                        {
                            if(!strcmp(currentNode->child->sibling->node->ele.internalNode->label, "ID_ARR"))
                            {
                                sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of %s type (Expected Boolean).",
                                currentNode->node->ele.internalNode->lineNumStart,
                                currentNode->child->child->node->ele.leafNode->lexeme,
                                currentNode->child->sibling->child->node->ele.leafNode->lexeme,
                                currentNode->node->ele.internalNode->label,
                                leftType->tp.type);
                            }
                            else
                            {
                                sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of %s type (Expected Boolean).",
                                currentNode->node->ele.internalNode->lineNumStart,
                                currentNode->child->child->node->ele.leafNode->lexeme,
                                currentNode->child->sibling->node->ele.internalNode->label,
                                currentNode->node->ele.internalNode->label,
                                leftType->tp.type);
                            }
                        }
                        else
                        {
                            if(!strcmp(currentNode->child->sibling->node->ele.internalNode->label, "ID_ARR"))
                            {
                                sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of %s type (Expected Boolean).",
                                currentNode->node->ele.internalNode->lineNumStart,
                                currentNode->child->node->ele.internalNode->label,
                                currentNode->child->sibling->child->node->ele.leafNode->lexeme,
                                currentNode->node->ele.internalNode->label,
                                leftType->tp.type);
                            }
                            // else
                            // {
                            //     sprintf(err, "Line %d: Operands (%s, %s) of operator (%s) are of %s type (Expected Boolean).",
                            //     currentNode->node->ele.internalNode->lineNumStart,
                            //     currentNode->child->node->ele.internalNode->label,
                            //     currentNode->child->sibling->node->ele.internalNode->label,
                            //     currentNode->node->ele.internalNode->label,
                            //     leftType->tp.type);
                            // }
                        }
                        pushSemanticError(err);
                        // if(retType!=NULL)free(retType);
                        // if(leftType!=NULL)free(leftType);
                        // if(rightType!=NULL)free(rightType);
                        // if(err!=NULL) free(err);
                        return  NULL;
                    }   
                    // if(leftType!=NULL)free(leftType);
                    // if(rightType!=NULL)free(rightType);
                    // if(err!=NULL) free(err);
                    return retType;         
                }
                else
                {
                    //Type mismatch among operands, ERROR

                    sprintf(err,"Line %d: LHS (%s) and RHS (%s) of %s operator type mismatch.", 
                    currentNode->node->ele.internalNode->lineNumStart, 
                    leftType->tp.type,
                    rightType->tp.type,
                    currentNode->node->ele.internalNode->label);
                    pushSemanticError(err);
                    // if(retType!=NULL)free(retType);
                    // if(leftType!=NULL)free(leftType);
                    // if(rightType!=NULL)free(rightType);
                    // if(err!=NULL) free(err);
                    return NULL;
                }
            }
        }
        //If its a unary expression
        else
        {
            type *childType = typeChecker(currentNode->child, tbStack);
            if(childType == NULL)
                return NULL;
            if(childType->tag == ArrayType)
            {
                //Operand can't be of an array type, ERROR
                if(!strcmp(currentNode->child->node->ele.internalNode->label, "ID_ARR"))
                {
                    sprintf(err, "Line %d: Operand (%s) of unary operator (%s) is of Array type.",
                    currentNode->node->ele.internalNode->lineNumStart,
                    currentNode->child->child->node->ele.leafNode->lexeme,
                    currentNode->node->ele.internalNode->label);
                }
                // else
                // {
                //     sprintf(err, "Line %d: Operand (%s) of unary operator (%s) is of Array type.",
                //     currentNode->node->ele.internalNode->lineNumStart,
                //     currentNode->child->node->ele.internalNode->label,
                //     currentNode->node->ele.internalNode->label);
                // }
                pushSemanticError(err);
                // if(childType!=NULL) free(childType);
                // if(err!=NULL) free(err);
                return NULL;
            }
            type *retType = (type *)malloc(sizeof(type));
            if(!strcmp(childType->tp.type, "INTEGER"))
            {
                retType->tag = IdentifierType;
                retType->tp.type = "INTEGER";
            }
            else if(!strcmp(childType->tp.type, "REAL"))
            {
                retType->tag = IdentifierType;
                retType->tp.type = "REAL";
            }
            else
            {
                //Type of operand is wrong, ERROR
                if(!strcmp(currentNode->child->node->ele.internalNode->label, "ID_ARR"))
                {
                    sprintf(err, "Line %d: Operand (%s) of unary operator (%s) are of Boolean type.",
                    currentNode->node->ele.internalNode->lineNumStart,
                    currentNode->child->child->node->ele.leafNode->lexeme,
                    currentNode->node->ele.leafNode->lexeme);
                }
                // else
                // {
                //     sprintf(err, "Line %d: Operands (%s) of unary operator (%s) are of Boolean type.",
                //     currentNode->node->ele.internalNode->lineNumStart,
                //     currentNode->child->node->ele.internalNode->label,
                //     currentNode->node->ele.internalNode->label);
                // }
                pushSemanticError(err);
                // if(retType!=NULL) free(retType);
                // if(childType!=NULL) free(childType);
                // if(err!=NULL) free(err);
                return NULL;
            }
            // if(childType!=NULL) free(childType);
            // if(err!=NULL) free(err);
            return retType;
        }
    }
    else
    {
        astNode *trav = currentNode->child;
        while(trav !=  NULL)
        {
            return typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
    }     
}

void traverseAndMark(astNode * root, tableStack * tbStack, int * prevValues, int *size, int *index, int *isConst)
{
    // if(root == NULL)
    // {
    //     return;
    // }

    if(*index == *size)
    {
        *size = (*size)*2;
        prevValues = (int *)realloc(prevValues, (*size)*sizeof(int));
    }

    if(root->node->tag == Leaf)
    {
        if(strcmp(root->node->ele.leafNode->type, "TRUE") && strcmp(root->node->ele.leafNode->type, "FALSE"))
        {
            *isConst = 0;
        }
        if(!strcmp(root->node->ele.leafNode->type, "ID"))
        {
            symbolTableNode *st = searchScope(tbStack, root);
            if(st == NULL)
            {
                //Not declared, ERROR
                //Push error and return
                //searchscope will have inserted error
                return;
            }
            if(st->ele.tag == Identifier)
            {
                st->ele.data.id.isAssigned = 0;
                prevValues[*index] = 1;
                *index = *index + 1;
            }
        }
        return;
    }
    
    astNode *trav = root->child;

    while(trav != NULL)
    {
        traverseAndMark(trav, tbStack, prevValues, size, index, isConst);
        trav = trav->sibling;
    }
}

void checkAssignment(astNode *root, tableStack *tbStack, int *error, int *prevValues, int *index)
{
    //root is the expresssion node of while's condition
    if(root->node->tag == Leaf)
    {
        if(!strcmp(root->node->ele.leafNode->type, "ID"))
        {
            symbolTableNode *st = searchScope(tbStack, root);

            // printf("Checking for %s: ", root->node->ele.leafNode->lexeme);
            // getchar();

            if(st == NULL)
            {
                /***** NOT NEEDED ******/
                //Not declared, ERROR
                //Push error and return
                //searchscope will have inserted the error
                
                return;
            }
            if(st->ele.tag == Identifier)
            {
                if(st->ele.data.id.isAssigned == 1)
                {
                    *error = 0;
                    // printf(" Changed \n");
                    // getchar();
                }    
                else
                {
                    st->ele.data.id.isAssigned = prevValues[*index];
                    // printf(" Not Changed \n");
                    // getchar();
                }    

                *index = *index + 1;
            }
        }
        return;
    }
    
    astNode *trav = root->child;

    while(trav != NULL)
    {
        checkAssignment(trav, tbStack, error, prevValues, index);
        trav = trav->sibling;
    }
}

// void checkConstant(astNode * root, int *isConstant)
// {
//     if(root == NULL)
//         return;

//     if(root->node->tag == Leaf)
//     {
//             if(strcmp(root->node->ele.leafNode->type, "TRUE") && strcmp(root->node->ele.leafNode->type, "FALSE"))
//             {
//                 *isConstant = 0;
//             }
//     }

//     astNode * trav = root->child;

//     while(trav != NULL)
//     {
//         checkConstant(trav, isConstant);
//         trav = trav->sibling;
//     }
// }