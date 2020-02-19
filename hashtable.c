#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define HASHSIZE 1000

int enumTerminal=0, enumNonTerminal=0, curr_size = 10, epsilonENUM;

typedef struct{
    char str[25];
    int enumcode;
} Terminal;

typedef struct{
    char str[25];
    int enumcode;
} NonTerminal;

typedef union{
        Terminal t;
        NonTerminal nt;
} TokenType; 

//changable element
typedef struct{
    int tag;
    TokenType type;
}Element;

//node of the hash chain
typedef struct node
{
    Element ele ;
    struct node * next ;
}Node;

// base list
typedef struct{
    Node * head;
    Node * tail;
    int size ;
}LinkedList;

//the head of the linked list
typedef struct
{
    LinkedList arr[HASHSIZE] ;
}Hashtable;

//finding the hash of the string
int hash_func(char *str)
{
    int i = 0 ;
    long long val = 11 ;
    while(str[i]!='\0')
    {
        val = val*31 + str[i] ;
        i++ ;
    }
    return val % HASHSIZE ;
}


Element* hash_find(char * str, Hashtable * hash_tb)
{
    int hash;
    hash = hash_func(str);
    
    Node *trav = hash_tb->arr[hash].head; 
    char * lexeme;
    while(trav != NULL)
    {
        //write the logic for the comparison based on what we decide
        if(trav->ele.tag==1)
            lexeme = trav->ele.type.nt.str;
        else if (trav->ele.tag==2)
            lexeme = trav->ele.type.t.str;
        if(strcmp(lexeme,str) == 0)
            return &(trav->ele);
        trav = trav->next;
    }
    return NULL;
}

void hash_insert(Element * ele, Hashtable * hash_tb)
{
    char * str;
    if(ele->tag == 1)
        str = ele->type.nt.str;
    else
        str = ele->type.t.str;

    if(hash_find(str,hash_tb) != NULL)
        return; 

    int hash;
    
    if(ele->tag==1)
        hash= hash_func(ele->type.nt.str);
    else if (ele->tag==2)
        hash= hash_func(ele->type.t.str);

    Node *temp = (Node*)malloc(sizeof(Node)) ;
    temp->ele = *ele ;
    temp->next = NULL ;

    if( hash_tb->arr[hash].head == NULL)
    {
        hash_tb->arr[hash].head = temp ;
        hash_tb->arr[hash].tail = temp ;
        hash_tb->arr[hash].size = 1 ;
    }
    else
    {
        hash_tb->arr[hash].tail->next = temp ;
        hash_tb->arr[hash].tail = temp ;
        hash_tb->arr[hash].size++ ;
    }
    
    return ;
}

typedef struct
{
    int size;
    LinkedList * arr;
}Grammar;

void populateGrammarArray(Grammar * grammar, Hashtable * hash_tb, char * str, int TorNT, int index)
{
    Element * ele = hash_find(str, hash_tb);

    Node * newHead = (Node *)malloc(sizeof(Node));
    Node * newTail = (Node *)malloc(sizeof(Node));
    
    if(ele != NULL)     //Is in the hash table
    {
        grammar->size += 1;
        newHead->ele = *ele;
        newHead->next = NULL;
        newTail = newHead;
        LinkedList list;
        list.head = newHead;
        list.tail = newTail;
        list.size = 0;
        grammar->arr[index] = list;
    }
    else    //Isn't in the hash table
    {
        grammar->size += 1;
        
        TokenType type;
        NonTerminal nt;
        
        nt.enumcode = enumNonTerminal;
        enumNonTerminal++;
        strcpy(nt.str, str);
        type.nt = nt;

        newHead->ele.tag = 2;
        newHead->ele.type = type;
        newHead->next = NULL;
        newTail = newHead;

        LinkedList list;
        list.head = newHead;
        list.tail = newTail;
        list.size = 0;
        
        hash_insert(&(newHead->ele), hash_tb);
        grammar->arr[index] = list;
    }
}

void insertInLinkedList(Grammar * grammar, Hashtable *hash_tb, char * str, int TorNT, int index)
{
    Node * trav = grammar->arr[index].head;
    Node * tail = grammar->arr[index].tail;
    grammar->arr[index].size += 1;

    Node * newNode = (Node *)malloc(sizeof(Node));
    
    Element * ele = hash_find(str, hash_tb);
    
    tail->next = newNode;
    if(ele != NULL)     //Found in hash table, can be a T or a NT
    {
        newNode->ele = *ele;
        newNode->next = NULL;
        grammar->arr[index].tail = newNode;
    }
    else        //Not found in hash table, must be a T (since I have already populated hash table with all NTs)
    {
        TokenType type;
        Terminal t;
        Element ele;

        t.enumcode = enumTerminal;
        enumTerminal++;
        strcpy(t.str, str);
        type.t = t;
        
        if(strcmp(str, "EPSILON") == 0)
            epsilonENUM = t.enumcode;
        
        ele.tag = 2;
        ele.type = type;

        newNode->ele = ele;
        newNode->next = NULL;
        tail->next = newNode;
        grammar->arr[index].tail = tail->next;

        hash_insert(&ele, hash_tb);        
    }
}

