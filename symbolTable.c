#include "symbolTable.h"
#include "ast.h"
#include "parser.h"

void sympush(tableStack *stack, tableStackEle *newNode)
{
    if(stack->top == NULL)
    {
        stack->size = 1;
        stack->top = newNode;
        stack->bottom = newNode;
    }

    newNode->next = stack->top;
    stack->top = newNode;
    stack->size ++;
}

tableStackEle * sympop(tableStack *stack)
{
    if(stack->size != 0)
    {
        tableStackEle * temp = stack->top;
        stack->top = stack->top->next;
        stack->size--;

        if(stack->size==0)
            stack->bottom = NULL;

        return temp;
    }
    else
      return NULL;
}

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

//used to find the enumeration hash table for terminals and non terminals
symbolTableNode* sym_hash_find(char * str, hashSym * hash_tb)
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
		//looking at an array
        else if (trav->ele.tag == Array)
            lexeme = trav->ele.data.arr.lexeme;
        //looking at a module
        else if(trav->ele.tag == Module)
            lexeme = trav->ele.data.mod.lexeme;
		//return the appropriate node
        if(strcmp(lexeme,str) == 0)
            return &trav;
		
		//keep searching
        trav = trav->next;
    }

	//if not present, returns NULL
    return NULL;
}

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
    free(oldTable->arr);
    free(oldTable);
    return newTable;    
}

//common for both hash tables of keywords and TorNT
void sym_hash_insert(symbolTableNode * newNode, hashSym * hash_tb)
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
    if(sym_hash_find(str, hash_tb) != NULL)
        return;
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
    return ;
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

symbolTable* intializeSymbolTable(char *str, int lineNumStart, int lineNumEnd)
{
    symbolTable * ST = (symbolTable*)malloc(sizeof(symbolTable));
    ST->child = NULL;
    ST->sibling = NULL;
    ST->symLexeme = str;
    ST->lineNumStart = lineNumStart;
    ST->lineNumEnd = lineNumEnd;
    hashSym *hashtb = (hashSym*)malloc(sizeof(hashSym));
    intializeHashSym(hashtb);
    ST->hashtb = *hashtb;
    return ST;    
}

