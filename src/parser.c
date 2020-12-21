/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

#include "lexer.h"
#include "lexerDef.h"
#include "parser.h"
#include "parserDef.h"

int enumTerminal = 0;
int enumNonTerminal=0;
int epsilonENUM = 0;


// this function is used to initialise the Grammar array
// this means that only the non terminals (any term that occures in LHS of a rule)
// is populated into the grammar structure using this function 
void populateGrammarArray(Grammar * grammar, char * str, int TorNT, int index)
{
    Element * ele = hash_find(str, hash_tb);
	
	//Node is the structure that has ele insidme it, 
	//ele has type, which has NT or T, which have str to store the token
    Node * newHead = (Node *)malloc(sizeof(Node));
    Node * newTail = NULL;
    
    if(ele != NULL)     //Is in the hash table
    {
        //increase the grammar size by one
        grammar->size += 1;
        
        //newHead's ele equated to ele we got from hash table
        //this is static copying mechanism, cause newHead->ele is not a pointer
        newHead->ele = *ele;
        newHead->next = NULL;
        newTail = newHead;
        LinkedList list;
        list.head = newHead;
        list.tail = newTail;
        list.size = 0;
        
        // add the non terminal of the LHS to the grammar
        grammar->arr[index] = list;
    }
    else    //Isn't in the hash table
    {
        grammar->size += 1;
        
        TokenType type;
        NonTerminal nt;
        
        //we thus intialise the statically allocated type and nt
        nt.enumcode = enumNonTerminal;
        enumNonTerminal++;
        strcpy(nt.str, str);
        type.nt = nt;
		
		//the mallocated ele now copies the info to it's type
		//thus heap now has the above initialised info
        newHead->ele.tag = 1;
        newHead->ele.type = type;
        newHead->next = NULL;
        newTail = newHead;
		
        LinkedList list;
        list.head = newHead;
        list.tail = newTail;
        list.size = 0;
        
        //add the non terminal in the hash table as well
        hash_insert(&(newHead->ele), hash_tb);
        grammar->arr[index] = list;
			
    }
}

// this function inserts RHS side of the rules
void insertInLinkedList(Grammar * grammar, char * str, int TorNT, int index)
{
    //take the head and the tail of the appropriate grammar rule
    Node * trav = grammar->arr[index].head;
    Node * tail = grammar->arr[index].tail;
    
    //increase the rule's size by one
    grammar->arr[index].size += 1;
    
    //new node for the linkedlist
    Node * newNode = (Node *)malloc(sizeof(Node));
    
    Element * ele = hash_find(str, hash_tb);
    
    tail->next = newNode;
    if(ele != NULL)     //Found in hash table, can be a T or a NT
    {
    	//again static copying
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
		
		//global enumcode is set here
        if(strcmp(str, "EPSILON") == 0)
            epsilonENUM = t.enumcode;

        ele.tag = TorNT;
        ele.type = type;
		
		//static copying once more
        newNode->ele = ele;
        newNode->next = NULL;
        tail->next = newNode;
        grammar->arr[index].tail = tail->next;
		
		//since it wasn't in hash insert it now
        hash_insert(&ele, hash_tb);        
    }
}


