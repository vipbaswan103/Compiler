//return the hash value of the str in the hash table 
int tmp_hash_func(hashSym *hashtb,char *str)
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
    symbolTableNode *find = sym_hash_find(str, hash_tb, 1, newNode);
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



void nasmCode()
{
    // if(arg1 find in synmol table) load AX, arg1
    // else MOV AX, arg1
    // if(arg2 find in synmol table)  load BX, arg2
    // else MOV BX, arg2
    // ADD CX,BX, AX
    // STORE result, CX
}