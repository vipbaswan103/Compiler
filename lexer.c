/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

#include "lexer.h"
#include "lexerDef.h"

int lineNum = 1;
char *keywords[] = {"integer","real","boolean","of","array","start","end","declare","module","driver",
					"program","get_value","print","use","with","parameters","true","false","takes","input",
					"returns","AND","OR","for","in","switch","case","break","default","while","driverdef","driverenddef"};


// Takes a string to calculate its hash
int hash_func(char *str)
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
    return val % HASHSIZE ;
}


Element* hash_find(char * str, Hashtable * hash_tb)
{
    int hash;
    hash = hash_func(str);
    
	//access the head at that position, which is of type Node*
    Node * trav = hash_tb->arr[hash].head; 
    char * lexeme;
    while(trav != NULL)
    {
        //looking at a non terminal
        if(trav->ele.tag==1)
            lexeme = trav->ele.type.nt.str;
		//looking at a terminal
        else if (trav->ele.tag==2)
            lexeme = trav->ele.type.t.str;
		//return the trminal or non terminal if the lexeme matches
        if(strcmp(lexeme,str) == 0)
            return &(trav->ele);
		
		//keep searching
        trav = trav->next;
    }

	//if not present, returns NULL
    return NULL;
}


void hash_insert(Element * ele, Hashtable * hash_tb)
{
	//extract the name of the lexeme according to the tag
    char * str;
    if(ele->tag == 1)
        str = ele->type.nt.str;
    else
        str = ele->type.t.str;

	//find if it's already there in the hash table
    if(hash_find(str,hash_tb) != NULL)
        return; 

    int hash;
    
	//calculate the correct position in the hash table
    if(ele->tag==1)
        hash = hash_func(ele->type.nt.str);
    else if (ele->tag==2)
        hash = hash_func(ele->type.t.str);

	//wrap the ele (fully copy the static thing into a dynamic) in a Node for the hash's linked list
    Node *temp = (Node*)malloc(sizeof(Node)) ;
    temp->ele = *ele ;
    temp->next = NULL ;

	//if the list is empty 
    if( hash_tb->arr[hash].head == NULL)
    {
        hash_tb->arr[hash].head = temp ;
        hash_tb->arr[hash].tail = temp ;
        hash_tb->arr[hash].size = 1 ;
    }
	//if the list is not empty
    else
    {
        hash_tb->arr[hash].tail->next = temp ;
        hash_tb->arr[hash].tail = temp ;
        hash_tb->arr[hash].size++ ;
    }
    
    return ;
}


//simply mallocates memory to the hash table ADT
void initializeKeyHash()
{
	keyhash = (Hashtable *)malloc(sizeof(Hashtable));
}


//initialises the key hash table with predefined keywords above
void populate_keyhash()
{
	Element * ele = (Element *)malloc(sizeof(Element));
	for(int i=0; i<32; i++)
	{
		ele->tag = 2;
		strcpy(ele->type.t.str, keywords[i]);
		hash_insert(ele, keyhash);
	}
}

//finding something in keyhash table (simple)
int keyhash_find(char * str)
{
	Element * ele = hash_find(str, keyhash);
	if(ele == NULL)
		return 0;
	return 1;
}


//tag indicated the type of error to be pushed
void insertError(char * description, char* lexeme, int linenum, int tag)
{
    if(tag==1)
    {
        if(LexHead==NULL)   //List is empty
        {
            LexHead=(ErrorNode *)(malloc(sizeof(ErrorNode)));
            LexHead->tag=tag;
            LexHead->next=NULL;
            strcpy(LexHead->err.lex.description, description);
            LexHead->err.lex.lineNum=linenum;
            strcpy(LexHead->err.lex.lexeme, lexeme);
            return;
        }
    }
    else
    {
        if(SynHead==NULL)   //List is empty
        {
            SynHead=(ErrorNode *)(malloc(sizeof(ErrorNode)));
            SynHead->tag=tag;
            SynHead->next=NULL;
            strcpy(SynHead->err.lex.description, description);
            SynHead->err.lex.lineNum=linenum;
            strcpy(SynHead->err.lex.lexeme, lexeme);
            return;
        }
    }
    
    ErrorNode * newnode=(ErrorNode *)malloc(sizeof(ErrorNode));
    newnode->tag=tag;
    newnode->next=NULL;
    strcpy(newnode->err.lex.description, description);
    newnode->err.lex.lineNum=linenum;
    strcpy(newnode->err.lex.lexeme, lexeme);

    ErrorNode * trav = NULL; 
    if(tag == 1)
        trav = LexHead;
    else
        trav = SynHead;
    
    while(trav->next!=NULL)
        trav=trav->next;
    trav->next=newnode;
}