Grammar * read_grammar(char * filename)
{
    Grammar * grammar = (Grammar *)malloc(sizeof(Grammar));
    grammar->size = 0;
    grammar->arr = NULL;
    
    FILE *fp = fopen(filename,"r");
    if(fp == NULL)
    {
        printf("Error in opening the grammar file\n");
        return NULL;
    }
    
    //first pass
    Hashtable *hash_tb;
    hash_tb = (Hashtable* )malloc(sizeof(Hashtable));
    
    for(int i=0;i<HASHSIZE;i++)
    {
        hash_tb->arr[i].head=NULL;
        hash_tb->arr[i].tail=NULL;
    }
    
    char * temp;
    int rulecount = 0;
    while(fscanf(fp,"%s%*[^\n]",temp)!=0)
    {
        populateGrammarArray(grammar,hash_tb, temp,1,rulecount);
        rulecount++;
        fgetc(fp);
    }
    grammar->size = rulecount;
    grammar->arr = (LinkedList*) malloc(sizeof(LinkedList)*rulecount);
    fseek(fp,0L,SEEK_SET);
        
    while(fscanf(fp,"%s%*[^\n]",temp)!=0)
    {
        populateGrammarArray(grammar, hash_tb, temp, 1, rulecount);
        rulecount++;
        fgetc(fp);
    }


    //second pass
    fseek(fp,0L,SEEK_SET);
    
    int rulenum = 0;
    int linestart=1;
    while(fscanf(fp,"%[^\n ]",temp)!=0)
    {
        if(linestart==1)
        {
            linestart=0;
            fgetc(fp);
        }    
        else
        {
            //give temp to vipin
            //check that it is a non terminal or not
            Element *ele = hash_find(temp, hash_tb);
            if(ele==NULL)
            {
                insertInLinkedList(grammar, hash_tb, temp, 2, rulenum);
            }
            else
            {
                if(ele->tag == 1)
                    insertInLinkedList(grammar, hash_tb, temp, 1, rulenum);
                else if(ele->tag == 2)
                    insertInLinkedList(grammar, hash_tb, temp, 2, rulenum);
            }
            if(fgetc(fp)=='\n')
            {
                linestart=1;
                rulenum++;    
            }
        }
        
    }  
    return grammar;
}

void printGrammar(Grammar * grammar)
{
    Node * trav = NULL;
    for(int i=0; i<grammar->size; i++)
    {
        trav = grammar->arr[i].head;

        while(trav->next != NULL)
        {
            if(trav->ele.tag == 1)
                printf("%s (%d) -> ",trav->ele.type.nt.str, trav->ele.type.nt.enumcode);
            else if(trav->ele.tag == 2)
                printf("%s (%d) -> ",trav->ele.type.t.str, trav->ele.type.t.enumcode);
            
            trav = trav->next;
        }
        if(trav->ele.tag == 1)
                printf("%s (%d)\n",trav->ele.type.nt.str, trav->ele.type.nt.enumcode);
        else if(trav->ele.tag == 2)
                printf("%s (%d)\n",trav->ele.type.t.str, trav->ele.type.t.enumcode);
    }
}

int main(int argc, char * argv[])
{
    if(argc != 2)
    {
        printf("Wrong number of args\n");
    }

    Grammar * grammar = read_grammar(argv[1]);

    printGrammar(grammar);
}

int ** initializeFirst()
{
    int ** firstSet = (int **)malloc(sizeof(int*)*enumNonTerminal);
    for(int i=0; i<enumNonTerminal; i++)
    {
        firstSet[i] = (int *)malloc(sizeof(int)*(enumTerminal+3));
        memset(firstSet[i],0,sizeof(int)*(enumTerminal+3));
        firstSet[i][enumTerminal+1] = -1;  //Indicates whether we have calculated follow set or not, special
        firstSet[i][enumTerminal+2] = -1;  //Indicates whether we have traversed this NT before or not, special
    }
    return firstSet;
}