// Use this function to print the grammar as interpreted in the linked list array
//only a utility function to test the grammar function
void printGrammar(Grammar * grammar)
{
    Node * trav = NULL;
    for(int i=0; i<grammar->size; i++)
    {
        trav = grammar->arr[i].head;

        while(trav != NULL && trav->next != NULL)
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


void initializeParser()
{
    enumToTerminal = NULL;
    enumToNonTerminal = NULL;
    enumTerminal = 0;
    enumNonTerminal = 0;
    epsilonENUM = 0;
    hash_tb = NULL;
}

Grammar * read_grammar(char * filename)
{
    Grammar * grammar = (Grammar *)malloc(sizeof(Grammar));
    grammar->size = 0;
    grammar->arr = NULL;
    
    //Opening the file
    FILE *fp = fopen(filename,"r");
    if(fp == NULL)
    {
        printf("Error in opening the grammar file\n");
        return NULL;
    }
    
    //FIRST PASS RULECOUNT 
    hash_tb = (Hashtable* )malloc(sizeof(Hashtable));
    
    //initialise the hash table 
    for(int i=0;i<HASHSIZE;i++)
    {
        hash_tb->arr[i].head=NULL;
        hash_tb->arr[i].tail=NULL;
    }
    
    char * temp = (char *)malloc(sizeof(char)*NTSIZE);
    int rulecount = 0;
    
    //read the whole line and count the number of rules
    while(fscanf(fp,"%s%*[^\n]\n",temp) != EOF)
    {
        rulecount++;
        memset(temp, '\0', sizeof(char)*NTSIZE);
    }
    rewind(fp);
    //go back to the beginning of the file

    //initialise the grammar structure
    grammar->arr = (LinkedList*) malloc(sizeof(LinkedList)*rulecount);
    rulecount = 0;

    //we now reaad the rules ones by one
    while(fscanf(fp,"%s%*[^\n]\n",temp) !=  EOF)
    {
        //populategrammararray fnction poulates the LHS non terminals
        populateGrammarArray(grammar, temp, 1, rulecount);
        rulecount++;
        memset(temp, '\0', sizeof(char)*NTSIZE);
    }
    rewind(fp);
    //file ppointer is back at the beginning


    //SECOND PASS begins here. The nonterminals are populated already
    int rulenum = 0;
    int linestart=1;

    char * xyz = (char *)malloc(sizeof(char)*RULESIZE);
    char * token;
    memset(xyz, '\0', sizeof(char)*RULESIZE);

    int flag = 0;
    
    // read the whole line at a time
    while(fscanf(fp,"%[^\n]\n",xyz) != EOF)
    {
        //keep tokenising through strtok
        token = strtok(xyz, " ");
        flag = 0;
        while(token)
        {
            if(flag == 0)
            {
                flag++;
                token = strtok(NULL, " ");
            }
            Element *ele = hash_find(token, hash_tb);
            if(ele==NULL)
            {
                //insert the terminal inside the linkedlist
                //because it was not found
                insertInLinkedList(grammar, token, 2, rulenum);
            }
            else
            {
                //if found, it could be both terminal or NT
                //depending on what it is, insert it appropriately
                if(ele->tag == 1)
                    insertInLinkedList(grammar, token, 1, rulenum);
                else if(ele->tag == 2)
                    insertInLinkedList(grammar, token, 2, rulenum);
            }
            flag = 1;
            token = strtok(NULL, " ");
        }
        rulenum++;
        memset(xyz, '\0', sizeof(char)*RULESIZE);
    }
    return grammar;  
}

int ** initializeFirst()
{
    int ** firstSet = (int **)malloc(sizeof(int*)*enumNonTerminal);
    firstEquations = (int **)malloc(sizeof(int *)*(enumNonTerminal));

    for(int i=0; i<enumNonTerminal; i++)
    {
        firstSet[i] = (int *)malloc(sizeof(int)*(enumTerminal+3));
        firstEquations[i] = (int *)malloc(sizeof(int)*(enumNonTerminal+1));
        memset(firstSet[i],0,sizeof(int)*(enumTerminal+3));
        memset(firstEquations[i],0,sizeof(int)*(enumNonTerminal+1));
        firstSet[i][enumTerminal+1] = -1;  //Indicates whether we have calculated first set or not, special
        firstSet[i][enumTerminal+2] = -1;  //Indicates whether we have traversed this NT before or not, special
    }
    return firstSet;
}

int ** initializeFollow()
{
    int ** followSet = (int **)malloc(sizeof(int*)*enumNonTerminal);
    followEquations = (int **)malloc(sizeof(int*)*enumNonTerminal);
    for(int i=0; i<enumNonTerminal; i++)
    {
        followSet[i] = (int *)malloc(sizeof(int)*(enumTerminal+3));
        followEquations[i] = (int *)malloc(sizeof(int)*(enumNonTerminal+1));
        memset(followSet[i],0,sizeof(int)*(enumTerminal+3));
        memset(followEquations[i],0,sizeof(int)*(enumNonTerminal+1));
        
        //Indicates whether we have calculated follow set or not, special
        followSet[i][enumTerminal+1] = -1;  
        
        //Indicates whether we have traversed this NT before or not, special
        followSet[i][enumTerminal+2] = -1;  

        // //Indicates on what nonTerminal does i have circular dependency with
        // followSet[i][enumTerminal+3] = -1;
    }
        
    followSet[0][enumTerminal] = 1; 
    //Puts '$' in the follow of start symbol
    
    return followSet;
}

int setOR(int * arr1, int * arr2)
{
    int changed = 0;
    for(int i=0; i<=enumTerminal; i++)
    {
        if((arr1[i]==0 && arr2[i]==1) && i!=epsilonENUM)
        {
            changed=1;
            arr1[i] = 1;
        }
        
    }
    return changed;
}


//Basically a DFS implementation for First and Follow Set
// Analogy of nodes that are already visited and done
// Or nodes that are visited but not done (you have cycled back)
// Or nodes that are not visited and hence you are visiting them 
// for the first time through some other NT's rule

/*For a NT, we have following cases:
    1) It's follow has been calculated.     
    followSet[NT][enumTerminal+1] = 1
    followSet[NT][enumTerminal+2] = 1
    
    
    2) It's follow set is being calculated and it has been traversed.   
    followSet[NT][enumTerminal+1] = -1
    followSet[NT][enumTerminal+2] = 1
    
    3) It's follow set is not yet calculated.   
    followSet[NT][enumTerminal+1] = -1
    followSet[NT][enumTerminal+2] = -1
*/

void calculateFirstEquations(Grammar * grammar, int **firstSet, int ** firstEquations)
{
    while(1)
    {
        int changed = 0;

        //travel through the grammar for epsilons
        for(int i=0; i<grammar->size; i++)
        {
            //LHS of the grammar
            Node * trav = grammar->arr[i].head;
            
            // RHS's head 
            trav = trav->next;

            // the RHS head is a non terminal
            if(trav->ele.tag == 1)
            {
                while(trav != NULL)
                {
                    //If current symbol is a NT and doesn't derive EPSILON, then break
                    if(trav->ele.tag == 1)
                    {
                        if(firstSet[trav->ele.type.nt.enumcode][epsilonENUM] != 1)
                            break;
                    }
                    else    //if current symbol is a Terminal, break
                        break;
                    trav = trav->next;
                }

                //If every symbol on RHS derives an EPSILON, put EPSILON in this NT's first
                if(trav==NULL)
                {
                    if(firstSet[grammar->arr[i].head->ele.type.nt.enumcode][epsilonENUM] != 1)
                        changed = 1;
                    firstSet[grammar->arr[i].head->ele.type.nt.enumcode][epsilonENUM] = 1;
                }
            }

            // RHS head is a terminal
            else
            {
                // if it is an EPSILON
                if(trav->ele.type.t.enumcode == epsilonENUM)
                {
                    if(firstSet[grammar->arr[i].head->ele.type.nt.enumcode][epsilonENUM] != 1)
                        changed = 1;
                    firstSet[grammar->arr[i].head->ele.type.nt.enumcode][epsilonENUM] = 1;
                }
            }
        }
        if(changed == 0)
            break;
    }

    // traverse the grammar for other non-terminals
    for(int i=0;i<grammar->size;i++)
    {
        // start from the LHS 
        int ntEnum = grammar->arr[i].head->ele.type.nt.enumcode;
        Node *trav  = grammar->arr[i].head;
        trav = trav->next;
        if(trav->ele.tag == 1)
        {
            while(trav != NULL)
            {
                if(trav->ele.tag == 1)
                {
                    int size = firstEquations[ntEnum][0];
                    firstEquations[ntEnum][size+1] = trav->ele.type.nt.enumcode;
                    firstEquations[ntEnum][0]++;
                    //No more NTs from RHS need to be added if current EPSILON isn't in FIRST(this NT)
                    if(firstSet[trav->ele.type.nt.enumcode][epsilonENUM] == 0)
                        break;
                }
                else
                {
                    firstSet[ntEnum][trav->ele.type.t.enumcode] = 1;
                    break;
                }
                trav = trav->next;
            }
        }
        else
            firstSet[ntEnum][trav->ele.type.t.enumcode] = 1;
    }
}

void calculateFollowEquations(Grammar * grammar, int **followSet, int **firstSet, int **followEquations)
{
    for(int i=0; i<grammar->size; i++)
    {
        Node * trav = grammar->arr[i].head;
        trav = trav->next;
        //travel through the rule's RHS

        while(trav != NULL)
        {
            if(trav->ele.tag == 1)
            {
                if(trav->next == NULL)  // RUle A -> X....B where we visit B
                {
                    int index = followEquations[trav->ele.type.nt.enumcode][0];
                    followEquations[trav->ele.type.nt.enumcode][index+1] = grammar->arr[i].head->ele.type.nt.enumcode;
                    followEquations[trav->ele.type.nt.enumcode][0]++;
                }
                else
                {
                    // Rule is of type A -> X...B..X and we are at B after which 
                    //there is something on right side for sure
                    int row = trav->ele.type.nt.enumcode;
                    Node * tmp = trav->next;
                    while(tmp != NULL)
                    {
                        if(tmp->ele.tag == 2)
                        {
                            // simply set the terminal in the follow
                            followSet[row][tmp->ele.type.t.enumcode] = 1;
                            break;
                        }
                        else
                        {
                            // the follow equation will add to it the follow of the Non terminal
                            // int index = followEquations[row][0];
                            // followEquations[row][index+1] = grammar->arr[i].head->ele.type.nt.enumcode;
                            // followEquations[row][0]++;
                            setOR(followSet[row], firstSet[tmp->ele.type.nt.enumcode]);
                            if(firstSet[tmp->ele.type.nt.enumcode][epsilonENUM] != 1)
                            {
                                break;
                            }
                        }
                        tmp = tmp->next;
                    }
                    
                    if(tmp == NULL)
                    {
                        int index = followEquations[row][0];
                        followEquations[row][index+1] = grammar->arr[i].head->ele.type.nt.enumcode;
                        followEquations[row][0]++;
                    }
                }
            }
            trav = trav->next;
        }
    }
}

void calculateFirstSet(Grammar * grammar, int ** firstSet, int ** firstEquations)
{
    while(1)
    {
        // keep going until no change is detected
        int changed = 0;
        for(int i=0; i<enumNonTerminal; i++)
        {
            // for all the non terminals occuring in the first set equation of this non terminal
            for(int j=1; j<=firstEquations[i][0]; j++)
            {
                int nonT = firstEquations[i][j];
                changed = changed || setOR(firstSet[i], firstSet[nonT]);
            }
        }
        //  break condition
        if(changed == 0)
            break;
    }
}

void calculateFollowSet(Grammar * grammar, int ** followSet, int ** followEquations)
{
    while(1)
    {
        // keep going until no change is detected
        int changed = 0;
        
        for(int i=0; i<enumNonTerminal; i++)
        {
            for(int j=1; j<=followEquations[i][0]; j++)
            {
                int nonT = followEquations[i][j];
                changed = changed || setOR(followSet[i], followSet[nonT]);
            }
        }
        
        if(changed == 0)
            break;
    }
}

void map(Grammar * grammar)
{
    enumToTerminal = (char **)malloc(sizeof(char *)*(enumTerminal+1));
    enumToNonTerminal = (char **)malloc(sizeof(char *)*enumNonTerminal);
    char *EndOfFile = (char*)malloc(sizeof(char)*4);
    strcpy(EndOfFile,"EOF");
    enumToTerminal[enumTerminal] = EndOfFile;
    for(int i=0; i<grammar->size; i++)
    {
        Node * trav = grammar->arr[i].head;
        while(trav!=NULL)
        {
            if(trav->ele.tag == 1)
            {
                enumToNonTerminal[trav->ele.type.nt.enumcode] = trav->ele.type.nt.str;
            }
            else
            {
                enumToTerminal[trav->ele.type.t.enumcode] = trav->ele.type.t.str;
            }
            trav = trav->next;
        }
    }
}

void printMap()
{
    printf("---- Terminals ---- \n");
    for(int i=0; i<enumTerminal+1 ; i++)
    {
        printf("%s  %d\n",enumToTerminal[i], i);
    }

    printf("---- Non Terminals ---- \n");
    for(int i=0; i<enumNonTerminal ; i++)
    {
        printf("%s  %d\n",enumToNonTerminal[i], i);
    }
}

void printFirst(int ** firstSet)
{
    for(int i=0; i<enumNonTerminal; i++)
    {
        printf("%s : ", enumToNonTerminal[i]);
        for(int j=0; j<enumTerminal; j++)
        {
            if(firstSet[i][j]==1)
                printf("%s ", enumToTerminal[j]);
        }
        printf("\n");
    }
}
void printFirstEquations()
{
    for(int i=0; i<enumNonTerminal; i++)
    {
        printf("%s : ", enumToNonTerminal[i]);

        for(int j=1; j<=firstEquations[i][0]; j++)
        {
            printf("%s ", enumToNonTerminal[firstEquations[i][j]]);
        }
        printf("\n");
    }
}
void printFollow(int ** followSet)
{
    for(int i=0; i<enumNonTerminal; i++)
    {
        printf("%s : ", enumToNonTerminal[i]);
        for(int j=0; j<=enumTerminal; j++)
        {
            if(followSet[i][j]==1)
                printf("%s ", enumToTerminal[j]);
        }
        printf("\n");
    }
}
void printFollowEquations()
{
    for(int i=0; i<enumNonTerminal; i++)
    {
        printf("%s : ", enumToNonTerminal[i]);

        for(int j=1; j<=followEquations[i][0]; j++)
        {
            printf("%s ", enumToNonTerminal[followEquations[i][j]]);
        }
        printf("\n");
    }
}

int ** intializeParseTable()
{
    int ** parseTable = (int **) malloc(sizeof(int*)*enumNonTerminal);
    
    for (int i=0;i<enumNonTerminal;i++)
    {
        parseTable[i] = (int*)malloc(sizeof(int)*enumTerminal+1);
        
        //initialise the table with errors now itself
        for(int j=0;j<enumTerminal+1;j++)
            parseTable[i][j]=(-1);
    }
    return parseTable;
}

void createParseTable(Grammar *grammar, int **parseTable, int **firstSet, int **followSet)
{
    
    for(int i = 0 ; i < grammar->size ; i++)
    {
        Node *trav = grammar->arr[i].head;
        
        //array to get firstset of the RHS of the rule
        int arr[enumTerminal+1];
        memset(arr,0, sizeof(int)*(enumTerminal+1));
        trav = trav->next;
        while(trav!=NULL)
        {
            if(trav->ele.tag == 2)
            {
                if(trav->ele.type.t.enumcode == epsilonENUM)
                    setOR(arr,followSet[grammar->arr[i].head->ele.type.nt.enumcode]);
                else
                    arr[trav->ele.type.t.enumcode] = 1;
                break;
            }
            else
            {
                setOR(arr,firstSet[trav->ele.type.nt.enumcode]);
                
                if(firstSet[trav->ele.type.nt.enumcode][epsilonENUM]==0)        //Base case, stop now
                    break;
            }

            trav = trav->next;
        
        }
        if(trav==NULL)
        {
            setOR(arr,followSet[grammar->arr[i].head->ele.type.nt.enumcode]);
        }
        for(int j=0 ; j<enumTerminal+1 ; j++)
        {
            if(arr[j])
            {
                parseTable[grammar->arr[i].head->ele.type.nt.enumcode][j] = i;
            }
        }
		
		memset(arr,0, sizeof(int)*(enumTerminal+1));
		setOR(arr,followSet[grammar->arr[i].head->ele.type.nt.enumcode]);
		
		for(int j=0; j < enumTerminal+1; j++)
		{
			if(arr[j]==1)
			{
				if(parseTable[grammar->arr[i].head->ele.type.nt.enumcode][j] == -1)
					parseTable[grammar->arr[i].head->ele.type.nt.enumcode][j] = -2;
			}
		}
    }
    return;
}

void printParseTable(Grammar *grammar,int ** parseTable)
{
    for(int j=0;j<enumTerminal+1;j++)
    {
        printf("%20s  ",enumToTerminal[j]);
    }
    printf("\n");

    for(int i = 0 ; i < enumNonTerminal ; i++)    
    {
        printf("%20s  ",enumToNonTerminal[i]);
        for(int j=0;j<enumTerminal+1;j++)
        {
            printf("%20d  ",parseTable[i][j]);
        }
        printf("\n");
    }    

    return;
}

//This function creates a TreeNode given a terminal or a non-terminal
TreeNode * getNode(NonTerminal * nt, Token * tkn, int whichOne)
{
    TreeNode * newNode = (TreeNode *)malloc(sizeof(TreeNode));
    //Node is a non-terminal i.e. non-leaf node
    if(whichOne == 1)   
    {
        newNode->tag = 1;
        newNode->ele.nonleaf.nt = *nt;
    }
    //Node is a terminal i.e. leaf node
    else if(whichOne == 2)  
    {
        newNode->tag = 2;
        newNode->ele.leaf.tkn = *tkn;
    }
    newNode->sibling = NULL;
    newNode->child = NULL;
    return newNode;
}

//This function inserts node as the sibling of head
//Pass the pointer to the node ("head") whose sibling has to be "node"
//Returns a pointer to the head of siblings. Assign it back to the head of the siblings
TreeNode * siblingInsert(TreeNode * head, TreeNode * node)      
{                                                               
    if(head == NULL)
    {
        head = node;
        node->child = NULL;
        node->sibling = NULL;
        return head;
    }

    TreeNode * trav = head;

    while(trav->sibling != NULL)
    {
        trav = trav->sibling;
    }

    trav->sibling = node;
    node->child = NULL;
    node->sibling = NULL;
    return head;
}


//This function inserts newNode as the child of parent
void insert(TreeNode * parent, TreeNode * newNode)
{
    parent->child = siblingInsert(parent->child, newNode);
}



void inOrder(TreeNode * root, TreeNode * parent)
{
    if(root == NULL)
    {
        return;
    }
    
    TreeNode * trav = root->child;
    inOrder(trav, root);

    if(root->tag == 2)  //Its a leaf
    {
        if(strcmp(root->ele.leaf.tkn.token, "NUM") == 0 && (root->ele.leaf.tkn.value != NULL))
        {
            int *x = (int *)root->ele.leaf.tkn.value;
            printf("|| %10s || %10d || %15s || %10d || %20s || %8s || %20s ||", 
            root->ele.leaf.tkn.lexeme, root->ele.leaf.tkn.lineNum, root->ele.leaf.tkn.token, *x, parent->ele.nonleaf.nt.str, "YES", "----");
        }
        else if(strcmp(root->ele.leaf.tkn.token, "RNUM") == 0 && (root->ele.leaf.tkn.value != NULL))
        {
            float *x = (float *)root->ele.leaf.tkn.value;
            printf("|| %10s || %10d || %15s || %10lf || %20s || %8s || %20s ||", 
            root->ele.leaf.tkn.lexeme, root->ele.leaf.tkn.lineNum, root->ele.leaf.tkn.token, *x, parent->ele.nonleaf.nt.str, "YES", "----");
        }
        else
        {
            printf("|| %10s || %10d || %15s || %10s || %20s || %8s || %20s ||", 
            root->ele.leaf.tkn.lexeme, root->ele.leaf.tkn.lineNum, root->ele.leaf.tkn.token,"----",parent->ele.nonleaf.nt.str,"YES","----");
        }
    }
    else    //It's an internal node
    {
        if(parent == NULL)  //If root is the ROOT node
        {
            printf("|| %10s || %10s || %15s || %10s || %20s || %8s || %20s ||", 
            "----","----","----","----","ROOT","NO",root->ele.nonleaf.nt.str);
        }
        else    //If root is not the ROOT node
        {
            printf("|| %10s || %10s || %15s || %10s || %20s || %8s || %20s ||", 
            "----","----","----","----",parent->ele.nonleaf.nt.str, "NO", root->ele.nonleaf.nt.str);   
        }
    }
    printf("\n");
    while(trav != NULL)
    {
        trav = trav->sibling;
        inOrder(trav, root);
    }
}


void printTokenStream(TreeNode * root)
{
    if(root == NULL)
    {
        return;
    }

    if(root->tag == 2)  //Its a leaf
    {
        if(strcmp(root->ele.leaf.tkn.token,"EPSILON")!=0)
            printf("%s (%d)  ", root->ele.leaf.tkn.token, root->ele.leaf.tkn.lineNum);
    }
    
    // TreeNode * trav = root;
    // inOrder(trav->child);

    // trav = trav->sibling;
    // inOrder(trav);    

    TreeNode * trav = root->child;
    printTokenStream(trav);

    while(trav != NULL)
    {
        trav = trav->sibling;
        printTokenStream(trav);
    }
}

// void printTokenStream(TreeNode * root)
// {
//     if(root == NULL)
//     {
//         return;
//     }

//     if(root->tag == 2)  //Its a leaf
//     {
//         if(strcmp(root->ele.leaf.tkn.token,"EPSILON")!=0)
//             printf("%s (%d)  ", root->ele.leaf.tkn.token, root->ele.leaf.tkn.lineNum);
//     }
    
//     // TreeNode * trav = root;
//     // inOrder(trav->child);

//     // trav = trav->sibling;
//     // inOrder(trav);    

//     TreeNode * trav = root->child;
//     printTokenStream(trav);

//     while(trav != NULL)
//     {
//         trav = trav->sibling;
//         printTokenStream(trav);
//     }
// }

TreeNode * pop(Stack * st)
{
    if(st == NULL || st->top == NULL)
        return NULL;

    StackNode * top = st->top->next;
    TreeNode * trNode = st->top->trnode;
    free(st->top);
    st->top = top;
    st->size -= 1;
    return (trNode);
}

Stack * push(Stack * st, TreeNode * trNode)
{
    StackNode * top =(StackNode *)malloc(sizeof(StackNode));
    top->trnode = trNode;
    top->next = NULL;
    if(st == NULL)
    {
        st = (Stack *)malloc(sizeof(Stack));
        st->top = top;
        st->size = 1;
        return st;
    }
    top->next = st->top;
    st->top = top;
    st->size += 1;
    return st;
}

void printStack(Stack * st)
{
    if(st==NULL)
        return;
    StackNode * trav = st->top;
    while(trav != NULL)
    {
    	//Its an internal node
        if(trav->trnode->tag == 1)  
            printf("%s (%d)\n", trav->trnode->ele.nonleaf.nt.str, trav->trnode->ele.nonleaf.nt.enumcode);
        else    //Its a leaf
            printf("%s (%d)\n", trav->trnode->ele.leaf.tkn.token, trav->trnode->ele.leaf.tkn.lineNum);
        trav = trav->next;
    }
}

/*******************************************************************/



TreeNode * parser(Grammar * grammar, int ** parsetable)
{
    TreeNode * parseTree = NULL;
    Token * eofTkn = (Token *)malloc(sizeof(Token));
    NonTerminal * nt =(NonTerminal *)malloc(sizeof(NonTerminal));

    eofTkn->lexeme = (char *)malloc(sizeof(char)*4);
    strcpy(eofTkn->lexeme, "EOF");
    eofTkn->token = (char *)malloc(sizeof(char)*4);
    strcpy(eofTkn->token, "EOF");
    eofTkn->lineNum = -1;
    TreeNode * eof = getNode(NULL, eofTkn, 2);

    nt = &(grammar[0].arr->head->ele.type.nt);
    parseTree = getNode(nt,NULL,1);

    Stack *mainStack = NULL, *auxillaryStack = NULL;
    
    mainStack = push(mainStack, eof);
    mainStack = push(mainStack, parseTree);

    Token * tkn = NULL;
    TreeNode * node = NULL;
    Element * ele = NULL;

    char temp[20] = "EOF\0";

    int ruleNum = -10, index = 0;

    // tkn = tempo_tokens[index];
    tkn = getNextToken();       //Make 1 lookahead

    int j;
    char * error = (char *)malloc(sizeof(char)*200);
    memset(error, '\0', sizeof(char)*200);
    int isFirst = 1, isFirstTerminal = 1;
    while(mainStack->size != 0)
    {
        //top of the stack is terminal

        if(mainStack->top->trnode->tag == 2)    
        {
			isFirst = 1;
            //get the next token from the lexer if the top of stack is a non terminal
            //and the input matched it
            if(strcmp(tkn->token, mainStack->top->trnode->ele.leaf.tkn.token) == 0)
            {
                node = pop(mainStack);
                node->ele.leaf.tkn.lineNum = tkn->lineNum;
                node->ele.leaf.tkn.value = tkn->value;
                node->ele.leaf.tkn.lexeme = tkn->lexeme;
                tkn = getNextToken();
                // printf("%s\n", tkn->lexeme);
            }
            //throw an error
            else
            {
		// if((strcmp(mainStack->top->trnode->ele.leaf.tkn.token,"EOF")==0) && (strcmp(tkn->token, "EOF")!=0))
		// {
		// 	tkn = getNextToken();
		// 	continue;
		// }
		if(isFirstTerminal == 1)
		{
			sprintf(error, "Expected %s, got %s", mainStack->top->trnode->ele.leaf.tkn.token, tkn->token);
			insertError(error, tkn->lexeme, tkn->lineNum, 2);
		}
		
		isFirstTerminal++;
                node = pop(mainStack);
            }
        }
        else // the top of the stack is a non terminal
        {
            //pop the NT and find its enumerated code in hash table
			ele = hash_find(tkn->token, hash_tb);
			if(strcmp(tkn->token, "EOF") == 0)
                j = enumTerminal;
            else
                j = ele->type.t.enumcode;

			ruleNum = parsetable[mainStack->top->trnode->ele.nonleaf.nt.enumcode][j];
			if(ruleNum < 0)
            	{
					if(strcmp(tkn->token, "EOF") == 0 && mainStack->top->trnode->tag == 1)	//If top of the stack is a terminal and we have read complete program
					{
						break;
					}
                    //error
                    if(ruleNum == -1)
                    {
                        if(isFirst == 1)
                        {
                            sprintf(error, "Did not expect %s (%s)", tkn->lexeme, tkn->token);
                            insertError(error, tkn->lexeme, tkn->lineNum, 2);
                        }
                        isFirst++;
                        tkn = getNextToken();   //Ignore the token
                        continue;
                    }
                    else if(ruleNum == -2)
                    {
                        if(isFirst == 1)
                        {
                            sprintf(error, "Did not expect %s (%s)", tkn->lexeme, tkn->token);
                            insertError(error, tkn->lexeme, tkn->lineNum, 2);
							isFirst++;
                        }
                        else
                        {
                            isFirst = 1;
                        }
                        node = pop(mainStack);
                        continue;
                    }
                }
			isFirstTerminal = 1;
            node = pop(mainStack);  //node must a be non-terminal

            ele = hash_find(tkn->token, hash_tb);

            if(ele == NULL && (strcmp(tkn->token,"EOF") != 0))
            {
                //Problem in grammar parsing
                // printf(" \n The top of the stack was an unidentified terminal ! This error should not occur actually");
                // return NULL;
            }
            else
            {
                if(strcmp(tkn->token, "EOF") == 0)
                    j = enumTerminal;
                else
                    j = ele->type.t.enumcode;
                
                ruleNum = parsetable[node->ele.nonleaf.nt.enumcode][j];
                Node * trav = NULL;
                trav = grammar->arr[ruleNum].head->next;
                
                if(trav->ele.tag == 2 && trav->ele.type.t.enumcode == epsilonENUM)
                {
                    Token * tkn = (Token *)malloc(sizeof(Token));
                    tkn->lexeme = (char *)malloc(sizeof(char)*8);
                    strcpy(tkn->lexeme, "EPSILON");
                    tkn->token = (char *)malloc(sizeof(char)*8);
                    strcpy(tkn->token, "EPSILON");
                    TreeNode * pshNode = getNode(NULL,tkn,2);
                    insert(node, pshNode);
                    continue;
                }
                //Push the RHS of the rule on auxillary stack and also insert the node in the parse tree
                while(trav != NULL)
                {
                    TreeNode * pshNode;
                    if(trav->ele.tag == 2)  //If it is a terminal, insert as a leaf node in the tree
                    {
                        Token * tkn = (Token *)malloc(sizeof(Token));
                        tkn->lexeme = trav->ele.type.t.str;
                        tkn->token = trav->ele.type.t.str;
                        pshNode = getNode(NULL, tkn, 2);
                    }
                    else //If it is a non-terminal, insert as a non-leaf node in the trees
                    {
                        pshNode = getNode(&(trav->ele.type.nt), NULL, 1);
                    }
                    //push in reverse manner on the auxiliary stack
                    auxillaryStack = push(auxillaryStack, pshNode);
                    //insert in that order in the tree now itself
                    insert(node, pshNode);
                    trav = trav->next;
                }
                
                //Now, pop TreeNode from the auxillary stack and transfer it to the main stack
                //in the reverse order of what it is in auxiliary stack
                StackNode * top = auxillaryStack->top;
                TreeNode * tmp = NULL;
                while(auxillaryStack->size != 0)
                {
                    tmp = pop(auxillaryStack);   
                    mainStack = push(mainStack, tmp);
                }
            }
        }
    }
    return parseTree;
}

void freeprasetree(TreeNode *root)
{
    //base case
    if(root==NULL)
        return;
    
    //call one the child
    if(root->child!=NULL)
        freeprasetree(root->child);
    
    //free all the siblings
    if(root->sibling!=NULL)
    {
        TreeNode * trav = root->sibling;
        while(trav!=NULL)
        {
            freeprasetree(trav);
            trav=trav->sibling;
        } 
    }    

    free(root);
}

void freerule(Node* trav)
{
    if(trav==NULL)
    return;

    freerule(trav->next);
    free(trav);
}

void freegrammar(Grammar* root)
{
    for(int i=0;i<root->size;i++)
    {
        freerule(root->arr->head);
    }
}