void printErrorList(int whichOne)
{
    ErrorNode * trav = NULL;
    trav = (whichOne == 1) ? LexHead : SynHead;

    while(trav != NULL)
    {
        if(whichOne == 1)
            printf("%s %d : %s", "Line ", trav->err.lex.lineNum, trav->err.lex.description);
        else
            printf("%s %d : %s", "Line ", trav->err.syn.lineNum, trav->err.syn.description);
        printf("\n");
        trav = trav->next;
    }
}

//simply takes a string and converts it to upper case
void uppertoken(char *str)
{
	int i=0;
	while(str[i] != '\0')
	{
		str[i] = toupper(str[i]);
		i++;
	}
}

//the buffer you ask for is filled up with more data
void readBuffer(FILE *fp, char * buffer)
{
    memset(buffer, '\0', sizeof(char)*BUFFERSIZE);
	int read = fread(buffer,1,BUFFERSIZE,fp);
	if(ferror(fp))
	{
		printf("Error in reading the file\n");
		exit(-1);
	}
}

//call this function when you want to start the lexical anaylsis
void initializeLexer(char * filename)
{
	//create new buffers
	buffer1 = (char *)malloc(sizeof(char) * BUFFERSIZE);
	buffer2 = (char *)malloc(sizeof(char) * BUFFERSIZE);

	//start at the first point
	startptr=0;
	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("Error opening file");
		return;
	}

	//read the file into the appropriate buffer(!)
	readBuffer(fp, buffer1);

	//set the pointers appropriately
	reading = 1;
	toRead = 1;
	lineNum = 1;
	SynHead = NULL;
	LexHead = NULL;
}

//read prog into buffer
Token * getNextToken()	
{
	Token * tkn = NULL;
	char * buffer;

	//which buffer am I supposed toRead
	if(toRead == 1)
		buffer = buffer1;
	else
		buffer = buffer2;
	

	//since this function will make a call to nextToken 
	//which will increment the global startptr,
	//keep giving tokens until the startpointer goes to NULL, 
	//which will refelect the end of the file
	while(buffer[startptr]!='\0')
	{
		//take the next token
		tkn = nextToken(fp, buffer, buffer1, buffer2, &startptr, &reading, &toRead);

		//Update the buffer to one which we are supposed to read next (Needed if tkn is NULL)
		if(toRead == 1)
			buffer = buffer1;
		else
			buffer = buffer2;

		//If we encounter spaces or nextlines, keep searching for some valid token
		if(tkn == NULL) 
			continue;
		
		else 
		{
			//Found the valid token, return it to the parser
			if(strcmp(tkn->token,"ERR")!=0) 
			{
				break;
			}
			//If lexical error, continue searching for the valid token
			else if(strcmp(tkn->token,"ERR")==0)   
			{
				insertError(tkn->lexeme, tkn->lexeme, tkn->lineNum, 1);
				continue;
			}
		}
    }

	//We didn't get any valid token but we have reached the end of the input, return EOF
    if(tkn == NULL && buffer[startptr] == '\0') 
    {
        tkn = (Token *)malloc(sizeof(Token));
        tkn->lexeme = (char *)malloc(sizeof(char)*4);
        strcpy(tkn->lexeme, "EOF");
        tkn->token = (char *)malloc(sizeof(char)*4);
        strcpy(tkn->token, "EOF");
        tkn->lineNum = lineNum;
    }

	//finally return the token, which could be valid or EOF 
	return tkn;
}

//this is like a utility function to convert the lexeme to an integer
int lexeme2int(char *s)
{
	int i=0, val=0;
	while(s[i]!='\0')
	{
		val = 10*val + (s[i]-'0');
		i++;
	}
	return val;
}

