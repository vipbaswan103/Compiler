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

symbolTableNode *searchScope(tableStack *tbStack, astNode *key)
{
    //initialise the temporary stack
    tableStack *tempStack = (tableStack *)malloc(sizeof(tableStack));
    symbolTableNode *ret = NULL;
    tableStackEle *temp = NULL;
    
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
        free(tempStack);
        return NULL;
    }
    
    // You had found the variable in some scope
    else
    {
        //Check whether declaration is above or below the definition (using lineNumbers)
        //TODO check if line number is the same for usage and declaration in the same/different scope, considering
        //everything is written in the same line 
        if(ret->lineNum >= key->node->ele.leafNode->lineNum)
        {
            if(ret->lineNum == key->node->ele.leafNode->lineNum)
            {
                if(ret->aux != 1)   //Variable hasn't been decalared
                {
                    //ERROR
                    free(tempStack);
                    return NULL;
                }
            }
            else
            {
                //ERROR
                free(tempStack);
                return NULL;
            }
        }

        //Push everything in tempStack into the tbStack
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
        if(!strcmp(currentNode->node->ele.leafNode->type, "TRUE") || !strcmp(currentNode->node->ele.leafNode->type, "FALSE"))
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
            else if(idNode->ele.tag == Module)
            {
                free(typeLeaf);
                return NULL;
                //ERROR
            }
        }
        else
        {
            // ERROR : Invalid Leaf Node
            free(typeLeaf);
            return NULL;
        }
        return typeLeaf;
    }

    
    
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
        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStack));
        newTable->next = NULL;
        newTable->ele = symbolTableRoot;
        sympush(tbStack,newTable);
        
        // creating and pushing the moduledec table
        newTable= (tableStackEle *)malloc(sizeof(tableStack));
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
        free(newTable);

        //Pop the Program symbol table
        newTable = sympop(tbStack);
        free(newTable);
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULES1"))
    {
        astNode * trav = currentNode->child;
        tableStackEle *newTable = NULL;

        symbolTable *st = symbolTableRoot->child->sibling;

        //Iterate over all the modules and push their corresponding scope on the stack (and recursively call typechecker)
        while(trav != NULL)
        {
            newTable = (tableStackEle *)malloc(sizeof(tableStack));
            newTable->ele = st;
            newTable->next = NULL;
            sympush(tbStack, newTable);
            st = st->sibling;
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
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

        //Iterate over all the modules and push their corresponding scope on the stack (and recursively call typechecker)
        while(trav != NULL)
        {
            newTable = (tableStackEle *)malloc(sizeof(tableStack));
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
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULE"))
    {
        //only child of module is moduledef
        symbolTableNode *ret = sym_hash_find(tbStack->top->ele->symLexeme, &(symbolTableRoot->hashtb), 0, NULL);
        ret->aux = 1;
        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->ele = tbStack->top->ele->child;
        newTable->next = NULL;

        //Push the moduleDef's symbol table on the stack
        sympush(tbStack, newTable);
    
        //Call typeChecker on moduleDef
        typeChecker(currentNode->child->sibling->sibling->sibling, tbStack);

        //Pop this moduleDef's symbol table
        newTable = sympop(tbStack);
        free(newTable);
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "DRIVER"))
    {
        symbolTable *trav = symbolTableRoot->child->sibling;
        while(!strcmp(trav->symLexeme, "Driver"))
        {
            trav = trav->sibling;
        }

        //Push the driver's symbol table on the stack
        sympush(tbStack, trav);
        
        //Push the moduleDef's symbol table on the stack
        sympush(tbStack, trav->child);

        //Call type checker on driver's moduleDef
        typeChecker(currentNode->child, tbStack);

        tableStackEle *table = sympop(tbStack);
        free(table);
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
            //New scope starts for FOR, WHILE, SWITCH
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") || !strcmp(trav->node->ele.internalNode->label, "WHILE") || !strcmp(trav->node->ele.internalNode->label, "SWITCH"))
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
        free(newTable);
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "DECLARE"))
    {
        astNode *trav = currentNode->child->child;
        symbolTableNode *st = NULL;
        while(trav != NULL)
        {
            st = sym_hash_find(trav->node->ele.leafNode->lexeme, &(tbStack->top->ele->child), 0, NULL);
            st->aux = 1;
            trav = trav->sibling;   
        }
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "ASSIGNOP"))
    {
        /*2 children:
            1) ID_node
            2) expression
        */ 

        //Search for ID_node in the symbol table tree
        symbolTableNode *ret = searchScope(tbStack, currentNode->child);
        
        //ID_node is not declared, stop right here
        //searchscope function has already caught the error
        if(ret == NULL) 
        {
            return NULL;
        }    
        
        //ID_node var is of type array
        if(ret->ele.tag == Array)
        {
            //ID_node can't be an array var, ERROR
            return NULL;
        }

        ret->ele.data.id.isAssigned = 1;    //ID_node has been assigned something

        //Find the rightExpression's type
        type *rightType = typeChecker(currentNode->child->sibling, tbStack);

        //Either there was an error in type calculation of rightExpression or the type is an arrayType or type doesn't match
        if((rightType == NULL) || (rightType->tag == ArrayType) || strcmp(ret->ele.data.id.type, rightType->tp.type))
        {
            //Type mismatch, ERROR
            free(rightType);
            return NULL;
        }
        free(rightType);
        return NULL;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "GET_VALUE"))
    {
        /*  1 Child:
                1) ID_node
        */

        //Search for ID_node in the symbol table tree
        symbolTableNode *ret= searchScope(tbStack, currentNode->child);

        //ID_node isn't declared, stop right here
        if(ret == NULL)
            return NULL;
        
        //ID_node var is of Arry type
        if(ret->ele.tag == Array)
        {
            //Can't be array varible, ERROR
            return NULL;
        }
        
        //ID_node has been assigned something
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
        
        //The ID_node should be of Array type
        if(leftType_id->ele.tag != Array)
        {
            //Error
            return NULL;
        }
        
        //The index should be of Integer type or NUM
        if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "RNUM"))
        {
            //Index can't be a real constant, ERROR
            return NULL;
        }
        else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "NUM"))
        {
            //Its OK to have a NUM as index, but still check the bounds if the array is static
            astNode *num = currentNode->child->sibling;

            //Check if the array is static or not
            if(leftType_id->ele.data.arr.isDynamic == 0)
            {
                //Static array, do bound checking
                if(num->node->ele.leafNode->value < leftType_id->ele.data.arr.lowerIndex->value
                    || num->node->ele.leafNode->value > leftType_id->ele.data.arr.upperIndex->value)
                {
                    //out of bounds error
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
                return NULL;
            }

            //Index must be an Identifier var (not an Array var)
            if(index->ele.tag != Identifier)
            {
                //index can't be an array var, ERROR
                return NULL;
            }
            
            //Index ID isn't of integer type 
            if(strcmp(index->ele.data.id.type, "Integer"))
            {
                //Error
                return NULL;
            }
            
            /********Not required, since we don't know the value of index (because it's an ID)*********/
            // //The index should be within the bounds of the array, if the array is static
            // if(leftType_id->ele.data.arr.isDynamic == 0)
            // {
            //     // if(index->ele.data.id.value < leftType_id->ele.data.arr.lowerIndex->value
            //     //     || index->ele.data.id.value > leftType_id->ele.data.arr.upperIndex->value)
            //     // {
            //     //     //out  of bounds error
            //     // }
            // }
        }
        
        //Everything on LHS is fine, now do type checking
        type * rightType = typeChecker(currentNode->child->sibling->sibling,tbStack);
        
        //Error while calculating type of the right side expression
        if(rightType == NULL)
        {
            //Some problem/error on the RHS_expression, return NULL
            free(rightType);
            return NULL;
        }        
        
        //RHS expression can't be of an array type
        if(rightType->tag == ArrayType)
        {
            //RHS_expression can't be of an array type, ERROR
            free(rightType);
            return NULL;
        }

        //Right and left type should finally match
        if(strcmp(leftType_id->ele.data.arr.type, rightType->tp.type))
        {
            //Type mismatch, ERROR
            free(rightType);
            return NULL;
        }
        
        free(rightType);
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

        //Function is not defined
        if(ret == NULL)
        {   
            return NULL;
        }
        //Function is defined
        else
        {
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
                    return NULL;
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
                        return NULL;
                    }
                }
            }   
        }
        
        /***********************************************************************/

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
            //Search for trav in the symbol table tree
            symbolTableNode *id_arr = searchScope(tbStack, trav);

            //trav isn't declared
            if(id_arr == NULL)
            {
                //Not declared, ERROR
                return NULL;
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
                        return NULL;
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
                    }
                    //Check bounds (check only if types match)
                    if(isError = 0)
                    {
                        //check the lower index matches when they are static NUMs
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
        if(outputsize != ret->ele.data.mod.outputcount)
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
            if(ret->ele.data.mod.outputList[i].tag == id_arr->ele.tag)
            {
                // BOTH ARE IDs //
                if(id_arr->ele.tag == Identifier)
                {
                    if(strcmp(id_arr->ele.data.id.type, ret->ele.data.mod.outputList[i].data.id.type))
                    {
                        //Type mismatch, ERROR
                        id_arr->ele.data.id.isAssigned = 1;
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
        tableStackEle *temp = sympop(tbStack);
        symbolTableNode *id = searchScope(tbStack, currentNode->child);//sym_hash_find(currentNode->child->node->ele.leafNode->lexeme, &(tbStack->top->ele->hashtb),0,NULL);
        sympush(tbStack,temp);

        if(id == NULL)
        {
           //Index variable not declared, ERROR
           return NULL; 
        }
        
        if(id->ele.tag == Array)
        {
            //Can't be an array variable, ERROR
            return NULL;
        }

        if(!strcmp(id->ele.data.id.type,"REAL") || !strcmp(id->ele.data.id.type,"BOOLEAN"))
        {
            //Type invalid, ERROR
            return NULL;
        }
        
        id->ele.data.id.isIndex = 1;    //Mark this variable as index variable

        symbolTable *currentSTNode = tbStack->top->ele->child;
        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        /* NOT NEEEDED NOW */
        
        // newTable->ele = currenSTNode->child;
        // newTable->next = NULL;
        // sympush(tbStack, newTable);

        // //entering new scope
        // currenSTNode = tbStack->top->ele;

        /* NOT NEEEDED NOW */
        
        astNode *trav = currentNode->child->sibling->sibling;
        while(trav != NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") || !strcmp(trav->node->ele.internalNode->label, "SWITCH") || !strcmp(trav->node->ele.internalNode->label, "WHILE"))
            {
                // newTable = sympop(tbStack);
                newTable->ele = currentSTNode;
                newTable->next = NULL;
                sympush(tbStack,newTable);
                currentSTNode = currentSTNode->sibling;     
            }
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
        sympop(tbStack);
        free(newTable);
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
        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStackEle));

        tableStackEle *temp = sympop(tbStack);
        type *exprType = typechecker(currentNode->child, tbStack);
        sympush(tbStack,temp);

        if((exprType == NULL) || (exprType->tag == ArrayType) || (strcmp(exprType->tp.type,"BOOLEAN")))
        {
            // ERROR - Expr in While is either has error or of wrong type.
            return NULL;
        }

        astNode *trav = currentNode->child->sibling;
        while(trav != NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") || !strcmp(trav->node->ele.internalNode->label, "SWITCH") || !strcmp(trav->node->ele.internalNode->label, "WHILE"))
            {
                // newTable = sympop(tbStack);
                newTable->ele = currentSTNode;
                newTable->next = NULL;
                sympush(tbStack,newTable);
                currentSTNode = currentSTNode->sibling;
            }
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
        sympop(tbStack);
        free(newTable);
        return NULL;
    } 
    else if(!strcmp(currentNode->node->ele.internalNode->label, "SWITCH"))
    {
        /*  3 Children
                1) ID
                2) Case
                3) Default
        */
        symbolTableNode *id = searchScope(tbStack, currentNode->child); //sym_hash_find(currentNode->child->node->ele.leafNode->lexeme, &(tbStack->top->ele->hashtb),0,NULL);
        
        if(id == NULL)
        {
           //Index variable not declared, ERROR
           return NULL;
        }
        
        if(id->ele.tag == Array)
        {
            //Can't be an array variable, ERROR
            return NULL;
        }

        if(!strcmp(id->ele.data.id.type,"REAL"))
        {
            //Type invalid, ERROR
            return NULL;
        }

                
        astNode * trav = currentNode->child->sibling;
        if(!strcmp(id->ele.data.id.type,"INTEGER"))
        {
            astNode* findDefault = currentNode->child;
            while(findDefault->sibling!=NULL)
            {
                findDefault = findDefault->sibling;
            }
            if(strcmp(findDefault->node->ele.internalNode->label,"DEFAULT"))
            {
                //error DEFAULT IS MISSING
                return NULL;
            }
            
            while(!strcmp(trav->node->ele.internalNode->label, "DEFAULT"))
            {
                if(!strcmp(trav->child->node->ele.leafNode->type, "RNUM") || !strcmp(trav->child->node->ele.leafNode->type, "TRUE") || !strcmp(trav->child->node->ele.leafNode->type, "FALSE"))
                {
                    //Value can't be a real constant, true or false, ERROR
                }
                //Now value is either ID or NUM
                if(!strcmp(trav->child->node->ele.leafNode->type, "ID"))
                {
                    type *valType = typeChecker(trav->child, tbStack);

                    if(valType == NULL)
                    {
                        //ID isn't declared, ERROR
                        return NULL;
                    }
                    if(valType->tag == ArrayType)
                    {
                        //ID can't be a array var, ERROR
                        return NULL;
                    }
                    if(strcmp(valType->tp.type, "INTEGER"))
                    {
                        //ID can't be of any type except integer, ERROR
                        return NULL;
                    }
                }
                trav = trav->sibling;
            }
        }
        astNode * trav = currentNode->child->sibling;
        if(!strcmp(id->ele.data.id.type,"BOOLEAN"))
        {
            astNode* findDefault = currentNode->child;
            while(findDefault->sibling!=NULL)
            {
                findDefault = findDefault->sibling;
            }
            if(!strcmp(findDefault->node->ele.internalNode->label,"DEFAULT"))
            {
                //error DEFAULT IS PRESENT
                return NULL;
            }
            while(trav != NULL)
            {
                if(!strcmp(trav->child->node->ele.leafNode->type, "RNUM") || !strcmp(trav->child->node->ele.leafNode->type, "NUM"))
                {
                    //Value can't be a real constant, ERROR
                }
                if(!strcmp(trav->child->node->ele.leafNode->type, "ID"))
                {
                    type *valType = typeChecker(trav->child, tbStack);
                    if(valType == NULL)
                    {
                        //ID isn't declared, ERROR
                        return NULL;
                    }
                    if(valType->tag == ArrayType)
                    {
                        //ID can't be a array var, ERROR
                        return NULL;
                    }
                    if(strcmp(valType->tp.type, "BOOLEAN"))
                    {
                        //ID can't be of any type except integer, ERROR
                        return NULL;
                    }
                }
                trav = trav->sibling;
            }
        }     
        astNode * trav = currentNode->child;

        while(trav != NULL)
        {
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
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
        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        
        astNode *trav = currentNode->child->sibling;
        while(trav != NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") || !strcmp(trav->node->ele.internalNode->label, "SWITCH") || !strcmp(trav->node->ele.internalNode->label, "WHILE"))
            {
                // newTable = sympop(tbStack);
                newTable->ele = currentSTNode;
                newTable->next = NULL;
                sympush(tbStack,newTable);
                currentSTNode = currentSTNode->sibling;
            }
            typeChecker(trav, tbStack);
            trav = trav->sibling;
        }
        sympop(tbStack);
        free(newTable);
        return NULL;
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
                free(leftType);
                free(rightType);
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
                        free(leftType);
                        free(rightType);
                        return  NULL;
                    } 

                    free(leftType);
                    free(rightType);          
                    return retType;         
                }
                else
                {
                    //Type mismatch among operands, ERROR
                    free(leftType);
                    free(rightType);
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
                        free(leftType);
                        free(rightType);
                        return  NULL;
                    } 
                    free(leftType);
                    free(rightType);          
                    return retType;         
                }
                else
                {
                    //Type mismatch among operands, ERROR
                    free(leftType);
                    free(rightType);
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
                        free(leftType);
                        free(rightType);
                        return  NULL;
                    }   
                    free(leftType);
                    free(rightType);        
                    return retType;         
                }
                else
                {
                    //Type mismatch among operands, ERROR
                    free(retType);
                    free(leftType);
                    free(rightType);
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
                free(childType);
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
                free(retType);
                free(childType);
                return NULL;
            }
            free(childType);
            return retType;
        }
    } 
}