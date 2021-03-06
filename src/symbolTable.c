/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

#include<string.h>
#include<math.h>
#include "symbolTable.h"

// push into symbol table stack
void sympush(tableStack *stack, tableStackEle *newNode)
{
    if(stack->top == NULL)
    {
        stack->size = 1;
        stack->top = newNode;
        stack->bottom = newNode;
        newNode->next = NULL;
        return;
    }

    newNode->next = stack->top;
    stack->top = newNode;
    stack->size++;
}

//pop from symbol table stack
tableStackEle * sympop(tableStack *stack)
{
    if(stack->size != 0)
    {
        tableStackEle * temp = stack->top;
        stack->top = stack->top->next;
        stack->size--;

        if(stack->size==0)
            stack->bottom = NULL;
        temp->next = NULL;
        return temp;
    }
    else
      return NULL;
}

//return the hash value of the str in the hash table 
int sym_hash_func(hashSym *hashtb,char *str)
{
    int i = 0 ;
    long long val = 11 ;

    while(str[i] != '\0')
    {
        //We use 31 as a prime number - creating 31-ary encoded hash numbers 
		val = val*31 + str[i] ;
        i++ ;
    }

	//if the value has overflown, invert it
	//cause we can still deal with positive indices
    if(val < 0)
        val = val * (-1);

	//returns where in the hash table the string should be found/inserted.
    return val % hashtb->hashtbSize;
}


symbolTableNode* sym_hash_find(char * str, hashSym * hash_tb, int replace, symbolTableNode *key)
{
    int hash = sym_hash_func(hash_tb, str);
    
	//access the head at that position, which is of type Node*
    symbolTableNode * trav = hash_tb->arr[hash].head;
    char * lexeme;
    while(trav != NULL)
    {
        //looking at a identifier
        if(trav->ele.tag == Identifier)
            lexeme = trav->ele.data.id.lexeme;
		//looking at an arrayw
        else if (trav->ele.tag == Array)
            lexeme = trav->ele.data.arr.lexeme;
        //looking at a module
        else if(trav->ele.tag == Module)
            lexeme = trav->ele.data.mod.lexeme;
		//return the appropriate node
        if(strcmp(lexeme,str) == 0)
        {
            // if(replace == 0)
            //     return trav;

            symbolTableNode *tmp = (symbolTableNode*)malloc(sizeof(symbolTableNode));
            *tmp = *trav;
            if(replace == 1)
                *trav = *key;
            if(replace == 1)
                return tmp;         
            else
            {
                if(tmp!=NULL) free(tmp);
                return trav;
            }
        }
		//keep searching
        trav = trav->next;
    }

	//if not present, returns NULL
    return NULL;
}

// create a new symbol table twice the size of the previous
hashSym *rehash(hashSym *oldTable)
{
    hashSym *newTable = (hashSym *)malloc(sizeof(hashSym));
    newTable->hashtbSize = (oldTable->hashtbSize)*2;
    newTable->eleCount = 0;
    newTable->arr = (linkedListSym*)malloc(sizeof(linkedListSym)*newTable->hashtbSize);

    symbolTableNode * trav = NULL;
    symbolTableNode * tmp;
    for(int i=0; i<oldTable->hashtbSize; i++)
    {
        if(oldTable->arr[i].head == NULL && oldTable->arr[i].tail == NULL)
            continue;
        trav = oldTable->arr[i].head;
        while(trav != NULL)
        {
            tmp = trav->next;
            sym_hash_insert(trav, newTable);
            trav = tmp;
        }
    }
    if(oldTable->arr!=NULL) free(oldTable->arr);
    if(oldTable!=NULL) free(oldTable);
    return newTable;    
}

//common for both hash tables of keywords and TorNT
symbolTableNode* sym_hash_insert(symbolTableNode * newNode, hashSym * hash_tb)
{
	//extract the name of the lexeme according to the tag
    char * str;
    if(newNode->ele.tag == Identifier)
        str = newNode->ele.data.id.lexeme;
    else if(newNode->ele.tag == Array)
        str = newNode->ele.data.arr.lexeme;
    else if(newNode->ele.tag == Module)
        str = newNode->ele.data.mod.lexeme;
    
	//find if it's already there in the hash table
    symbolTableNode *find = sym_hash_find(str, hash_tb, 0, newNode);
    if(find != NULL)
    {
        //return what you found, there is something fishy
        return find;
    }
    int hash;
    
    hash_tb->eleCount++;
    if(hash_tb->eleCount > (3*hash_tb->hashtbSize)/2)
        hash_tb = rehash(hash_tb);

	//calculate the correct position in the hash table
    if(newNode->ele.tag == Identifier)
        hash = sym_hash_func(hash_tb, newNode->ele.data.id.lexeme);
    else if (newNode->ele.tag == Array)
        hash = sym_hash_func(hash_tb, newNode->ele.data.arr.lexeme);
    else if(newNode->ele.tag == Module)
        hash = sym_hash_func(hash_tb, newNode->ele.data.mod.lexeme);

    //if the list is empty 
    newNode->next = NULL;
    if( hash_tb->arr[hash].head == NULL)
    {
        hash_tb->arr[hash].head = newNode;
        hash_tb->arr[hash].tail = newNode;
        hash_tb->arr[hash].size = 1;
    }
	//if the list is not empty
    else
    {
        hash_tb->arr[hash].tail->next = newNode;
        hash_tb->arr[hash].tail = newNode;
        hash_tb->arr[hash].size += 1;
    }
    
    //inserted fine, so return NULL
    return NULL;
}
//simply mallocates memory to the hash table ADT
void initializeHashSym(hashSym *hash_tb)
{
	hash_tb->hashtbSize = INTIALHASHSIZE;
    hash_tb->eleCount = 0;
    hash_tb->arr = (linkedListSym*)malloc(sizeof(linkedListSym)* hash_tb->hashtbSize);
    for(int i = 0; i < hash_tb->hashtbSize ; i++)
    {
        hash_tb->arr[i].size = 0;
        hash_tb->arr[i].head = NULL;
        hash_tb->arr[i].tail = NULL;
    }
    return;
}