//this is like a utility function to convert the lexeme to an real number
float lexeme2real(char *s)
{
	int i=0, j=0, flag=0, sign=1, exp_pow=0;
	float val=0.0, frac=0.0, exp=0.0;

	while(s[i]!='\0')
	{
		
		if(s[i]=='.')
		{
			//set the flag as 1 if decimal is found
			flag = 1;
			i++;
		}
		else if(s[i]=='E' || s[i] == 'e')
		{
			//set the flag as 2 if the exponentiation is to be done
			flag = 2;
			i++;
			if(s[i]=='-')
			{
				sign = -1;
				i++;
			}
			else if(s[i]=='+')
			{
				i++;
			}	
		}
		
		//flag=0 means simple number, no E no decimal (.)
		if(flag==0)
		{
			val = val*10 + (s[i]-'0');	
		}

		//flag=1 means we are after the decimal
		else if(flag==1)
		{
			j++;
			frac = frac + pow(10,(-1)*j)*(s[i]-'0');							
		}

		//flag=2 means we are after the exp E symbol
		else if(flag==2)
		{
			exp_pow = exp_pow*10 + (s[i]-'0');
		}

		i++ ;
	}
	
	//appropriately add them all
	exp = pow(10,sign*exp_pow);
	val = val+frac;
	val = val*exp;

	return val;	
}

void incrementPointer(FILE * fp, char ** buffer, char * buffer1, char * buffer2, int * reading, int * ptr)
{
	if(*(ptr) >= BUFFERSIZE-1)	//Buffer exhausted
	{
		*ptr = 0;
		if(*reading == 1)		//If I was reading buffer1, then refill buffer2
		{
			*reading = 2;
			readBuffer(fp, buffer2);
			*buffer = buffer2;
		}
		else					//If I was reading buffer2, then refill buffer1
		{
			*reading = 1;
			readBuffer(fp, buffer1);
			*buffer = buffer1;
		}
	}
	else						
	{
		(*ptr) = (*ptr) + 1;
	}
}

void decrementPointer(FILE * fp, char ** buffer, char * buffer1, char * buffer2, int * reading, int * ptr)
{
	if(*(ptr) == 0)	//Buffer exhausted
	{
		*ptr = BUFFERSIZE-1;
		if(*reading == 1)		//If I was reading buffer1, then move back to buffer2
		{
			*reading = 2;
			*buffer = buffer2;
		}
		else					//If I was reading buffer2, then move back to buffer1
		{
			*reading = 1;
			*buffer = buffer1;
		}
	}
	else						
	{
		(*ptr) = (*ptr) - 1;
	}
}
char * fillLexeme(char * buffer1, char * buffer2, int reading, int toRead, int startptr, int ptr)
{
	char * lexeme = NULL, *buffer;
	int len = 0, j=0;

	if(reading == toRead)		//Lexeme doesn't span two buffers
	{
		len = ptr-startptr+1;
		lexeme = (char *)malloc(sizeof(char)*len);
		// memset(lexeme, '\0', sizeof(char)*len);
		if(reading == 1)
			buffer = buffer1;
		else
			buffer = buffer2;

		for(int i=startptr; i<ptr; i++)
		{
			lexeme[j++] = buffer[i];
		}
	}
	else		//Lexeme spans two buffers
	{
		//Calculating length of the buffer
		len = BUFFERSIZE-startptr;
		len += (ptr+1);
		
		//Firstly read from the initial buffer
		//toRead indicates the initial buffer
		if(toRead == 1)
			buffer = buffer1;
		else
			buffer = buffer2;

		lexeme = (char *)malloc(sizeof(char)*len);
		// memset(lexeme, '\0', sizeof(char)*len);
		for(int i=startptr; i<BUFFERSIZE; i++)
		{
			lexeme[j++] = buffer[i];
		}

		//Now, read from the current buffer
		if(reading == 1)
			buffer = buffer1;
		else
			buffer = buffer2;
			
		for(int i=0; i<ptr; i++)
		{
			lexeme[j++] = buffer[i];
		}
	}
	lexeme[len-1] = '\0';
	return lexeme;
}

void getErrorToken(Token * tkn)
{
    tkn->lexeme = (char *)malloc(sizeof(char)*200);
    tkn->token = (char *)malloc(sizeof(char)*4);
    strcpy(tkn->token, "ERR");
    tkn->lineNum = lineNum;
}