int * calculateFirstSet(Grammar *grammar, int nonTerminal, int **firstSet)
{
    if(firstSet[nonTerminal][enumTerminal+1]==1)
        return firstSet[nonTerminal];
    if(firstSet[nonTerminal][enumTerminal+2]==1)
    {
        int * arr = (int *)malloc(sizeof(int)*(enumTerminal+3));
        memset(arr, 0, sizeof(int)*(enumTerminal+3));
        return arr;
    }    

    for(int i = 0 ; i < grammar->size ; i++)
    {
        Node *trav = grammar->arr[i].head;
        if(trav->ele.type.nt.enumcode == nonTerminal)
        {
            while(trav!=NULL)
            {
                if(trav->next->ele.tag == 2) // A->XB and X is a Terminal 
                {
                    firstSet[nonTerminal][trav->next->ele.type.t.enumcode] = 1;
                    break;
                }
                else //A->XB and X is a NonTerminal
                {
                    firstSet[nonTerminal][enumTerminal+2] = 1;
                    int *arr = calculateFirstSet(grammar, trav->next->ele.type.nt.enumcode, firstSet);
                    setOR(firstSet[nonTerminal], arr);
                    if(arr[epsilonENUM])
                    {
                        trav = trav->next;
                    }
                    else
                    {
                        break;
                    }                   
                }
            }
            if(trav == NULL)
                firstSet[nonTerminal][epsilonENUM] = 1;
        }
    }
    firstSet[nonTerminal][enumTerminal+1] = 1;
    firstSet[nonTerminal][enumTerminal+2] = 1;
    return firstSet[nonTerminal];
}

int ** initializeFollow()
{
    int ** followSet = (int **)malloc(sizeof(int*)*enumNonTerminal);
    for(int i=0; i<enumNonTerminal; i++)
    {
        followSet[i] = (int *)malloc(sizeof(int)*(enumTerminal+3));
        memset(followSet[i],0,sizeof(int)*(enumTerminal+3));
        followSet[i][enumTerminal+1] = -1;  //Indicates whether we have calculated follow set or not, special
        followSet[i][enumTerminal+2] = -1;  //Indicates whether we have traversed this NT before or not, special
    }
    followSet[0][enumTerminal] = 1; //Puts '$' in the follow of start symbol
    return followSet;
}

void setOR(int * arr1, int * arr2)
{
    int tmp = arr1[epsilonENUM];
    for(int i=0; i<=enumTerminal; i++)
    {
        arr1[i] = (arr1[i]==1 || arr2[i]==1) ? 1 : 0;
    }
    arr1[epsilonENUM] = tmp;
}


//Basically a DFS implementation
/*For a NT, we have following cases:
    1) It's follow has been calculated.     followSet[NT][enumTerminal+1] = 1
                                            followSet[NT][enumTerminal+2] = 1

    2) It's follow set is being calculated and it has been traversed.   followSet[NT][enumTerminal+1] = -1
                                                                        followSet[NT][enumTerminal+2] = 1
    3) It's follow set is not yet calculated.   followSet[NT][enumTerminal+1] = -1
                                                followSet[NT][enumTerminal+2] = -1
*/
int * calculateFollowSet(Grammar * grammar, int nonTerminal, int ** followSet, int ** firstSet)
{
    if(followSet[nonTerminal][enumTerminal+1] == 1)    //We have already calculated the follow set
        return followSet[nonTerminal];
    
    if(followSet[nonTerminal][enumTerminal+2] == 1)     //Cycle detected
    {
        int * arr = (int *)malloc(sizeof(int)*(enumTerminal+3));
        memset(arr, 0, sizeof(int)*(enumTerminal+3));
        return arr;
    }
    
    for(int i=0; i<grammar->size; i++)
    {
        Node * trav = grammar->arr[i].head;
        while(trav != NULL)
        {
            if(trav->ele.tag == 1)
            {
                if(trav->ele.type.nt.enumcode == nonTerminal)   //We found the matching non-terminal
                {
                    if(trav->next == NULL)  //Rule is of type A -> XB
                    {
                        setOR(followSet[nonTerminal], calculateFollowSet(grammar, grammar->arr[i].head->ele.type.nt.enumcode, followSet, firstSet));
                    }
                    else    //Rule is of type A -> XBY
                    {
                        if(trav->next->ele.tag == 1)    //If Y is a terminal
                        {
                            followSet[nonTerminal][trav->next->ele.type.t.enumcode] = 1;
                        }
                        else   //If Y is a non-terminal
                        {
                            setOR(followSet[nonTerminal], firstSet[trav->next->ele.type.nt.enumcode]);
                            if (firstSet[trav->next->ele.type.nt.enumcode][epsilonENUM] == 1) //First set contains EPSILON
                            {
                                followSet[nonTerminal][enumTerminal+2] = 1;
                                setOR(followSet[nonTerminal], calculateFollowSet(grammar, trav->next->ele.type.nt.enumcode, followSet, firstSet));
                            }
                        }
                    }
                    
                }
            }
            trav = trav->next;
        }
    }
    //We are done with this non-terminal
    followSet[nonTerminal][enumTerminal+2] = 1;     //Marked as already traversed
    followSet[nonTerminal][enumTerminal+1] = 1;     //Marked as follow set already calculated
    return followSet[nonTerminal];
}