// mallocates memory to the symbol table that use the above function to make a hash table
symbolTable* initializeSymbolTable(char *str, int lineNumStart, int lineNumEnd)
{
    symbolTable * ST = (symbolTable*)malloc(sizeof(symbolTable));
    ST->child = NULL;
    ST->sibling = NULL;
    ST->symLexeme = str;
    ST->lineNumStart = lineNumStart;
    ST->lineNumEnd = lineNumEnd;
    hashSym *hashtb = (hashSym*)malloc(sizeof(hashSym));
    initializeHashSym(hashtb);
    ST->hashtb = *hashtb;
    ST->currentOffset = 0;
    ST->dummyCurrentOffset = 0;
    return ST;    
}

void initializeErrorList()
{
    semErrorList = (semanticError *)malloc(sizeof(semanticError));
    semErrorList->numErrors = 0;
    semErrorList->head = NULL;
}


void insertSemError(semanticErrorNode *err)
{
    semErrorList->numErrors++;
    
    if(semErrorList->head == NULL)
    {
        semErrorList->head = err;
        return;
    }
    
    semanticErrorNode* trav = semErrorList->head;
    while(trav->next!=NULL)
        trav=trav->next;
    
    err->next=NULL;
    trav->next = err;       
    
}

void formulation(astNode *astRoot, symbolTable *current)
{
    if(astRoot == NULL)
        return;
    // if(astRoot->node->tag == Internal)
    //     printf("%s\n", astRoot->node->ele.internalNode->label);
    // else
    //     printf("%s\n", astRoot->node->ele.leafNode->type);
    
    if(astRoot->node->tag == Leaf)
        return;

    if(!strcmp(astRoot->node->ele.internalNode->label, "PROGRAM"))
    {
        char *str = (char*)malloc(sizeof(char)*8); strcpy(str,"Program");
        symbolTable *programST = initializeSymbolTable(str, astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
        
        // Pass the Root Symbol Table Node
        // Base case

        symbolTableRoot = programST;

        formulation(astRoot->child, programST);
        formulation(astRoot->child->sibling, programST);
        formulation(astRoot->child->sibling->sibling, programST);
        formulation(astRoot->child->sibling->sibling->sibling, programST);
    }
    else if(!strcmp(astRoot->node->ele.internalNode->label, "DECLARE"))
    {
        //ID_LIST - Child1
        //type - Child2
                //Child2 can either be of array type or INTEGER/REAL/BOOLEAN
                
        astNode *idlist = astRoot->child->child;
        astNode *type = astRoot->child->sibling;
        while(idlist != NULL)
        {
            //Its of array type
            symbolTableNode *newNode = (symbolTableNode*)malloc(sizeof(symbolTableNode));
            newNode->aux = 0;
            if(type->node->tag == Internal)
            {
                
                newNode->ele.tag = Array;
                newNode->ele.data.arr.lexeme = idlist->node->ele.leafNode->lexeme;
                newNode->ele.data.arr.type = type->child->sibling->node->ele.leafNode->type;

                newNode->ele.data.arr.lowerIndex = (identifier*)malloc(sizeof(identifier));
                newNode->ele.data.arr.lowerIndex->lexeme = type->child->child->node->ele.leafNode->lexeme;
                newNode->ele.data.arr.lowerIndex->type = type->child->child->node->ele.leafNode->type;
                newNode->ele.data.arr.lowerIndex->value = type->child->child->node->ele.leafNode->value;

                newNode->ele.data.arr.upperIndex = (identifier*)malloc(sizeof(identifier));
                newNode->ele.data.arr.upperIndex->lexeme = type->child->child->sibling->node->ele.leafNode->lexeme;
                newNode->ele.data.arr.upperIndex->type = type->child->child->sibling->node->ele.leafNode->type;
                newNode->ele.data.arr.upperIndex->value = type->child->child->sibling->node->ele.leafNode->value;
                
                newNode->lineNum = idlist->node->ele.leafNode->lineNum;

                int tmp = 0, dummyTmp = 0;
                if(!strcmp(type->child->sibling->node->ele.leafNode->type,"INTEGER"))
                {
                    tmp = INTEGER_SIZE;
                    dummyTmp = DUMMY_INTEGER_SIZE;
                }
                else if(!strcmp(type->child->sibling->node->ele.leafNode->type,"REAL"))
                {                
                    tmp = REAL_SIZE;
                    dummyTmp = DUMMY_REAL_SIZE;
                }
                else if(!strcmp(type->child->sibling->node->ele.leafNode->type,"BOOLEAN"))
                {
                    tmp = BOOLEAN_SIZE;
                    dummyTmp = DUMMY_BOOLEAN_SIZE;
                }
                
                if( (!strcmp(newNode->ele.data.arr.lowerIndex->type,"ID")) 
                || (!strcmp(newNode->ele.data.arr.upperIndex->type,"ID")) )
                {
                    //dynamic array
                    newNode->width = POINTER_SIZE;
                    newNode->offset = currentOffset + newNode->width;
                    currentOffset += POINTER_SIZE;
                    newNode->ele.data.arr.isDynamic = 1;  
                    newNode->dummyWidth = DUMMY_POINTER_SIZE;
                    newNode->dummyOffset = dummyCurrentOffset;
                    dummyCurrentOffset += newNode->dummyWidth; 
                }
                else    
                {
                    //static array
                    newNode->ele.data.arr.isDynamic = 0;
                    int size = *(int *)newNode->ele.data.arr.upperIndex->value - *(int*)newNode->ele.data.arr.lowerIndex->value + 1;
                    newNode->width = tmp * size + POINTER_SIZE;
                    newNode->offset = currentOffset + POINTER_SIZE;
                    currentOffset += newNode->width;
                    newNode->dummyOffset = dummyCurrentOffset;
                    newNode->dummyWidth = dummyTmp*size + DUMMY_POINTER_SIZE;
                    dummyCurrentOffset += newNode->dummyWidth;
                }
                newNode->isParameter = 0;   //Its a local var, not an I/O parameter
                newNode->next = NULL;
            }
            //Its an ID
            else
            {
                newNode->ele.tag = Identifier;

                newNode->ele.data.id.lexeme = idlist->node->ele.leafNode->lexeme;
                newNode->ele.data.id.type = type->node->ele.leafNode->type;
                newNode->ele.data.id.value = idlist->node->ele.leafNode->value;
                newNode->lineNum = idlist->node->ele.leafNode->lineNum;
                
                if(!strcmp(type->node->ele.leafNode->type,"INTEGER"))
                {
                    newNode->width = INTEGER_SIZE;
                    newNode->dummyWidth = DUMMY_INTEGER_SIZE;
                }
                else if(!strcmp(type->node->ele.leafNode->type,"REAL"))
                {
                    newNode->width = REAL_SIZE;
                    newNode->dummyWidth = DUMMY_REAL_SIZE;
                }
                else if(!strcmp(type->node->ele.leafNode->type,"BOOLEAN"))
                {
                    newNode->width = BOOLEAN_SIZE;
                    newNode->dummyWidth = DUMMY_BOOLEAN_SIZE;
                }
                    
                newNode->offset = currentOffset + newNode->width;
                currentOffset += newNode->width;
                newNode->dummyOffset = dummyCurrentOffset;
                dummyCurrentOffset += newNode->dummyWidth;
                newNode->isParameter = 0;   //Its a local var, not an I/O parameter
                newNode->next = NULL;
            }
            symbolTableNode *ret = sym_hash_insert(newNode, &(current->hashtb));
            if(ret != NULL) //redeclaration of an ID within the same scope
            {
                char *err = (char *)malloc(sizeof(char)*250);
                char *whatType = (char *)malloc(sizeof(char)*10);
                char *lexeme = NULL, *type = NULL;
                int lineNum;

                lineNum = ret->lineNum;
                if(ret->ele.tag == Identifier)
                {
                    lexeme = ret->ele.data.id.lexeme;
                    type = ret->ele.data.id.type;  
                    strcpy(whatType, "ID");
                }
                else if(ret->ele.tag == Array)
                {
                    lexeme = ret->ele.data.arr.lexeme;
                    type = ret->ele.data.arr.type;
                    strcpy(whatType, "ARRAY");
                }
                else if(ret->ele.tag == Module)
                {
                    lexeme = ret->ele.data.mod.lexeme;
                    type = "Module\0";
                    strcpy(whatType, "MODULE");
                }

                sprintf(err, "Line %d: %s (%s, %s) variable is already declared at %d. Redeclaration of %s at %d", idlist->node->ele.leafNode->lineNum, lexeme, whatType, type, lineNum, lexeme, idlist->node->ele.leafNode->lineNum);
                semanticErrorNode *errNode = (semanticErrorNode *)malloc(sizeof(semanticErrorNode));
                errNode->errorMessage = err;
                errNode->next = NULL;
                insertSemError(errNode);
                // if(ret!=NULL) free(ret);
            }
            else    //No redeclaration error, check for clash with the output_list
            {
                int isModError = 0;
                ret = sym_hash_find(current->symLexeme, &(symbolTableRoot->hashtb), 0, NULL);
                if(ret != NULL)
                {
                    int size = ret->ele.data.mod.outputcount;
                    char *lexeme = NULL;
                    for(int i=0; i<size; i++)
                    {
                        if(ret->ele.data.mod.outputList[i].tag == Identifier)
                        {
                            if(!strcmp(ret->ele.data.mod.outputList[i].data.id.lexeme, idlist->node->ele.leafNode->lexeme))
                            {
                                isModError = 1;
                                lexeme = ret->ele.data.mod.outputList[i].data.id.lexeme;
                            }
                        }
                        else if(ret->ele.data.mod.outputList[i].tag == Array)
                        {
                            if(!strcmp(ret->ele.data.mod.outputList[i].data.arr.lexeme, idlist->node->ele.leafNode->lexeme))
                            {
                                isModError = 1;
                                lexeme = ret->ele.data.mod.outputList[i].data.arr.lexeme;
                            }
                        }
                        if(isModError == 1)
                        {
                            char *err = (char *)malloc(sizeof(char)*250);
                            sprintf(err, "Line %d: Declaration of %s at line number %d is invalid. Already defined in the output list of module %s (Shadowing of output parameters is illegal).", idlist->node->ele.leafNode->lineNum, lexeme, idlist->node->ele.leafNode->lineNum, current->symLexeme);
                            semanticErrorNode *errNode = (semanticErrorNode *)malloc(sizeof(semanticErrorNode));
                            errNode->errorMessage = err;
                            errNode->next = NULL;
                            insertSemError(errNode);
                            break;
                        }
                    }
                }
            }
            idlist = idlist->sibling;
        }
    }
    else if(!strcmp(astRoot->node->ele.internalNode->label, "MODULEDEC"))
    {
        //insert in the current scope
        char *str = (char*)malloc(sizeof(char)*20); strcpy(str,"Module Declarations");
        symbolTable *moduleDecST = initializeSymbolTable(str, astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
        astNode *trav = astRoot->child;

        while(trav != NULL)
        {
            symbolTableNode *newNode = (symbolTableNode*)malloc(sizeof(symbolTableNode));
            newNode->aux = 0;
            newNode->ele.tag = Module;
            newNode->ele.data.mod.lexeme = trav->node->ele.leafNode->lexeme;
            newNode->ele.data.mod.inputcount = 0;
            newNode->ele.data.mod.outputcount = 0;
            newNode->ele.data.mod.inputList = NULL;
            newNode->ele.data.mod.outputList = NULL;
            newNode->isParameter = 0;   //Its a module name, not a variable
            newNode->lineNum = trav->node->ele.leafNode->lineNum;
            newNode->next = NULL; 
            dummyCurrentOffset = 0;
            currentOffset = 0;
            symbolTableNode* ret = sym_hash_insert(newNode, &(moduleDecST->hashtb));
            if(ret != NULL)
            {
                char *err = (char *)malloc(sizeof(char)*250);
                char *whatType = (char *)malloc(sizeof(char)*10);
                char *lexeme = NULL, *type = NULL;
                int lineNum;

                lineNum = ret->lineNum;

                // Identifier and array can be removed here //not used 
                if(ret->ele.tag == Identifier)
                {
                    lexeme = ret->ele.data.id.lexeme;
                    type = ret->ele.data.id.type;  
                    strcpy(whatType, "ID");
                }
                else if(ret->ele.tag == Array)
                {
                    lexeme = ret->ele.data.arr.lexeme;
                    type = ret->ele.data.arr.type;
                    strcpy(whatType, "ARRAY");
                }
                else if(ret->ele.tag == Module)
                {
                    lexeme = ret->ele.data.mod.lexeme;
                    type = "Module\0";
                    strcpy(whatType, "MODULE");
                }

                sprintf(err, "Line %d: %s (%s, %s) module name is already declared at %d. Redeclaration of %s at %d", trav->node->ele.leafNode->lineNum, lexeme, whatType, type, lineNum, lexeme, trav->node->ele.leafNode->lineNum);
                semanticErrorNode *errNode = (semanticErrorNode *)malloc(sizeof(semanticErrorNode));
                errNode->errorMessage = err;
                errNode->next = NULL;
                insertSemError(errNode);
                // if(ret!=NULL) free(ret);
            }
            
            trav = trav->sibling;
        }


        //linking of symbols tables
        symbolTable *tmp = current->child;
        if(tmp == NULL)
        {
            current->child = moduleDecST;
        }
        else
        {
            while(tmp->sibling != NULL)
            {
                tmp = tmp->sibling;
            }
            tmp->sibling = moduleDecST;
        }    
    }
    else if(!strcmp(astRoot->node->ele.internalNode->label, "MODULE"))
    {
        char *str = (char*)malloc(sizeof(char)*21); strcpy(str,astRoot->child->node->ele.leafNode->lexeme);
        symbolTable * moduleST = initializeSymbolTable(str,astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
        
        // Child 1 = ID_node
        // Child 2 = inpList_node
        // Child 3 = ret
        // Child 4 = modDef_node

        //This newNode will be inserted in the Program (the root) symbol table
        symbolTableNode *newNode = (symbolTableNode*)malloc(sizeof(symbolTableNode));
        newNode->aux = 0;
        newNode->ele.tag = Module;
        newNode->ele.data.mod.lexeme = astRoot->child->node->ele.leafNode->lexeme;
        newNode->ele.data.mod.inputcount = 0;
        newNode->ele.data.mod.outputcount = 0;
        newNode->isParameter = 1;   
        currentOffset = 0;
        dummyCurrentOffset = 0;
        //head of the input list
        astNode *traveller = astRoot->child->sibling->child;
        while(traveller!=NULL)
        {
            newNode->ele.data.mod.inputcount++;
            traveller=traveller->sibling;
        }
        
        //head of the output list
        traveller = astRoot->child->sibling->sibling->child;
        while(traveller!=NULL)
        {
            newNode->ele.data.mod.outputcount++;
            traveller=traveller->sibling;
        }
        newNode->ele.data.mod.inputcount = newNode->ele.data.mod.inputcount/2;
        newNode->ele.data.mod.outputcount = newNode->ele.data.mod.outputcount/2;    
        
        newNode->ele.data.mod.inputList = (elementSym*)malloc(sizeof(elementSym)*newNode->ele.data.mod.inputcount);
        newNode->ele.data.mod.outputList = (elementSym*)malloc(sizeof(elementSym)*newNode->ele.data.mod.outputcount);

        //Put the variables in input list inside its hashTable
        traveller = astRoot->child->sibling->child;
        int i = 0;
        int makeUnique = 1;
        while(traveller!=NULL)
        {
            symbolTableNode *node = (symbolTableNode*)malloc(sizeof(symbolTableNode));
            node->aux = 0;
            if(traveller->sibling->node->tag == Internal)
            {
                //traveller->sibling is an internal node with label "ARRAY"
                //Its children are:
                    //1) RangeNode
                    //2) BasicDataType Node
                node->ele.tag = Array;
                node->ele.data.arr.lexeme = traveller->node->ele.leafNode->lexeme;
                node->ele.data.arr.type = traveller->sibling->child->sibling->node->ele.leafNode->type;

                node->ele.data.arr.lowerIndex = (identifier*)malloc(sizeof(identifier));
                node->ele.data.arr.lowerIndex->lexeme = traveller->sibling->child->child->node->ele.leafNode->lexeme;
                node->ele.data.arr.lowerIndex->type = traveller->sibling->child->child->node->ele.leafNode->type;
                node->ele.data.arr.lowerIndex->value = traveller->sibling->child->child->node->ele.leafNode->value;

                node->ele.data.arr.upperIndex = (identifier*)malloc(sizeof(identifier));
                node->ele.data.arr.upperIndex->lexeme = traveller->sibling->child->child->sibling->node->ele.leafNode->lexeme;
                node->ele.data.arr.upperIndex->type = traveller->sibling->child->child->sibling->node->ele.leafNode->type;
                node->ele.data.arr.upperIndex->value = traveller->sibling->child->child->sibling->node->ele.leafNode->value;

                node->lineNum = traveller->node->ele.leafNode->lineNum;
                
                // int tmp=0;
                // if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"INTEGER"))
                //     tmp = INTEGER_SIZE;
                // else if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"REAL"))
                //     tmp = REAL_SIZE;
                // else if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"BOOLEAN"))
                //     tmp = BOOLEAN_SIZE;

                char * lexeme = (char *)malloc(sizeof(char)*11);
                memset(lexeme, '\0', sizeof(char)*11);
                symbolTableNode *lowerIndexNode = (symbolTableNode*)malloc(sizeof(symbolTableNode));
                lowerIndexNode->ele.tag = Identifier;

                if(!strcmp(traveller->sibling->child->child->node->ele.leafNode->type, "NUM"))
                {
                    sprintf(lexeme, "%s_%d", traveller->sibling->child->child->node->ele.leafNode->lexeme, makeUnique);
                    makeUnique++;
                    lowerIndexNode->ele.data.id.lexeme = lexeme;
                    lowerIndexNode->ele.data.id.value = traveller->sibling->child->child->node->ele.leafNode->value;
                }
                else
                {
                    lowerIndexNode->ele.data.id.lexeme = traveller->sibling->child->child->node->ele.leafNode->lexeme;
                    lowerIndexNode->ele.data.id.value = traveller->sibling->child->child->node->ele.leafNode->value;
                }
                lowerIndexNode->ele.data.id.type = (char *)malloc(sizeof(char)*11);
                strcpy(lowerIndexNode->ele.data.id.type, "INTEGER\0");
                lowerIndexNode->aux = 0;
                lowerIndexNode->lineNum = traveller->sibling->child->child->node->ele.leafNode->lineNum;
                lowerIndexNode->next = NULL;

                lexeme = (char *)malloc(sizeof(char)*11);
                memset(lexeme, '\0', sizeof(char)*11);
                symbolTableNode *upperIndexNode = (symbolTableNode*)malloc(sizeof(symbolTableNode));
                upperIndexNode->ele.tag = Identifier;
                if(!strcmp(traveller->sibling->child->child->sibling->node->ele.leafNode->type, "NUM"))
                {
                    sprintf(lexeme, "%s_%d", traveller->sibling->child->child->sibling->node->ele.leafNode->lexeme, makeUnique);
                    makeUnique++;
                    upperIndexNode->ele.data.id.lexeme = lexeme;
                    upperIndexNode->ele.data.id.value = traveller->sibling->child->child->sibling->node->ele.leafNode->value;
                }
                else
                {
                    upperIndexNode->ele.data.id.lexeme = traveller->sibling->child->child->sibling->node->ele.leafNode->lexeme;
                    upperIndexNode->ele.data.id.value = traveller->sibling->child->child->sibling->node->ele.leafNode->value;
                }
                upperIndexNode->ele.data.id.type = (char *)malloc(sizeof(char)*11);
                strcpy(upperIndexNode->ele.data.id.type, "INTEGER\0");
                upperIndexNode->aux = 0;
                upperIndexNode->lineNum = traveller->sibling->child->child->sibling->node->ele.leafNode->lineNum;
                upperIndexNode->next = NULL;

                if( (!strcmp(node->ele.data.arr.lowerIndex->type,"ID")) 
                || (!strcmp(node->ele.data.arr.upperIndex->type,"ID")) )
                {
                    //dynamic array
                    node->ele.data.arr.isDynamic = 1;   
                }
                else    
                {                   
                    //static array
                    node->ele.data.arr.isDynamic = 0;
                }
                node->offset = currentOffset;
                node->width = POINTER_SIZE;
                currentOffset += node->width;

                lowerIndexNode->width = INTEGER_SIZE;
                lowerIndexNode->offset = currentOffset;
                currentOffset += lowerIndexNode->width;
                        
                upperIndexNode->width = INTEGER_SIZE;
                upperIndexNode->offset = currentOffset;
                currentOffset += upperIndexNode->width;

                lowerIndexNode->isParameter = 2;
                upperIndexNode->isParameter = 2;
                node->isParameter = 1;
                node->next = NULL;
                
                node->dummyOffset = dummyCurrentOffset;
                node->dummyWidth = (2*DUMMY_INTEGER_SIZE) + DUMMY_POINTER_SIZE;
                dummyCurrentOffset += node->dummyWidth;
                sym_hash_insert(lowerIndexNode, &(moduleST->hashtb));
                sym_hash_insert(upperIndexNode, &(moduleST->hashtb));
            }
            //Its an ID
            else
            {
                node->ele.tag = Identifier;
                node->ele.data.id.lexeme = traveller->node->ele.leafNode->lexeme;
                node->ele.data.id.type = traveller->sibling->node->ele.leafNode->type;
                node->ele.data.id.value = traveller->node->ele.leafNode->value;
                node->lineNum = traveller->node->ele.leafNode->lineNum;
                
                if(!strcmp(traveller->sibling->node->ele.leafNode->type,"INTEGER"))
                {
                    node->width = INTEGER_SIZE;
                    node->dummyWidth = DUMMY_INTEGER_SIZE;
                }
                else if(!strcmp(traveller->sibling->node->ele.leafNode->type,"REAL"))
                {
                    node->width = REAL_SIZE;
                    node->dummyWidth = DUMMY_REAL_SIZE;
                }
                else if(!strcmp(traveller->sibling->node->ele.leafNode->type,"BOOLEAN"))
                {
                    node->width = BOOLEAN_SIZE;
                    node->dummyWidth = DUMMY_BOOLEAN_SIZE;
                }
                node->next = NULL;
                node->offset = currentOffset;
                currentOffset += node->width;
                node->isParameter = 1;

                node->dummyOffset = dummyCurrentOffset;
                dummyCurrentOffset += node->dummyWidth;
            }
            newNode->ele.data.mod.inputList[i] = node->ele;
            symbolTableNode *ret = sym_hash_insert(node, &(moduleST->hashtb));
            if(ret != NULL)
            {
                char *err = (char *)malloc(sizeof(char)*250);
                char *whatType = (char *)malloc(sizeof(char)*10);
                char *lexeme = NULL, *type = NULL;
                int lineNum;

                lineNum = ret->lineNum;
                if(ret->ele.tag == Identifier)
                {
                    lexeme = ret->ele.data.id.lexeme;
                    type = ret->ele.data.id.type;  
                    strcpy(whatType, "ID");
                }
                else if(ret->ele.tag == Array)
                {
                    lexeme = ret->ele.data.arr.lexeme;
                    type = ret->ele.data.arr.type;
                    strcpy(whatType, "ARRAY");
                }
                else if(ret->ele.tag == Module)
                {
                    lexeme = ret->ele.data.mod.lexeme;
                    type = "Module\0";
                    strcpy(whatType, "MODULE");
                }
                sprintf(err, "Line %d: %s (%s, %s) variable is already declared at %d. Redeclaration of %s at %d", 
                traveller->node->ele.leafNode->lineNum, lexeme, whatType, type, lineNum, lexeme, traveller->node->ele.leafNode->lineNum);
                semanticErrorNode *errNode = (semanticErrorNode *)malloc(sizeof(semanticErrorNode));
                errNode->errorMessage = err;
                errNode->next = NULL;
                insertSemError(errNode);
                // if(ret!=NULL) free(ret);
            }
            //skip two at a time cause we have [ ID -> Datatype -> ID -> DataType .... ]
            traveller = traveller->sibling->sibling;
            i++;
        }
        traveller = astRoot->child->sibling->sibling->child;
        i = 0;
        //Put the variables in output list inside its hashTable
        while(traveller!=NULL)
        {
            symbolTableNode *node = (symbolTableNode*)malloc(sizeof(symbolTableNode));
            node->aux = 0;
            if(traveller->sibling->node->tag == Internal)
            {
                node->ele.tag = Array;
                node->ele.data.arr.lexeme = traveller->node->ele.leafNode->lexeme;
                node->ele.data.arr.type = traveller->sibling->child->sibling->node->ele.leafNode->type;

                node->ele.data.arr.lowerIndex = (identifier*)malloc(sizeof(identifier));
                node->ele.data.arr.lowerIndex->lexeme = traveller->sibling->child->child->node->ele.leafNode->lexeme;
                node->ele.data.arr.lowerIndex->type = traveller->sibling->child->child->node->ele.leafNode->type;
                node->ele.data.arr.lowerIndex->value = traveller->sibling->child->child->node->ele.leafNode->value;

                node->ele.data.arr.upperIndex = (identifier*)malloc(sizeof(identifier));
                node->ele.data.arr.upperIndex->lexeme = traveller->sibling->child->child->sibling->node->ele.leafNode->lexeme;
                node->ele.data.arr.upperIndex->type = traveller->sibling->child->child->sibling->node->ele.leafNode->type;
                node->ele.data.arr.upperIndex->value = traveller->sibling->child->child->sibling->node->ele.leafNode->value;

                node->lineNum = traveller->node->ele.leafNode->lineNum;
                
                char * lexeme = (char *)malloc(sizeof(char)*11);
                memset(lexeme, '\0', sizeof(char)*11);
                symbolTableNode *lowerIndexNode = (symbolTableNode*)malloc(sizeof(symbolTableNode));
                lowerIndexNode->ele.tag = Identifier;

                if(!strcmp(traveller->sibling->child->child->node->ele.leafNode->type, "NUM"))
                {
                    sprintf(lexeme, "%s_%d", traveller->sibling->child->child->node->ele.leafNode->lexeme, makeUnique);
                    makeUnique++;
                    lowerIndexNode->ele.data.id.lexeme = lexeme;
                    lowerIndexNode->ele.data.id.value = traveller->sibling->child->child->node->ele.leafNode->value;
                }
                else
                {
                    lowerIndexNode->ele.data.id.lexeme = traveller->sibling->child->child->node->ele.leafNode->lexeme;
                    lowerIndexNode->ele.data.id.value = traveller->sibling->child->child->node->ele.leafNode->value;
                }
                lowerIndexNode->ele.data.id.type = (char *)malloc(sizeof(char)*11);
                strcpy(lowerIndexNode->ele.data.id.type, "INTEGER\0");
                lowerIndexNode->aux = 0;
                lowerIndexNode->lineNum = traveller->sibling->child->child->node->ele.leafNode->lineNum;
                lowerIndexNode->next = NULL;

                lexeme = (char *)malloc(sizeof(char)*11);
                memset(lexeme, '\0', sizeof(char)*11);
                symbolTableNode *upperIndexNode = (symbolTableNode*)malloc(sizeof(symbolTableNode));
                upperIndexNode->ele.tag = Identifier;
                if(!strcmp(traveller->sibling->child->child->sibling->node->ele.leafNode->type, "NUM"))
                {
                    sprintf(lexeme, "%s_%d", traveller->sibling->child->child->sibling->node->ele.leafNode->lexeme, makeUnique);
                    makeUnique++;
                    upperIndexNode->ele.data.id.lexeme = lexeme;
                    upperIndexNode->ele.data.id.value = traveller->sibling->child->child->sibling->node->ele.leafNode->value;
                }
                else
                {
                    upperIndexNode->ele.data.id.lexeme = traveller->sibling->child->child->sibling->node->ele.leafNode->lexeme;
                    upperIndexNode->ele.data.id.value = traveller->sibling->child->child->sibling->node->ele.leafNode->value;
                }
                upperIndexNode->ele.data.id.type = (char *)malloc(sizeof(char)*11);
                strcpy(upperIndexNode->ele.data.id.type, "INTEGER\0");
                upperIndexNode->aux = 0;
                upperIndexNode->lineNum = traveller->sibling->child->child->sibling->node->ele.leafNode->lineNum;
                upperIndexNode->next = NULL;

                int tmp=0;
                if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"INTEGER"))
                    tmp = INTEGER_SIZE;
                else if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"REAL"))
                    tmp = REAL_SIZE;
                else if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"BOOLEAN"))
                    tmp = BOOLEAN_SIZE;

               if( (!strcmp(node->ele.data.arr.lowerIndex->type,"ID")) 
               || (!strcmp(node->ele.data.arr.upperIndex->type,"ID")) )
                {
                    //dynamic array
                    node->ele.data.arr.isDynamic = 1;   
                }
                else    
                {
                    //static array
                    node->ele.data.arr.isDynamic = 0;
                }
                node->width = POINTER_SIZE;
                node->offset = currentOffset;
                currentOffset += POINTER_SIZE;

                lowerIndexNode->width = INTEGER_SIZE;
                lowerIndexNode->offset = currentOffset;
                currentOffset += lowerIndexNode->width;

                upperIndexNode->width = INTEGER_SIZE;
                upperIndexNode->offset = currentOffset;
                currentOffset += upperIndexNode->width;

                lowerIndexNode->isParameter = 2;
                upperIndexNode->isParameter = 2;
                node->isParameter = 1;
                node->next = NULL;

                node->offset = dummyCurrentOffset;
                node->dummyWidth = (2*DUMMY_INTEGER_SIZE) + DUMMY_POINTER_SIZE;
                dummyCurrentOffset += node->dummyWidth;
                sym_hash_insert(lowerIndexNode, &(moduleST->hashtb));
                sym_hash_insert(upperIndexNode, &(moduleST->hashtb));
            }
            //Its an ID
            else
            {
                node->ele.tag = Identifier;
                node->ele.data.id.lexeme = traveller->node->ele.leafNode->lexeme;
                node->ele.data.id.type = traveller->sibling->node->ele.leafNode->type;
                node->ele.data.id.value = traveller->node->ele.leafNode->value;
                node->lineNum = traveller->node->ele.leafNode->lineNum;
                
                if(!strcmp(traveller->sibling->node->ele.leafNode->type,"INTEGER"))
                {
                    node->width = INTEGER_SIZE;
                    node->dummyWidth = DUMMY_INTEGER_SIZE;
                }
                else if(!strcmp(traveller->sibling->node->ele.leafNode->type,"REAL"))
                {
                    node->width = REAL_SIZE;
                    node->dummyWidth = DUMMY_REAL_SIZE;
                }
                else if(!strcmp(traveller->sibling->node->ele.leafNode->type,"BOOLEAN"))
                {
                    node->width = BOOLEAN_SIZE;
                    node->dummyWidth = DUMMY_REAL_SIZE;
                }
                    
                node->next = NULL;
                node->offset = currentOffset;
                currentOffset += node->width;
                node->isParameter = 1;

                node->dummyOffset = dummyCurrentOffset;
                dummyCurrentOffset += node->dummyWidth;
            }
            newNode->ele.data.mod.outputList[i] = node->ele;
            symbolTableNode *ret = sym_hash_insert(node, &(moduleST->hashtb));
            if(ret != NULL)
            {
                char *err = (char *)malloc(sizeof(char)*250);
                char *whatType = (char *)malloc(sizeof(char)*10);
                char *lexeme = NULL, *type = NULL;
                int lineNum;

                lineNum = ret->lineNum;
                if(ret->ele.tag == Identifier)
                {
                    lexeme = ret->ele.data.id.lexeme;
                    type = ret->ele.data.id.type;  
                    strcpy(whatType, "ID");
                }
                else if(ret->ele.tag == Array)
                {
                    lexeme = ret->ele.data.arr.lexeme;
                    type = ret->ele.data.arr.type;
                    strcpy(whatType, "ARRAY");
                }
                else if(ret->ele.tag == Module)
                {
                    lexeme = ret->ele.data.mod.lexeme;
                    type = "Module\0";
                    strcpy(whatType, "MODULE");
                }
                sprintf(err, "Line %d: %s (%s, %s) variable is already declared at %d. Redeclaration of %s at %d", 
                traveller->node->ele.leafNode->lineNum, lexeme, whatType, type, lineNum, lexeme, traveller->node->ele.leafNode->lineNum);
                semanticErrorNode *errNode = (semanticErrorNode *)malloc(sizeof(semanticErrorNode));
                errNode->errorMessage = err;
                errNode->next = NULL;
                insertSemError(errNode);
                // if(ret!=NULL) free(ret);
            } 
            //skip two at a time cause we have [ ID -> Datatype -> ID -> DataType .... ]
            traveller = traveller->sibling->sibling;
            i++;
        }

        newNode->lineNum = astRoot->child->node->ele.leafNode->lineNum;
        newNode->next = NULL;
        
        //current is actually the Program symbol table
        symbolTableNode * ret = sym_hash_insert(newNode, &(current->hashtb));
        if(ret != NULL)
        {
            char *err = (char *)malloc(sizeof(char)*250);
            char *whatType = (char *)malloc(sizeof(char)*10);
            char *lexeme = NULL, *type = NULL;
            int lineNum;

            lineNum = ret->lineNum;
            if(ret->ele.tag == Identifier)
            {
                lexeme = ret->ele.data.id.lexeme;
                type = ret->ele.data.id.type;  
                strcpy(whatType, "ID");
            }
            else if(ret->ele.tag == Array)
            {
                lexeme = ret->ele.data.arr.lexeme;
                type = ret->ele.data.arr.type;
                strcpy(whatType, "ARRAY");
            }
            else if(ret->ele.tag == Module)
            {
                lexeme = ret->ele.data.mod.lexeme;
                type = "Module\0";
                strcpy(whatType, "MODULE");
            }
            sprintf(err, "Line %d: %s (%s) procedure is already defined at %d. Redefinition of %s at %d", 
            newNode->lineNum, lexeme, whatType, lineNum, lexeme, newNode->lineNum);
            semanticErrorNode *errNode = (semanticErrorNode *)malloc(sizeof(semanticErrorNode));
            errNode->errorMessage = err;
            errNode->next = NULL;
            insertSemError(errNode);
            // if(ret!=NULL) free(ret);
        }
        
        moduleST->currentOffset = currentOffset;
        moduleST->dummyCurrentOffset = dummyCurrentOffset;
        currentOffset = 0;
        dummyCurrentOffset = 0;
        formulation(astRoot->child, moduleST);
        formulation(astRoot->child->sibling, moduleST);
        formulation(astRoot->child->sibling->sibling, moduleST);
        formulation(astRoot->child->sibling->sibling->sibling, moduleST);

        
        //linking the symbols tables
        symbolTable *tmp = current->child;
        if(tmp == NULL)
        {
            current->child = moduleST;
        }
        else
        {
            while(tmp->sibling != NULL)
            {
                tmp = tmp->sibling;
            }
            tmp->sibling = moduleST;
        }
    }
    else if(!strcmp(astRoot->node->ele.internalNode->label, "MODULEDEF"))
    {
        char *str = (char *)malloc(sizeof(char)*25); 
        sprintf(str, "%s",current->symLexeme);
        
        symbolTable* moduledefST = initializeSymbolTable(str,astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
        currentOffset = 0;
        dummyCurrentOffset = 0;
        //call the formulation on all children one by one
        //children are parallel statements
        astNode* trav = astRoot->child;
        while(trav!=NULL)
        {
            formulation(trav,moduledefST);
            trav=trav->sibling;
        }
        moduledefST->currentOffset = currentOffset;
        moduledefST->dummyCurrentOffset = dummyCurrentOffset;
        //linking of symbols tables
        symbolTable *tmp = current->child;
        if(tmp == NULL)
        {
            current->child = moduledefST;
        }
        else
        {
            while(tmp->sibling != NULL)
            {
                tmp = tmp->sibling;
            }
            tmp->sibling = moduledefST;
        }
        currentOffset = 0;
    }

    else if(!strcmp(astRoot->node->ele.internalNode->label, "DRIVER"))
    {
        char *str = (char*)malloc(sizeof(char)*10); strcpy(str,"Driver");
        symbolTable * driverST = initializeSymbolTable(str,astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
        
        currentOffset = 0;
        dummyCurrentOffset = 0;
        formulation(astRoot->child,driverST);
        //linking of symbols tables
        symbolTable *tmp = current->child;
        if(tmp == NULL)
        {
            current->child = driverST;
        }
        else
        {
            while(tmp->sibling != NULL)
            {
                tmp = tmp->sibling;
            }
            tmp->sibling = driverST;
        }   

        currentOffset = 0; 
    }
    else if(!strcmp(astRoot->node->ele.internalNode->label, "WHILE"))
    {
        char * str = (char *)malloc(sizeof(char)*(strlen(current->symLexeme)));
        memset(str, '\0', sizeof(char)*(strlen(str)));
        sprintf(str, "%s",current->symLexeme);
        symbolTable *whileST = initializeSymbolTable(str, astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
        // whileST->currentOffset = currentOffset;

        astNode *trav = astRoot->child->sibling;
        while(trav != NULL)
        {
            formulation(trav, whileST);
            trav = trav->sibling;
        }

        //linking of symbols tables
        symbolTable *tmp = current->child;
        if(tmp == NULL)
        {
            current->child = whileST;
        }
        else
        {
            while(tmp->sibling != NULL)
            {
                tmp = tmp->sibling;
            }
            tmp->sibling = whileST;
        }    
    }
    else if(!strcmp(astRoot->node->ele.internalNode->label, "FOR"))
    {
        char * str = (char *)malloc(sizeof(char)*(strlen(current->symLexeme)));
        memset(str, '\0', sizeof(char)*(strlen(str)));
        sprintf(str, "%s",current->symLexeme);
        symbolTable *forST = initializeSymbolTable(str, astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
        // forST->currentOffset = currentOffset;

        //call the formulation on all statements 
        astNode *trav = astRoot->child->sibling->sibling;
        while(trav != NULL)
        {
            formulation(trav, forST);
            trav = trav->sibling;
        }
        
        //linking of symbols tables
        symbolTable *tmp = current->child;
        if(tmp == NULL)
        {
            current->child = forST;
        }
        else
        {
            while(tmp->sibling != NULL)
            {
                tmp = tmp->sibling;
            }
            tmp->sibling = forST;
        }    

    }
    else if(!strcmp(astRoot->node->ele.internalNode->label, "SWITCH"))
    {
        //ID_node (switch var)
        //caseStmts
                    //ID, stmts
        //default
        char * str = (char *)malloc(sizeof(char)*(strlen(current->symLexeme)));
        memset(str, '\0', sizeof(char)*(strlen(str)));
        sprintf(str, "%s",current->symLexeme);
        symbolTable *switchST = initializeSymbolTable(str, astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
        // switchST->currentOffset = currentOffset;
        
        formulation(astRoot->child->sibling, switchST);

        formulation(astRoot->child->sibling->sibling, switchST);

        //linking of symbols tables
        symbolTable *tmp = current->child;
        if(tmp == NULL)
        {
            current->child = switchST;
        }
        else
        {
            while(tmp->sibling != NULL)
            {
                tmp = tmp->sibling;
            }
            tmp->sibling = switchST;
        }    
    }
    else
    {
        astNode *temp = astRoot->child;
        while(temp!=NULL)
        {
            formulation(temp,current);
            temp = temp->sibling;                  
        }
    }
}

void printSymbolTable(symbolTable *root, int level)
{
    if(root == NULL)
        return;

    // printf("***********************************************************************************************************************************\n");
    printSymTableNode(root,level);
    // printf("***********************************************************************************************************************************\n\n");
    symbolTable * tmp = root->child;
    while(tmp != NULL)
    {
        printSymbolTable(tmp, level+1);
        tmp = tmp->sibling;
    }
}

void activationRecordSize(symbolTable *root, int level)
{
    if(root == NULL)
        return;

    if(level == 2)
    // if((level==1 && (strcmp(root->symLexeme,"Module Declarations"))) 
    // || (level==2 && (!strcmp(root->symLexeme,"Driver"))))
    {
        printf("|| Function: %-20s Activation Size: %-10d || Level %d \n", root->symLexeme, root->dummyCurrentOffset, level);
    }
    symbolTable * tmp = root->child;
    while(tmp != NULL)
    {
        activationRecordSize(tmp, level+1);
        tmp = tmp->sibling;
    }
}

void printSymTableNode(symbolTable *symT, int level)
{
    if(symT == NULL)
        return;

    // printf("Name of Table: %s \n", symT->symLexeme);
    // printf("Start line of scope: %d , ",symT->lineNumStart);
    // printf("End line of scope: %d \n",symT->lineNumEnd);
    printHashTable(symT, level,symT->symLexeme,symT->lineNumStart,symT->lineNumEnd );
}

void printHashTable(symbolTable* symT, int level, char* scopename, int linestart, int lineend)
{
    
    int index = 0;
    hashSym hashtb = symT->hashtb;
    while(index<=hashtb.hashtbSize)
    {        
        linkedListSym* trav = &(hashtb.arr[index]);
        symbolTableNode *temp = trav->head;
        while(temp!=NULL)
        {
            if(temp->isParameter == 2 || temp->ele.tag==Module)
            {
                temp = temp->next;
                continue;
            }

            char variable[20];
            char scopeline[20];
            char isarray[10];
            char static_dynamic[20];
            char range[20];
            char type[20];

            
            if(temp->ele.tag==Identifier)
            {
                strcpy(variable,temp->ele.data.id.lexeme);
                if(temp->isParameter==1 || temp->isParameter==2)
                    sprintf(scopeline,"%d to %d",symT->child->lineNumStart, symT->child->lineNumEnd);
                else 
                    sprintf(scopeline,"%d to %d",linestart, lineend);
                strcpy(isarray,"NO");
                strcpy(static_dynamic," --- ");
                strcpy(range," --- ");
                strcpy(type,temp->ele.data.id.type);
                
            }
            else if(temp->ele.tag==Array)
            {
                strcpy(variable,temp->ele.data.arr.lexeme);

                if(temp->isParameter==1 || temp->isParameter==2)
                    sprintf(scopeline,"%d to %d",symT->child->lineNumStart, symT->child->lineNumEnd);
                else 
                    sprintf(scopeline,"%d to %d",linestart, lineend);
                strcpy(isarray,"YES");

                if(temp->ele.data.arr.isDynamic==0)
                {
                    strcpy(static_dynamic," Static ");
                    
                }
                else 
                {
                    strcpy(static_dynamic," Dynamic ");
                }

                sprintf(range,"[%s - %s]", temp->ele.data.arr.lowerIndex->lexeme, temp->ele.data.arr.upperIndex->lexeme);
                
                strcpy(type,temp->ele.data.arr.type);
            }
            else if(temp->ele.tag==Module)
            {
                strcpy(variable,temp->ele.data.mod.lexeme);
                sprintf(scopeline,"%d to %d",linestart, lineend);
                strcpy(isarray,"NO");
                strcpy(static_dynamic," --- ");
                strcpy(range," --- ");
                strcpy(type,"Module");
            }

            printf("|| %-20s | %-20s | %-15s | %-8d | %-8s | %-15s | %-15s | %-10s | %-8d | %-10d ||\n", 
            variable, scopename,scopeline, temp->dummyWidth, isarray, 
            static_dynamic, range, type, temp->dummyOffset, level);
    

            printf("\n");
            temp = temp->next;
        }
        index++;
    }
}

void printSemanticErrors()
{
    semanticErrorNode *trav = semErrorList->head;
    while(trav!=NULL)
    {
        printf("%s\n",trav->errorMessage);
        trav = trav->next;
    }
}