void removeComments()
{
	char * buffer;
	if(toRead == 1)
		buffer = buffer1;
	else
		buffer = buffer2;
	
	//until the file ends
	while(buffer[startptr]!='\0')
	{
		removeCommentsUtil(fp, buffer, buffer1, buffer2, &startptr, &reading, &toRead);

		//Update the buffer to one which we are supposed to read next (Needed if tkn is NULL)
		if(toRead == 1)
			buffer = buffer1;
		else
			buffer = buffer2;
    }


}
void removeCommentsUtil(FILE * fp, char *buffer, char * buffer1, char * buffer2, int *startptr, int * reading, int * toRead)
{
	int ptr = *startptr;
	if(buffer[ptr] == '*')
	{
		
		//The first character is *. 
		//I need to look ahead now.
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);

		if(buffer[ptr] != '*')
		{
			printf("*");
			*toRead = *reading;
			*startptr = ptr;
			return;
		}
		else
		{
			//Since the next is also a *, start the comments. 
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);

			// Untill my program ends, look for end of comments  
			while(buffer[ptr] != '\0')
			{
				// I am looking for **. Ignore the rest.
				if(buffer[ptr] != '*' &&  buffer[ptr] != '\n')
				{
					incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
					continue;
				}

				// If the line changes, move on but increment line count.
				else if(buffer[ptr] == '\n')
				{
					printf("\n");
					incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
					continue;		
				}

				// Encountered a *. Look for one more.
				else if(buffer[ptr] == '*')
				{
					incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
					if(buffer[ptr] == '*')	//Comment terminated
					{
						// Finally return NULL.
						incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
						*toRead = *reading;
						*startptr = ptr;
						return;
					}					
				}
			}
		}
	}
	else
	{
		printf("%c",buffer[ptr]);
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		*toRead = *reading;
		*startptr = ptr;
	}
}