void formulation(astNode* astRoot, symbolTable * current)
{

    if(!strcmp(astRoot->node->ele.internalNode->label, "PROGRAM"))
    {
        char *str = (char*)malloc(sizeof(char)*8); strcpy(str,"Program");
        symbolTable *programST = intializeSymbolTable(str, astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
        formulation(astRoot->child, programST);
        formulation(astRoot->child->sibling, programST);
        formulation(astRoot->child->sibling->sibling, programST);
        formulation(astRoot->child->sibling->sibling->sibling, programST);

        // Pass the Root Symbol Table Node
        // Base case
        symbolTableRoot = programST;
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
            if(type->node->tag == Internal)
            {
                newNode->ele.tag = Array;
                newNode->ele.data.arr.lexeme = idlist->node->ele.leafNode->lexeme;
                newNode->ele.data.arr.type = type->child->sibling->node->ele.leafNode->type;

                newNode->ele.data.arr.lowerIndex->lexeme = type->child->child->node->ele.leafNode->lexeme;
                newNode->ele.data.arr.lowerIndex->type = type->child->child->node->ele.leafNode->type;
                newNode->ele.data.arr.lowerIndex->value = type->child->child->node->ele.leafNode->value;

                newNode->ele.data.arr.upperIndex->lexeme = type->child->child->sibling->node->ele.leafNode->lexeme;
                newNode->ele.data.arr.upperIndex->type = type->child->child->sibling->node->ele.leafNode->type;
                newNode->ele.data.arr.upperIndex->value = type->child->child->sibling->node->ele.leafNode->value;

                newNode->lineNum = idlist->node->ele.leafNode->lineNum;

                if(!strcmp(type->child->sibling->node->ele.leafNode->type,"INTEGER"))
                    newNode->width = 2;
                else if(!strcmp(type->child->sibling->node->ele.leafNode->type,"REAL"))
                    newNode->width = 4;
                else if(!strcmp(type->child->sibling->node->ele.leafNode->type,"BOOLEAN"))
                    newNode->width = 1;
                
                newNode->next = NULL;
                sym_hash_insert(newNode, &(current->hashtb));
            }
            //Its an ID
            else
            {
                newNode->ele.tag = Identifier;
                newNode->ele.data.id.lexeme = type->node->ele.leafNode->lexeme;
                newNode->ele.data.id.type = type->node->ele.leafNode->type;
                newNode->ele.data.id.type = type->node->ele.leafNode->value;
                newNode->lineNum = idlist->node->ele.leafNode->lineNum;
                
                if(!strcmp(type->node->ele.leafNode->type,"INTEGER"))
                    newNode->width = 2;
                else if(!strcmp(type->node->ele.leafNode->type,"REAL"))
                    newNode->width = 4;
                else if(!strcmp(type->node->ele.leafNode->type,"BOOLEAN"))
                    newNode->width = 1;
                    
                newNode->next = NULL;
                sym_hash_insert(newNode, &(current->hashtb));
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
            newNode->ele.tag = Module;
            newNode->ele.data.mod.lexeme = trav->node->ele.leafNode->lexeme;
            newNode->ele.data.mod.inputcount = 0;
            newNode->ele.data.mod.outputcount = 0;
            newNode->ele.data.mod.inputList = NULL;
            newNode->ele.data.mod.outputList = NULL;
            newNode->lineNum = trav->node->ele.leafNode->lineNum;
            newNode->next = NULL; 
            sym_hash_insert(newNode, &(moduleDecST->hashtb));
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
        
            // arrASTnodes[0] = ID_node;
            // arrASTnodes[1] = inpList_node;
            // arrASTnodes[2] = ret;
            // arrASTnodes[3] = modDef_node;

        formulation(astRoot->child,moduleST);
        formulation(astRoot->child->sibling,moduleST);
        formulation(astRoot->child->sibling->sibling,moduleST);
        formulation(astRoot->child->sibling->sibling->sibling,moduleST);

        symbolTableNode *newNode = (symbolTableNode*)malloc(sizeof(symbolTableNode));
        newNode->ele.tag = Module;
        newNode->ele.data.mod.lexeme = astRoot->child->node->ele.leafNode->lexeme;
        
        astNode *traveller = astRoot->child->sibling->child;
        while(traveller!=NULL)
        {
            newNode->ele.data.mod.inputcount++;
            traveller=traveller->sibling;
        }

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

        traveller = astRoot->child->sibling->child;
        int i = 0;
        while(traveller!=NULL)
        {
            symbolTableNode *node = (symbolTableNode*)malloc(sizeof(symbolTableNode));
            if(traveller->sibling->node->tag == Internal)
            {
                node->ele.tag = Array;
                node->ele.data.arr.lexeme = traveller->node->ele.leafNode->lexeme;
                node->ele.data.arr.type = traveller->sibling->child->sibling->node->ele.leafNode->type;

                node->ele.data.arr.lowerIndex->lexeme = traveller->sibling->child->child->node->ele.leafNode->lexeme;
                node->ele.data.arr.lowerIndex->type = traveller->sibling->child->child->node->ele.leafNode->type;
                node->ele.data.arr.lowerIndex->value = traveller->sibling->child->child->node->ele.leafNode->value;

                node->ele.data.arr.upperIndex->lexeme = traveller->sibling->child->child->sibling->node->ele.leafNode->lexeme;
                node->ele.data.arr.upperIndex->type = traveller->sibling->child->child->sibling->node->ele.leafNode->type;
                node->ele.data.arr.upperIndex->value = traveller->sibling->child->child->sibling->node->ele.leafNode->value;

                node->lineNum = traveller->node->ele.leafNode->lineNum;

                if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"INTEGER"))
                    node->width = 2;
                else if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"REAL"))
                    node->width = 4;
                else if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"BOOLEAN"))
                    node->width = 1;
                
                node->next = NULL;
                sym_hash_insert(node, &(moduleST->hashtb));
            }
            //Its an ID
            else
            {
                node->ele.tag = Identifier;
                node->ele.data.id.lexeme = traveller->sibling->node->ele.leafNode->lexeme;
                node->ele.data.id.type = traveller->sibling->node->ele.leafNode->type;
                node->ele.data.id.type = traveller->sibling->node->ele.leafNode->value;
                node->lineNum = traveller->node->ele.leafNode->lineNum;
                
                if(!strcmp(traveller->sibling->node->ele.leafNode->type,"INTEGER"))
                    node->width = 2;
                else if(!strcmp(traveller->sibling->node->ele.leafNode->type,"REAL"))
                    node->width = 4;
                else if(!strcmp(traveller->sibling->node->ele.leafNode->type,"BOOLEAN"))
                    node->width = 1;
                    
                node->next = NULL;
                sym_hash_insert(node, &(moduleST->hashtb));
            }
            newNode->ele.data.mod.inputList[i] = node->ele;
            //skip two at a time cause we have [ ID -> Datatype -> ID -> DataType .... ]
            traveller = traveller->sibling->sibling;
            i++;
        }

        traveller = astRoot->child->sibling->sibling->child;
        i = 0;
        while(traveller!=NULL)
        {
            symbolTableNode *node = (symbolTableNode*)malloc(sizeof(symbolTableNode));
            if(traveller->sibling->node->tag == Internal)
            {
                node->ele.tag = Array;
                node->ele.data.arr.lexeme = traveller->node->ele.leafNode->lexeme;
                node->ele.data.arr.type = traveller->sibling->child->sibling->node->ele.leafNode->type;

                node->ele.data.arr.lowerIndex->lexeme = traveller->sibling->child->child->node->ele.leafNode->lexeme;
                node->ele.data.arr.lowerIndex->type = traveller->sibling->child->child->node->ele.leafNode->type;
                node->ele.data.arr.lowerIndex->value = traveller->sibling->child->child->node->ele.leafNode->value;

                node->ele.data.arr.upperIndex->lexeme = traveller->sibling->child->child->sibling->node->ele.leafNode->lexeme;
                node->ele.data.arr.upperIndex->type = traveller->sibling->child->child->sibling->node->ele.leafNode->type;
                node->ele.data.arr.upperIndex->value = traveller->sibling->child->child->sibling->node->ele.leafNode->value;

                node->lineNum = traveller->node->ele.leafNode->lineNum;

                if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"INTEGER"))
                    node->width = 2;
                else if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"REAL"))
                    node->width = 4;
                else if(!strcmp(traveller->sibling->child->sibling->node->ele.leafNode->type,"BOOLEAN"))
                    node->width = 1;
                
                node->next = NULL;
                sym_hash_insert(node, &(moduleST->hashtb));
            }
            //Its an ID
            else
            {
                node->ele.tag = Identifier;
                node->ele.data.id.lexeme = traveller->sibling->node->ele.leafNode->lexeme;
                node->ele.data.id.type = traveller->sibling->node->ele.leafNode->type;
                node->ele.data.id.type = traveller->sibling->node->ele.leafNode->value;
                node->lineNum = traveller->node->ele.leafNode->lineNum;
                
                if(!strcmp(traveller->sibling->node->ele.leafNode->type,"INTEGER"))
                    node->width = 2;
                else if(!strcmp(traveller->sibling->node->ele.leafNode->type,"REAL"))
                    node->width = 4;
                else if(!strcmp(traveller->sibling->node->ele.leafNode->type,"BOOLEAN"))
                    node->width = 1;
                    
                node->next = NULL;
                sym_hash_insert(node, &(moduleST->hashtb));
            }
            newNode->ele.data.mod.outputList[i] = node->ele;
            //skip two at a time cause we have [ ID -> Datatype -> ID -> DataType .... ]
            traveller = traveller->sibling->sibling;
            i++;
        }

        newNode->lineNum = astRoot->child->node->ele.leafNode->lineNum;
        newNode->next = NULL;
        
        sym_hash_insert(newNode, &(current->hashtb));

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
    else if(!strcmp(astRoot->node->ele.internalNode->label, "DRIVER"))
    {
        char *str = (char*)malloc(sizeof(char)*10); strcpy(str,"Driver");
        symbolTable * driverST = initializeSymbolTable(str,astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
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
        
    }
    else if(!strcmp(astRoot->node->ele.internalNode->label, "WHILE"))
    {
        char * str = (char *)malloc(sizeof(char)*(strlen(current->symLexeme)+10));
        memset(str, '\0', sizeof(char)*(strlen(str)));
        sprintf(str, "%s_WHILE",current->symLexeme);
        symbolTable *whileST = intializeSymbolTable(str, astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
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
        char * str = (char *)malloc(sizeof(char)*(strlen(current->symLexeme)+10));
        memset(str, '\0', sizeof(char)*(strlen(str)));
        sprintf(str, "%s_FOR",current->symLexeme);
        symbolTable *forST = intializeSymbolTable(str, astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
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
    else if(!strcmp(astRoot->node->ele.internalNode->label, "CASE"))
    {
        char * str = (char *)malloc(sizeof(char)*(strlen(current->symLexeme)+10));
        memset(str, '\0', sizeof(char)*(strlen(str)));
        sprintf(str, "%s_CASE",current->symLexeme);
        symbolTable *caseST = intializeSymbolTable(str, astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);
        
        astNode *trav = astRoot->child->sibling;
        while(trav != NULL)
        {
            formulation(trav, caseST);
            trav = trav->sibling;
        }

        //linking of symbols tables
        symbolTable *tmp = current->child;
        if(tmp == NULL)
        {
            current->child = caseST;
        }
        else
        {
            while(tmp->sibling != NULL)
            {
                tmp = tmp->sibling;
            }
            tmp->sibling = caseST;
        }    
    }
    else if(!strcmp(astRoot->node->ele.internalNode->label, "DEFAULT"))
    {
        char * str = (char *)malloc(sizeof(char)*(strlen(current->symLexeme)+10));
        memset(str, '\0', sizeof(char)*(strlen(str)));
        sprintf(str, "%s_DEFAULT",current->symLexeme);
        symbolTable *defST = intializeSymbolTable(str, astRoot->node->ele.internalNode->lineNumStart, astRoot->node->ele.internalNode->lineNumEnd);

        astNode *trav = astRoot->child;
        while(trav != NULL)
        {
            formulation(trav, defST);
            trav = trav->sibling;
        }

        //linking of symbols tables
        symbolTable *tmp = current->child;
        if(tmp == NULL)
        {
            current->child = defST;
        }
        else
        {
            while(tmp->sibling != NULL)
            {
                tmp = tmp->sibling;
            }
            tmp->sibling = defST;
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
    
    //Traverse all children

    //Add cases for specific AST nodes here
    
}

/*
    
    Symbol table to filled for following AST Node
        DECLARE - insert into current symbol table (top of stack)
        MODULEDEC - global insert 
        MODULE - 
        DRIVER - 
        MODULEDEF - 
        PROGRAM - create
        WHILE - create
        FOR - create
        SWITCH - create
*/