Token * nextToken(FILE * fp, char *buffer, char * buffer1, char * buffer2, int *startptr, int * reading, int * toRead)
{
	//toRead is the current buffer (with which this function is called).
	//It may change. This change is reflected in reading
	
	//NOTE: Combination of reading and ptr together defines the buffer
	int ptr = *startptr; 
	char *lexeme = NULL;
	char *tokenName = NULL;
	Token *tkn = (Token*) malloc(sizeof(Token));
	int state = 0;

	while(1)
	{
		switch(state)
		{
			case 0: //start state of the DFA
				switch (buffer[ptr])
				{
					case '=':
						state = 1;
						break;
					
					case '[':
						state = 3;
						break;

					case ']':
						state = 4;
						break;

					case '(':
						state = 5;
						break;
						
					case ')':
						state = 6;
						break;	
						
					case '\n':
						state = 9;
						break;
						
					case '+':
						state = 13;
						break;
						
					case '-':
						state = 15;
						break;
						
					case '/':
						state = 14;
						break;
						
					case '*':
						state = 16;
						break;
						
					case ',':
						state = 21;
						break;
						
					case '<':
						state = 22;
						break;
						
					case '>':
						state = 26;
						break;
						
					case '!':
						state = 38;
						break;
						
					case ':':
						state = 40;
						break;
						
					case '.':
						state = 43;
						break;
						
					case ';':
						state = 45;
						break;

					default:
						//munch the space and other spatial delimiters
						if(buffer[ptr]==' ' || buffer[ptr]=='\t')
							state = 7;
						//check whether you should start lexically parsing the ID
						else if(isalpha(buffer[ptr]))
							state = 10;
						//check if you should start lexically parsing the NUM or RNUM
						else if(isdigit(buffer[ptr]))
							state = 30;
						else
						{
							state=-1;
						}
						break;
				}
				break;
			
			case -1: 
				//ERROR
				tkn = (Token*) malloc(sizeof(Token));
				tokenName = (char *)malloc(sizeof(char)*4);
				tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
				char * lexeme = (char *)malloc(sizeof(char)*70);
				memset(lexeme, '\0', sizeof(char)*70);
				sprintf(tkn->lexeme, "%c is not a valid language alphabet", buffer[ptr]);
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *) NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
				
			case 1: //received a single = operator
				if(buffer[ptr]=='=')
					state = 2;
				else
				{
					//ERROR
					tkn = (Token*) malloc(sizeof(Token));
					tokenName = (char *)malloc(sizeof(char)*4);
					tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
					char * lexeme = (char *)malloc(sizeof(char)*50);
					memset(lexeme, '\0', sizeof(char)*50);
					strcpy(lexeme, "Expected == or :=, got =");
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *) NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;
				}
				break;
				
			case 2: //parse == operator
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*3);
				lexeme[0] = '='; lexeme[1] = '='; lexeme[2] = '\0';
				tokenName = (char *)malloc(sizeof(char)*3);
				strcpy(tokenName, "EQ");
				tokenName[2] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
				
			case 3: //parse [ character
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = '['; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*5);
				strcpy(tokenName, "SQBO");
				tokenName[4] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return
				 tkn;
				break;
				
			case 4: //parse ] character
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = ']'; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*5);
				strcpy(tokenName, "SQBC");
				tokenName[4] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
				
			case 5: //parse the ( character
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = '(';
				lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*3);
				strcpy(tokenName, "BO");
				tokenName[2] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
				
			case 6: //parse the ) character
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = ')'; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*3);
				strcpy(tokenName, "BC");
				tokenName[2] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;				
				break;
				
			case 7: //blank was encountered
			
				//stay in this state to munch all the blank and tab symbols
				if(buffer[ptr]==' ' || buffer[ptr]=='\t')
					state = 7;
				else
				{
					*toRead = *reading;
					*startptr = ptr;
					return NULL;
				}
				break;
				
			case 9: // encountered a \n
				lineNum++;
				*toRead = *reading;
				*startptr = ptr; 
				return NULL;
				
			case 10: //encountered an alphabet
				 //stay here until alnum now, but start was due to alphabet only
				 //because there was no other way to enter the state 10 from 0 (start) 
				if(isalnum(buffer[ptr]) || buffer[ptr]=='_')
					state = 10;
				else
                {
					lexeme = fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr);
                    if(strlen(lexeme)>20)
                    {
                        //ERROR
						tkn ->lexeme = (char *)malloc(sizeof(char)*100);
						memset(tkn->lexeme, '\0', sizeof(char)*100);
						char * errorLexeme = fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr);
						tkn->token = (char *)malloc(sizeof(char)*4);
						sprintf(tkn->lexeme, "%s variable is longer than 20 characters", errorLexeme);
						memset(tkn->token, '\0', sizeof(char)*4);
						strcpy(tkn->token, "ERR");
						tkn->lineNum = lineNum;
                        *toRead = *reading;
                        *startptr = ptr;
                        return tkn;
                    }
                    else
                    {
                        tkn = (Token*) malloc(sizeof(Token));
                        lexeme = fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr);
                        int lex_len = strlen(lexeme) + 1;
                        tkn->lexeme = (char *)malloc(sizeof(char)*lex_len);
                        strcpy(tkn->lexeme, lexeme);

                        if(keyhash_find(lexeme)==1)
                        {
                            tkn->token = (char*) malloc(sizeof(char) * lex_len);
                            uppertoken(lexeme); 
                            strcpy(tkn->token , lexeme);
                            tkn->value = NULL; tkn->lineNum = lineNum;
                        }
                        else
                        {
                            tkn->token = (char*) malloc(sizeof(char) * 3);
                            strcpy(tkn->token , "ID");
                            tkn->token[2]='\0';
                            tkn->value = NULL; tkn->lineNum = lineNum;
                        }

                        //reading is the buffer we are reading and ptr is the ending location of the lexeme in that buffer
                        //Now, make toRead same as reading, since we must start reading from the ptr
                        *toRead = *reading;
                        *startptr = ptr;
                        return tkn;
                    }
                }
				break;
				
			case 13: //encountered a + token
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = '+'; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*5);
				strcpy(tokenName, "PLUS");
				tokenName[4] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
				
			case 14: //encountered a / token
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = '/'; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*4);
				strcpy(tokenName, "DIV");
				tokenName[3] = '\0';
				tkn->lexeme = lexeme;
				tkn->lineNum = lineNum;
				tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
				
			case 15: //encountered a - token
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = '-'; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*6);
				strcpy(tokenName, "MINUS");
				tokenName[5] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
				
			case 16: //encountered a * token
				if(buffer[ptr]=='*')
					state = 18;
				else
				{
					tkn = (Token*) malloc(sizeof(Token));
					lexeme = (char *)malloc(sizeof(char)*2);
					lexeme[0] = '*';	lexeme[1] = '\0';
					tokenName = (char *)malloc(sizeof(char)*4);
					strcpy(tokenName, "MUL");
					tokenName[3] = '\0';
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *)NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;
				}
				break;
				
			case 18: //the comment has started
				
				// chack if the comment is about to end
				if(buffer[ptr]=='*')
					state = 19;
				//check if the line is incrementing
				else if(buffer[ptr]=='\n')
				{
					state = 18;
					lineNum++;
				}
				else if(buffer[ptr]=='\0')
				{
					//ERROR HANDLING: Comment not terminated
					
					
					tkn = (Token *)malloc(sizeof(Token));
					tokenName = (char *)malloc(sizeof(char)*4);
					tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
					char * lexeme = (char *)malloc(sizeof(char)*23);
					strcpy(lexeme, "Comment not terminated");		
					lexeme[22]='\0';
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *) NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;	
				}
				else
					state = 18;	
				break;
				
			case 19: //waiting for the comment to close, already encountered one closing *
				if(buffer[ptr]=='*')
				{
					state=20;
				}
				
				//nextline handling
				else if(buffer[ptr]=='\n')
				{
					state = 18;
					lineNum++;
				}
				else if(buffer[ptr]=='\0')
				{
					//ERROR HANDLING: Comment not terminated
					
					tkn = (Token *)malloc(sizeof(Token));
					tokenName = (char *)malloc(sizeof(char)*4);
					tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
					char * lexeme = (char *)malloc(sizeof(char)*23);
					strcpy(lexeme, "Comment not terminated");		
					lexeme[22]='\0';
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *) NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;	
				}	
				else
					state = 18;
				break;
				
			case 20: 
				*toRead = *reading;
				*startptr = ptr;
				return NULL;
				break;
				
				
			case 21://encountered a ',' symbol
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = ','; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*6);
				strcpy(tokenName, "COMMA");
				tokenName[5] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum;	tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
				
			case 22://encountered a < symbol
				if(buffer[ptr]=='<')
					state = 23;
				else if(buffer[ptr]=='=')
					state = 25;
				else 
				{
					tkn = (Token*) malloc(sizeof(Token));
					lexeme = (char *)malloc(sizeof(char)*2);
					lexeme[0] = '<';lexeme[1] = '\0';
					tokenName = (char *)malloc(sizeof(char)*3);
					strcpy(tokenName, "LT");
					tokenName[2] = '\0';
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *)NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;
				}		
				break;	
						
			case 23://encountered <<
				if(buffer[ptr]=='<')
					state = 24;
				else
				{
					tkn = (Token*) malloc(sizeof(Token));
					lexeme = (char *)malloc(sizeof(char)*3);
					lexeme[0] = '<'; lexeme[1] = '<'; lexeme[2] = '\0';
					tokenName = (char *)malloc(sizeof(char)*4);
					strcpy(tokenName, "DEF");
					tokenName[3] = '\0';
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *)NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;
				}
				break;
				
			case 24: //encountered <<<
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*4);
				lexeme[0] = '<'; lexeme[1] = '<'; lexeme[2] = '<'; lexeme[3] = '\0';
				tokenName = (char *)malloc(sizeof(char)*10);
				strcpy(tokenName, "DRIVERDEF");
				tokenName[9] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
				
			case 25: //encountered <=
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*3);
				lexeme[0] = '<'; lexeme[1] = '=';lexeme[2] = '\0';
				tokenName = (char *)malloc(sizeof(char)*3);
				strcpy(tokenName, "LE");
				tokenName[2] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;	
									
			case 26://encountered a >	
				if(buffer[ptr]=='>')
					state = 27;
				else if(buffer[ptr]=='=')
					state = 29;
				else 
				{
					tkn = (Token*) malloc(sizeof(Token));
					lexeme = (char *)malloc(sizeof(char)*2);
					lexeme[0] = '>';lexeme[1] = '\0';
					tokenName = (char *)malloc(sizeof(char)*3);
					strcpy(tokenName, "GT");
					tokenName[2] = '\0';
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *)NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;
				}		
				break;
				
			case 27://encountered a >>
				if(buffer[ptr]=='>')
					state = 28;
				else
				{
					tkn = (Token*) malloc(sizeof(Token));
					lexeme = (char *)malloc(sizeof(char)*3);
					lexeme[0] = '>'; lexeme[1] = '>'; lexeme[2] = '\0';
					tokenName = (char *)malloc(sizeof(char)*7);
					strcpy(tokenName, "ENDDEF");
					tokenName[6] = '\0';
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *)NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;
				}
				break;
				
			case 28://encountered a >>>
				tkn = (Token *)malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*4);
				lexeme[0] = '>'; lexeme[1] = '>'; lexeme[2] = '>'; lexeme[3] = '\0';
				tokenName = (char *)malloc(sizeof(char)*13);
				strcpy(tokenName, "DRIVERENDDEF");
				tokenName[12] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
				
			case 29://encountered a >=
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*3);
				lexeme[0] = '>'; lexeme[1] = '='; lexeme[2] = '\0';
				
				tokenName = (char *)malloc(sizeof(char)*3);
				strcpy(tokenName, "GE");
				tokenName[2] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;	
				break;	
						
			case 30://encountered a digit
			
				//encountered another digit
				if(isdigit(buffer[ptr]))
					state = 30;
					
				//encountered a decimal
				else if(buffer[ptr]=='.')
					state = 31;
				else
				{
					tkn = (Token*) malloc(sizeof(Token));
					lexeme = fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr);
					tokenName = (char *)malloc(sizeof(char)*4);
					strcpy(tokenName, "NUM");
					tokenName[3] = '\0';

					int * result = (int *)malloc(sizeof(int));
					*result = lexeme2int(lexeme);

					tkn->lexeme = lexeme; tkn->token = tokenName; 
					tkn->value = (void *)result; tkn->lineNum = lineNum;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;		
				}
				break;
				
			case 31: //encountered a decimal in numbers
				if(isdigit(buffer[ptr]))
					state = 32;
					
				// This is specially handled retraction
				//we decrement the pointer which has moved one step ahead.
				else if(buffer[ptr]=='.')
				{
					tkn = (Token*) malloc(sizeof(Token));
					decrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
					lexeme = fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr);
					tokenName = (char *)malloc(sizeof(char)*4);
					strcpy(tokenName, "NUM");
					tokenName[3] = '\0';

					int * result = (int *)malloc(sizeof(int));
					*result = lexeme2int(lexeme);
					tkn->lexeme = lexeme;
					tkn->token = tokenName;
					tkn->value = (void *)result;
					tkn->lineNum = lineNum;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;
				}
				else
				{
					//ERROR

					tkn = (Token*) malloc(sizeof(Token));
					tokenName = (char *)malloc(sizeof(char)*4);
					tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
					char * lexeme = (char *)malloc(sizeof(char)*200);
					memset(lexeme, '\0', sizeof(char)*200);
					sprintf(lexeme, "Expected NUM after (.) decimal point at %s", fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr));
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *) NULL;	
					*toRead = *reading;
					*startptr = ptr;
					return tkn;					
				}
				break;		
				
			case 32://digit after decimal
				if(isdigit(buffer[ptr]))
					state = 32;
					
				//exponential RNUM or not
				else if(buffer[ptr]=='E' || buffer[ptr]=='e')
					state = 33;
				else
				{
					lexeme = fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr);
					tkn = (Token *)malloc(sizeof(Token));
					tokenName = (char *)malloc(sizeof(char)*5);
					strcpy(tokenName, "RNUM");
					tokenName[4] = '\0';
					
					//lexeme2real is returning a float value. So take it and cast to a void*
					tkn->lexeme = lexeme; tkn->token = tokenName;
					float * var = (float *)malloc(sizeof(float));
					*var = lexeme2real(lexeme);	tkn->value = (void *) var; tkn->lineNum = lineNum;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;					
				}		
				break;
				
			case 33://after the exponent E or e
				if(buffer[ptr]=='+' || buffer[ptr]=='-')
					state = 34;
				else if(isdigit(buffer[ptr]))
					state = 35;
				else
				{
					//ERROR
					
					tkn = (Token*) malloc(sizeof(Token));
					tokenName = (char *)malloc(sizeof(char)*4);
					tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
					char * lexeme = (char *)malloc(sizeof(char)*100);
					memset(lexeme, '\0', sizeof(char)*100);
					sprintf(lexeme, "Expected a number or +/- after e/E exp symbol at %s", fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr));		
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *) NULL;	
					*toRead = *reading;
					*startptr = ptr;
					return tkn;				
				}
				break;
				
			case 34://signed exponent has been found
				if(isdigit(buffer[ptr]))
					state = 35;
				else
				{
					//ERROR
					
					tkn = (Token*) malloc(sizeof(Token));
					tokenName = (char *)malloc(sizeof(char)*4);
					tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
					char * lexeme = (char *)malloc(sizeof(char)*100);
					memset(lexeme, '\0', sizeof(char)*100);
					sprintf(lexeme, "Expected a number after [e/E][+/-] exponent symbols at %s", fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr));	
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *) NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;					
				}
				break;
				
			case 35://power digits re being prsed now
				if(isdigit(buffer[ptr]))
					state = 35;
				else
				{
					lexeme = fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr);
					tkn = (Token *)malloc(sizeof(Token));
					tokenName = (char *)malloc(sizeof(char)*5);
					strcpy(tokenName, "RNUM");
					tokenName[4] = '\0';
					
					//lexeme2real is returning a float value. So take it and cast to a void*
					tkn->lexeme = lexeme; tkn->token = tokenName;
					float * var = (float *)malloc(sizeof(float));
					*var = lexeme2real(lexeme);	tkn->value = (void *) var; tkn->lineNum = lineNum;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;
				}
				break;
								
			case 38://Encountered a ! symbol
				if(buffer[ptr]=='=')
					state = 39;
				else
				{
					//ERROR
					
					tkn = (Token*) malloc(sizeof(Token));
					tokenName = (char *)malloc(sizeof(char)*4);
					tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
					char * lexeme = (char *)malloc(sizeof(char)*100);
					memset(lexeme, '\0', sizeof(char)*100);
					strcpy(lexeme, "Expected !=, got !");		
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *) NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;					
				}
				break;
				
			case 39:// != (NE) symbol is prsed now
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *) malloc(sizeof(char)*3);
				lexeme[0] = '!'; lexeme[1] = '='; lexeme[2] = '\0';
				tokenName = (char *)malloc(sizeof(sizeof(char)*3));
				strcpy(tokenName, "NE");
				tokenName[2] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum;	tkn->token = tokenName;
				tkn->value = (void *) NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;				
				break;
				
			case 40://encountered a : symbol
				if(buffer[ptr]=='=')
					state = 41;
				else
				{
					tkn = (Token*) malloc(sizeof(Token));
					lexeme = (char *)malloc(sizeof(char)*2);
					lexeme[0] = ':'; lexeme[1] = '\0';
					tokenName = (char *)malloc(sizeof(char)*6);
					strcpy(tokenName, "COLON");
					tokenName[5] = '\0';
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *)NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;									
				}
				break;
				
			case 41://encountered := symbol
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*3);
				lexeme[0] = ':'; lexeme[1] = '='; lexeme[2] = '\0';
				tokenName = (char *)malloc(sizeof(char)*9);
				strcpy(tokenName, "ASSIGNOP");
				tokenName[8] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;	
				*toRead = *reading;
				*startptr = ptr;
				return tkn;					
				break;
				
			case 43://encountered a '.' symbol
				if(buffer[ptr]=='.')
					state = 44;
				else
				{
					//ERROR
					
					tkn = (Token*) malloc(sizeof(Token));
					tokenName = (char *)malloc(sizeof(char)*4);
					tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
					char * lexeme = (char *)malloc(sizeof(char)*100);
					memset(lexeme, '\0', sizeof(char)*100);
					strcpy(lexeme, "Expected .., got .");		
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *) NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;												
				}
				break;
				
			case 44:// encountered a '..' symbol
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*3);
				lexeme[0] = '.'; lexeme[1] = '.'; lexeme[2] = '\0';
				tokenName = (char *)malloc(sizeof(char)*8);
				strcpy(tokenName, "RANGEOP");
				tokenName[7] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
				
			case 45://encountered a ';' symbol
				tkn = (Token*) malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = ';';	lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*8);
				strcpy(tokenName, "SEMICOL");
				tokenName[7] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum;	tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;	
				break;			
				
			default:
				*toRead = *reading;
				*startptr = ptr;
				return NULL;
				break;													
		}
		
		//finally increment the buffer pointer
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
	}
	return NULL;
}
