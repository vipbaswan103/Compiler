#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<ctype.h>

#define HASHSIZE 1000
#define RULESIZE 1000
#define NTSIZE 60

/**************************************LEXER********************************************************/
#define BUFFERSIZE 35

char *keyhash[32] ;


void uppertoken(char *str)
{
	int i=0;
	while(str[i] != '\0')
	{
		str[i] = toupper(str[i]);
		i++;
	}
}


void populate_keyhash()
{
    
    keyhash[0] = (char*) malloc(sizeof(char)* (8) ) ;
    strcpy(keyhash[0], "integer") ;
    keyhash[1] = (char*) malloc(sizeof(char)* (5) ) ;
    strcpy(keyhash[1], "real"); 
    keyhash[2] = (char*) malloc(sizeof(char)* (8) ) ;
    strcpy(keyhash[2], "boolean"); 
    keyhash[3] = (char*) malloc(sizeof(char)* (3) ) ;
    strcpy(keyhash[3], "of") ;
    keyhash[4] = (char*) malloc(sizeof(char)* (6) ) ;
    strcpy(keyhash[4],"array") ;
    keyhash[5] = (char*) malloc(sizeof(char)* (6) ) ;
    strcpy(keyhash[5], "start") ;
    keyhash[6] = (char*) malloc(sizeof(char)* (4) ) ;
    strcpy(keyhash[6], "end") ;
    keyhash[7] = (char*) malloc(sizeof(char)* (7) ) ;
    strcpy(keyhash[7], "declare");  
    keyhash[8] = (char*) malloc(sizeof(char)* (7) ) ;
    strcpy(keyhash[8], "module") ;
    keyhash[9] = (char*) malloc(sizeof(char)* (7) ) ;
    strcpy(keyhash[9], "driver") ;
    keyhash[10] = (char*) malloc(sizeof(char)* (8) ) ;
    strcpy(keyhash[10], "program") ;
    keyhash[11] = (char*) malloc(sizeof(char)* (10) ) ;
    strcpy(keyhash[11], "get_value") ;
    keyhash[12] = (char*) malloc(sizeof(char)* (6) ) ;
    strcpy(keyhash[12], "print") ;
    keyhash[13] = (char*) malloc(sizeof(char)* (4) ) ;
    strcpy(keyhash[13], "use") ;
    keyhash[14] = (char*) malloc(sizeof(char)* (5) ) ;
    strcpy(keyhash[14], "with") ;
    keyhash[15] = (char*) malloc(sizeof(char)* (11) ) ;
    strcpy(keyhash[15], "parameters") ;
    keyhash[16] = (char*) malloc(sizeof(char)* (5) ) ;
    strcpy(keyhash[16], "true") ;
    keyhash[17] = (char*) malloc(sizeof(char)* (6) ) ;
    strcpy(keyhash[17], "false") ;
    keyhash[18] = (char*) malloc(sizeof(char)* (6) ) ;
    strcpy(keyhash[18], "takes") ;
    keyhash[19] = (char*) malloc(sizeof(char)* (6) ) ;
    strcpy(keyhash[19], "input") ;
    keyhash[20] = (char*) malloc(sizeof(char)* (8) ) ;
    strcpy(keyhash[20], "returns") ;
    keyhash[21] = (char*) malloc(sizeof(char)* (4) ) ;
    strcpy(keyhash[21], "AND") ;
    keyhash[22] = (char*) malloc(sizeof(char)* (3) ) ;
    strcpy(keyhash[22], "OR") ;
    keyhash[23] = (char*) malloc(sizeof(char)* (4) ) ;
    strcpy(keyhash[23], "for") ;
    keyhash[24] = (char*) malloc(sizeof(char)* (3) ) ;
    strcpy(keyhash[24], "in") ;
    keyhash[25] = (char*) malloc(sizeof(char)* (7) ) ;
    strcpy(keyhash[25], "switch") ;
    keyhash[26] = (char*) malloc(sizeof(char)* (5) ) ;
    strcpy(keyhash[26], "case") ;
    keyhash[27] = (char*) malloc(sizeof(char)* (6) ) ;
    strcpy(keyhash[27], "break") ;
    keyhash[28] = (char*) malloc(sizeof(char)* (8) ) ;
    strcpy(keyhash[28], "default") ;
    keyhash[29] = (char*) malloc(sizeof(char)* (6) ) ;
    strcpy(keyhash[29], "while") ;
    keyhash[30] = (char*) malloc(sizeof(char)* (10) ) ;
    strcpy(keyhash[30], "driverdef") ;
    keyhash[31] = (char*) malloc(sizeof(char)* (13) ) ;
    strcpy(keyhash[31], "driverenddef") ;
}

int keyhash_find(char * str)
{
    int i=0;
    while(i!=30)
    {
        if(strcmp(str,keyhash[i])==0)
        	return 1;
        i++;
    }
    return 0;
}


typedef struct
{
	char * token;
	char * lexeme;
	void * value;
	int lineNum;
} Token;

typedef struct 
{
	char * description;
	int lineNum;
} Lexical_Error;

typedef struct 
{
	Token ** Tokenized_code;
	int num_tokens;
} TokenStream;


int lineNum = 1;
// char *startPtr = NULL, *currentPtr = NULL;
// char *buffer;		//Twin Buffer
char *buffer1, *buffer2;
int startptr, reading, toRead;
FILE * fp;

Token * nextToken(FILE *, char *, char * , char *, int *, int *, int *);

void readBuffer(FILE *fp, char * buffer)
{
    memset(buffer, '\0', sizeof(char)*BUFFERSIZE);
	int read = fread(buffer,1,BUFFERSIZE,fp);
	if(ferror(fp))
	{
		printf("Error in reading the file\n");
		exit(-1);
	}
	// if(read<BUFFERSIZE)
	// 	buffer[read]='\0';
}

void initializeLexer(char * filename)
{
	buffer1 = (char *)malloc(sizeof(char) * BUFFERSIZE);
	buffer2 = (char *)malloc(sizeof(char) * BUFFERSIZE);
	startptr=0;
	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("Error opening file");
		return;
	}
	readBuffer(fp, buffer1);
	reading = 1;
	toRead = 1;
}

Token * getNextToken()	//read prog into buffer
{
	Token * tkn = NULL;
	char * buffer;
	if(toRead == 1)
		buffer = buffer1;
	else
		buffer = buffer2;
	
	while(buffer[startptr]!='\0')
	{
		tkn = nextToken(fp, buffer, buffer1, buffer2, &startptr, &reading, &toRead);

		//Update the buffer to one which we are supposed to read next (Needed if tkn is NULL)
		if(toRead == 1)
			buffer = buffer1;
		else
			buffer = buffer2;

		if(tkn == NULL) //If we encounter spaces or linenumbers, keep searching for some valid token
			continue;
		else 
		{
			if(strcmp(tkn->token,"ERR"))        //Found the valid token, return it to the parser
			{
				break;
			}
			else    //If lexical error, continue searching for the valid token
			{
				printf("LEXICAL ERROR (%d) :",tkn->lineNum);
				printf("%s\n",tkn->lexeme); 
                tkn = NULL;
			}
		}
    }

    if(tkn == NULL && buffer[startptr] == '\0') //We didn't get any valid token but we have reached the end of the input, return EOF
    {
        tkn = (Token *)malloc(sizeof(Token));
        tkn->lexeme = (char *)malloc(sizeof(char)*4);
        strcpy(tkn->lexeme, "EOF");
        tkn->token = (char *)malloc(sizeof(char)*4);
        strcpy(tkn->token, "EOF");
        tkn->lineNum = lineNum;
    }
	return tkn;
}

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

float lexeme2real(char *s)
{
	int i=0, j=0, flag=0, sign=1, exp_pow=0;
	float val=0.0, frac=0.0, exp=0.0;

	while(s[i]!='\0')
	{
		
		if(s[i]=='.')
		{
			flag = 1;
			i++;
		}
		else if(s[i]=='E' || s[i] == 'e')
		{
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
		
		if(flag==0)
		{
			val = val*10 + (s[i]-'0');	
		}
		else if(flag==1)
		{
			j++;
			frac = frac + pow(10,(-1)*j)*(s[i]-'0');							
		}
		else if(flag==2)
		{
			exp_pow = exp_pow*10 + (s[i]-'0');
		}

		i++ ;
	}
	
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

Token * nextToken(FILE * fp, char *buffer, char * buffer1, char * buffer2, int *startptr, int * reading, int * toRead)
{
	//toRead is the current buffer (with which this function is called).
	//It may change. This change is reflected in reading

	int ptr = *startptr; //NOTE: Combination of reading and ptr together defines the buffer
	char *lexeme = NULL;
	Token *tkn = (Token*) malloc(sizeof(Token));

	if(isalpha(buffer[ptr]))
	{
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		while(buffer[ptr] != '\0' && (isalnum(buffer[ptr]) || buffer[ptr] == '_'))
		{
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		}

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
			tkn->value = NULL; tkn->lineNum = lineNum;
		}

		//reading is the buffer we are reading and ptr is the ending location of the lexeme in that buffer
		//Now, make toRead same as reading, since we must start reading from the ptr
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}

	/* 	SPACES	*/

	else if(buffer[ptr] == ' ' || buffer[ptr] == '\t')
	{
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		//Ignoring as many spaces in a loop
		while(buffer[ptr] == ' ' || buffer[ptr] == '\t')
		{
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		}
		//We will start from the next non space/tab character at the beginning of the DFA
		*toRead = *reading;
		*startptr = ptr;
		return NULL;
	}
	
	/* 	NEWLINE	*/

	else if(buffer[ptr] == '\n')
	{
		//Ignoring as many newlines, just keep incrementing the line count
		while(buffer[ptr] == '\n')
		{
			lineNum++; 
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		}
		//We will start at the beginning of the DFA, from a non new line character
		*toRead = *reading;
		*startptr = ptr; 
		return NULL;
	}

	/* 	PLUS */
	
	else if(buffer[ptr] == '+')
	{
		// printf("Inside +\n");
		//Simple single layer condition, no branches, no ambiguity

		tkn = (Token *)malloc(sizeof(Token));
		lexeme = (char *)malloc(sizeof(char)*2);
		lexeme[0] = '+'; lexeme[1] = '\0';
		char * tokenName = (char *)malloc(sizeof(char)*5);
		strcpy(tokenName, "PLUS");
		tokenName[4] = '\0';
		tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
		tkn->value = (void *)NULL;
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}

	/* 	MINUS */

	else if(buffer[ptr] == '-')
	{
		//Simple single layer condition, no branches, no ambiguity
		// printf("Minus\n");
		tkn = (Token *)malloc(sizeof(Token));
		lexeme = (char *)malloc(sizeof(char)*2);
		lexeme[0] = '-'; lexeme[1] = '\0';
		char * tokenName = (char *)malloc(sizeof(char)*6);
		strcpy(tokenName, "MINUS");
		tokenName[5] = '\0';
		tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
		tkn->value = (void *)NULL;
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		*toRead = *reading;
		*startptr = ptr;
		// printf("%s\t%d\n",tkn->lexeme, tkn->lineNum);
		return tkn;
	}

	/* DIV */
	
	else if(buffer[ptr] == '/')
	{
		//Simple single layer condition, no branches, no ambiguity
		tkn = (Token *)malloc(sizeof(Token));
		lexeme = (char *)malloc(sizeof(char)*2);
		lexeme[0] = '/'; lexeme[1] = '\0';
		char * tokenName = (char *)malloc(sizeof(char)*4);
		strcpy(tokenName, "DIV");
		tokenName[3] = '\0';
		tkn->lexeme = lexeme;
		tkn->lineNum = lineNum;
		tkn->token = tokenName;
		tkn->value = (void *)NULL;
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}

	/* MUL and COMMMENTS */

	else if(buffer[ptr] == '*')
	{
		
		//The first character is *. 
		//I need to look ahead now.
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);

		if(buffer[ptr] != '*')
		{
			//If the next is not a *. 
			//Treat the previous * as a multiply
			tkn = (Token *)malloc(sizeof(Token));
			lexeme = (char *)malloc(sizeof(char)*2);
			lexeme[0] = '*';	lexeme[1] = '\0';
			char * tokenName = (char *)malloc(sizeof(char)*4);
			strcpy(tokenName, "MUL");
			tokenName[3] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
			*toRead = *reading;
			*startptr = ptr;
			return tkn;
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
					lineNum++;
					incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
					continue;		
				}

				// Encountered a *. Look for one more.
				else if(buffer[ptr] == '*')
				{
					incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
					if(buffer[ptr] == '*')
					{
						// Finally return NULL.
						incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
						*toRead = *reading;
						*startptr = ptr;
						return NULL;
					}
				}
			}
			
			if(buffer[ptr]=='\0')
			{
				//ERROR HANDLING: Comment not terminated
				tkn = (Token *)malloc(sizeof(Token));
				char * tokenName = (char *)malloc(sizeof(char)*4);
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
			

			//DEAD CODE, verify once
			// Finally set the start to current pointer because here the comment terminated 
			*toRead = *reading;
			*startptr = ptr;
			return NULL;
		}
	}

	/* SQBO */
	
	else if(buffer[ptr] == '[')
	{
		// Single branch, no branches, no ambiguity
		tkn = (Token *)malloc(sizeof(Token));
		lexeme = (char *)malloc(sizeof(char)*2);
		lexeme[0] = '['; lexeme[1] = '\0';
		char * tokenName = (char *)malloc(sizeof(char)*5);
		strcpy(tokenName, "SQBO");
		tokenName[4] = '\0';
		tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
		tkn->value = (void *)NULL;
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}
	
	/* SQBC */
	
	else if(buffer[ptr] == ']')
	{
		// Single branch, no branches, no ambiguity
		tkn = (Token *)malloc(sizeof(Token));
		lexeme = (char *)malloc(sizeof(char)*2);
		lexeme[0] = ']'; lexeme[1] = '\0';
		char * tokenName = (char *)malloc(sizeof(char)*5);
		strcpy(tokenName, "SQBC");
		tokenName[4] = '\0';
		tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
		tkn->value = (void *)NULL;
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}

	/* BO */

	else if(buffer[ptr] == '(')
	{
		// Single branch, no branches, no ambiguity
		
		tkn = (Token *)malloc(sizeof(Token));
		lexeme = (char *)malloc(sizeof(char)*2);
		lexeme[0] = '(';
		lexeme[1] = '\0';
		char * tokenName = (char *)malloc(sizeof(char)*3);
		strcpy(tokenName, "BO");
		tokenName[2] = '\0';
		tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
		tkn->value = (void *)NULL;
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}

	/* BC */
	
	else if(buffer[ptr] == ')')
	{
		// Single branch, no branches, no ambiguity
		tkn = (Token *)malloc(sizeof(Token));
		lexeme = (char *)malloc(sizeof(char)*2);
		lexeme[0] = ')'; lexeme[1] = '\0';
		char * tokenName = (char *)malloc(sizeof(char)*3);
		strcpy(tokenName, "BC");
		tokenName[2] = '\0';
		tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
		tkn->value = (void *)NULL;
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}
	
	/* NOT EQUAL */
	
	else if(buffer[ptr] == '!')
	{
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		if(buffer[ptr]=='=')
		{
			tkn = (Token *)malloc(sizeof(Token)*3);
			lexeme[0] = '!'; lexeme[1] = '='; lexeme[2] = '\0';
			char * tokenName = (char *)sizeof(Token);
			lexeme = (char *) malloc(sizeof(char)*3);
			strcpy(tokenName, "NE");
			tokenName[2] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum;	tkn->token = tokenName;
			tkn->value = (void *) NULL;
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		}
		else //There is an error 
		{
			tkn = (Token *)malloc(sizeof(Token));
			char * tokenName = (char *)malloc(sizeof(char)*4);
			tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
			char * lexeme = (char *)malloc(sizeof(char)*19);
			strcpy(lexeme, "Expected !=, got !");		
			lexeme[18]='\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *) NULL;
		}
		
		//In case there was an error, I am at the char after ! symbol
		//Else I am after = in !=
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}
	
	/* Branch with = */
	
	else if(buffer[ptr] == '=')
	{
		tkn = (Token *)malloc(sizeof(Token));
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		if(buffer[ptr] == '=')
		{
			// EQ token
			
			lexeme = (char *)malloc(sizeof(char)*3);
			lexeme[0] = '='; lexeme[1] = '='; lexeme[2] = '\0';
			char *tokenName = (char *)malloc(sizeof(char)*3);
			strcpy(tokenName, "EQ");
			tokenName[2] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		}
		else
		{
			//Error because = is nothing else.

			tkn = (Token *)malloc(sizeof(Token));
			char * tokenName = (char *)malloc(sizeof(char)*4);
			tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
			char * lexeme = (char *)malloc(sizeof(char)*19);
			strcpy(lexeme, "Expected ==, got =");
			lexeme[18]='\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum;	tkn->token = tokenName;
			tkn->value = (void *) NULL;		
		}

		// my pointer points at the next character that was not =. 
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}	

	/* COMMA */

	else if(buffer[ptr] == ',')
	{
		// Single branch, no branches, no ambiguity
		tkn = (Token *)malloc(sizeof(Token));
		lexeme = (char *)malloc(sizeof(char)*2);
		lexeme[0] = ','; lexeme[1] = '\0';
		char * tokenName = (char *)malloc(sizeof(char)*6);
		strcpy(tokenName, "COMMA");
		tokenName[5] = '\0';
		tkn->lexeme = lexeme; tkn->lineNum = lineNum;	tkn->token = tokenName;
		tkn->value = (void *)NULL;
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}


	else if(buffer[ptr] == '<')
	{
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		if(buffer[ptr] == '=')
		{
			// Less than equal to

			tkn = (Token *)malloc(sizeof(Token));
			lexeme = (char *)malloc(sizeof(char)*3);
			lexeme[0] = '<'; lexeme[1] = '=';lexeme[2] = '\0';
			
			char * tokenName = (char *)malloc(sizeof(char)*3);
			strcpy(tokenName, "LE");
			tokenName[2] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
			*toRead = *reading;
			*startptr = ptr;
			return tkn;
 		}
		else if(buffer[ptr] == '<')
		{
			// Definition << Symbol
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
			if(buffer[ptr] == '<')
			{
				tkn = (Token *)malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*4);
				lexeme[0] = '<'; lexeme[1] = '<'; lexeme[2] = '<'; lexeme[3] = '\0';
				char * tokenName = (char *)malloc(sizeof(char)*10);
				strcpy(tokenName, "DRIVERDEF");
				tokenName[9] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;

			}
			else
			{
				tkn = (Token *)malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*3);
				lexeme[0] = '<'; lexeme[1] = '<'; lexeme[2] = '\0';
				char * tokenName = (char *)malloc(sizeof(char)*4);
				strcpy(tokenName, "DEF");
				tokenName[3] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
			}
		}
		else
		{
			// Branch for Less than relational operator

			tkn = (Token *)malloc(sizeof(Token));
			lexeme = (char *)malloc(sizeof(char)*2);
			lexeme[0] = '<';lexeme[1] = '\0';
			
			char * tokenName = (char *)malloc(sizeof(char)*3);
			strcpy(tokenName, "LT");
			tokenName[2] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
			*toRead = *reading;
			*startptr = ptr;
			return tkn;
		}
	}
	else if(buffer[ptr] == ';')
	{
		// Single branch, no branches, no ambiguity
		tkn = (Token *)malloc(sizeof(Token));
		lexeme = (char *)malloc(sizeof(char)*2);
		lexeme[0] = ';';	lexeme[1] = '\0';
		char * tokenName = (char *)malloc(sizeof(char)*8);
		strcpy(tokenName, "SEMICOL");
		tokenName[7] = '\0';
		tkn->lexeme = lexeme; tkn->lineNum = lineNum;	tkn->token = tokenName;
		tkn->value = (void *)NULL;
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}
	else if(buffer[ptr] == '>')
	{
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		if(buffer[ptr] == '=')
		{
			// Greater than equal to
			tkn = (Token *)malloc(sizeof(Token));
			lexeme = (char *)malloc(sizeof(char)*3);
			lexeme[0] = '>'; lexeme[1] = '='; lexeme[2] = '\0';
			
			char * tokenName = (char *)malloc(sizeof(char)*3);
			strcpy(tokenName, "GE");
			tokenName[2] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
			*toRead = *reading;
			*startptr = ptr;
			return tkn;
 		}
		else if(buffer[ptr] == '>')
		{
			//Definition ending >>
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
			if(buffer[ptr] == '>')
			{
				incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				tkn = (Token *)malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*4);
				lexeme[0] = '>'; lexeme[1] = '>'; lexeme[2] = '>'; lexeme[3] = '\0';
				char * tokenName = (char *)malloc(sizeof(char)*13);
				strcpy(tokenName, "DRIVERENDDEF");
				tokenName[12] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
			}
			else
			{
				tkn = (Token *)malloc(sizeof(Token));
				lexeme = (char *)malloc(sizeof(char)*3);
				lexeme[0] = '>'; lexeme[1] = '>'; lexeme[2] = '\0';
				char * tokenName = (char *)malloc(sizeof(char)*7);
				strcpy(tokenName, "ENDDEF");
				tokenName[6] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
			}
		}
		else
		{
			//Greater than
			tkn = (Token *)malloc(sizeof(Token));
			lexeme = (char *)malloc(sizeof(char)*2);
			lexeme[0] = '>'; lexeme[1] = '\0';
			char * tokenName = (char *)malloc(sizeof(char)*3);
			strcpy(tokenName, "GT");
			tokenName[2] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
			*toRead = *reading;
			*startptr = ptr;
			return tkn;
		}
	}

	//Number parsing

	else if(isdigit(buffer[ptr]))
	{
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		while(isdigit(buffer[ptr]))
		{
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		}
		tkn = (Token *)malloc(sizeof(Token));
		if(buffer[ptr] == '.') //<digits>.
		{
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
			if(buffer[ptr] == '.')	//<digits>..
			{
				decrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);

				lexeme = fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr);
				char * tokenName = (char *)malloc(sizeof(char)*4);
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
			else if(isdigit(buffer[ptr]))	//<digits>.<digits>
			{
				//Post decimal number has started
				incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				while(isdigit(buffer[ptr]))
				{
					incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				}
				if(buffer[ptr] == 'E' || buffer[ptr] == 'e')		//<digits>.<digits>E
				{
					//The power of 10 is being parsed
					incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
					if(isdigit(buffer[ptr]))	//<digits>.<digits>E<digits>
					{
						//There is a number after the E symbol
						incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
						while(isdigit(buffer[ptr]))
						{
							incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
						}
					}
					else if(buffer[ptr] == '+' || buffer[ptr] == '-')	//<digits>.<digits>E<+ or ->
					{
						//There is a sign after the E symbol
						incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
						if(isdigit(buffer[ptr]))	//<digits>.<digits>E<+ or -><digits>
						{
							incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
							while(isdigit(buffer[ptr]))
							{
								incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
							}
						}
						else		//E+<nonDigit>, should be an error
						{
							//There should have been a number after E+ or E-
							tkn = (Token *)malloc(sizeof(Token));
							char * tokenName = (char *)malloc(sizeof(char)*4);
							tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
							char * lexeme = (char *)malloc(sizeof(char)*28);
							strcpy(lexeme, "Expected NUM after E+ or E-");		
							lexeme[27]='\0';
							tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
							tkn->value = (void *) NULL;
							*toRead = *reading;
							*startptr = ptr;
							return tkn;
						}
					}
					else 
					{
						//There is no number or +- after the E symbol. So it's an error
						tkn = (Token *)malloc(sizeof(Token));
						char * tokenName = (char *)malloc(sizeof(char)*4);
						tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
						char * lexeme = (char *)malloc(sizeof(char)*31);
						strcpy(lexeme, "Expected NUM or + or - after E");		
						lexeme[30]='\0';
						tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
						tkn->value = (void *) NULL;	
						*toRead = *reading;
						*startptr = ptr;
						return tkn;
					} 
				}
				//We come here in following ways:
				//1) We have encountered some number as: <digits>.<digits>
				//2) We have encountered some number as: <digits>.<digits>E<digits>
				//3) We have encountered some number as: <digits>.<digits>E<+ or ->
				//4) We have encountered some number as:<digits>.<digits>E<+ or -><digits>
				
				//The above 4 cases cover all the possible cases for real numbers

				lexeme = fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr);
				tkn = (Token *)malloc(sizeof(Token));
				char * tokenName = (char *)malloc(sizeof(char)*5);
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
			else	//<digits>.<not a . or a number>, this is an error
			{
				//We have a number, then a dot, but after that no dot or number
				//So num.nonumdot
				//Hence error
				tkn = (Token *)malloc(sizeof(Token));
				char * tokenName = (char *)malloc(sizeof(char)*4);
				tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
				char * lexeme = (char *)malloc(sizeof(char)*21);
				strcpy(lexeme, "Expected NUM after .");		
				lexeme[20]='\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *) NULL;	
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
			}
		}
		else //<digits>
		{
			lexeme = fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr);
			char * tokenName = (char *)malloc(sizeof(char)*4);
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
	}
	
	else if(buffer[ptr] == '.')
	{
		tkn = (Token *)malloc(sizeof(Token));
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		if(buffer[ptr]=='.')
		{
			
			// Found another dot.
			lexeme = (char *)malloc(sizeof(char)*3);
			lexeme[0] = '.'; lexeme[1] = '.'; lexeme[2] = '\0';
			char *tokenName = (char *)malloc(sizeof(char)*8);
			strcpy(tokenName, "RANGEOP");
			tokenName[7] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		}
		else
		{
			// A single dot should not have occured
			tkn = (Token *)malloc(sizeof(Token));
			char * tokenName = (char *)malloc(sizeof(char)*4);
			tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
			char * lexeme = (char *)malloc(sizeof(char)*19);
			strcpy(lexeme, "Expected .., got .");		
			lexeme[18]='\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *) NULL;		
		}
		// my pointer is at the next symbol not a dot
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}
	else if(buffer[ptr] == ':')
	{
		tkn = (Token *)malloc(sizeof(Token));
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		if( buffer[ptr]=='=')
		{
			//  Asignment := operator
			lexeme = (char *)malloc(sizeof(char)*3);
			lexeme[0] = ':'; lexeme[1] = '='; lexeme[2] = '\0';
			char *tokenName = (char *)malloc(sizeof(char)*9);
			strcpy(tokenName, "ASSIGNOP");
			tokenName[8] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
		}
		else
		{
			// Colon : simple branch
			lexeme = (char *)malloc(sizeof(char)*2);
			lexeme[0] = ':'; lexeme[1] = '\0';
			char *tokenName = (char *)malloc(sizeof(char)*6);
			strcpy(tokenName, "COLON");
			tokenName[5] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
		}
		
		//pointer at the next symbol after the definitely parsed branches, no error.
		*toRead = *reading;
		*startptr = ptr;
		return tkn;
	}

	// Only possible case is <number><some alphabet from where there is no branch on start state>
	// This is an error

	return NULL;
}

void printToken(Token * tkn)
{
	printf("%s\t\t",tkn->lexeme);
	printf("%s\t\t",tkn->token);
	printf("%d",tkn->lineNum);

	if(strcmp(tkn->token, "NUM") == 0)
	{
		int * x = (int *) (tkn->value);
		printf("\t\t%d", *x);
	}
	else if(strcmp(tkn->token, "RNUM") == 0)
	{
		float * x = (float *) (tkn->value);
		printf("\t\t%lf", *x);
	}
	printf("\n");
}
/***************************************************************************************************/
int enumTerminal=0, enumNonTerminal=0, curr_size = 10, epsilonENUM;
char **enumToTerminal;
char **enumToNonTerminal;

typedef struct{
    char str[60];
    int enumcode;
} Terminal;

typedef struct{
    char str[60];
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
Hashtable * hash_tb;
//grammar
typedef struct
{
    int size;
    LinkedList * arr;
}Grammar;


void printGrammar(Grammar *);
void map(Grammar *);
void printMap();
void setOR(int *, int *);
//finding the hash of the string
int hash_func(char *str)
{
    int i = 0 ;
    long long val = 11 ;

    while(str[i] != '\0')
    {
        val = val*31 + str[i] ;
        i++ ;
    }
    if(val < 0)
        val = val * (-1);

    return val % HASHSIZE ;
}


Element* hash_find(char * str, Hashtable * hash_tb)
{
    int hash;
    hash = hash_func(str);
    
    Node * trav = hash_tb->arr[hash].head; 
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
        hash = hash_func(ele->type.nt.str);
    else if (ele->tag==2)
        hash = hash_func(ele->type.t.str);

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

void populateGrammarArray(Grammar * grammar, char * str, int TorNT, int index)
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

        newHead->ele.tag = 1;
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

void insertInLinkedList(Grammar * grammar, char * str, int TorNT, int index)
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

        ele.tag = TorNT;
        ele.type = type;

        newNode->ele = ele;
        newNode->next = NULL;
        tail->next = newNode;
        grammar->arr[index].tail = tail->next;

        hash_insert(&ele, hash_tb);        
    }
}

// Use this function to print the grammar as interpreted in the linked list array

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
    while(fscanf(fp,"%[^\r]\r\n",xyz) != EOF)
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
    for(int i=0; i<enumNonTerminal; i++)
    {
        firstSet[i] = (int *)malloc(sizeof(int)*(enumTerminal+3));
        memset(firstSet[i],0,sizeof(int)*(enumTerminal+3));
        firstSet[i][enumTerminal+1] = -1;  //Indicates whether we have calculated follow set or not, special
        firstSet[i][enumTerminal+2] = -1;  //Indicates whether we have traversed this NT before or not, special
    }
    return firstSet;
}


int * calculateFirstSet(Grammar *grammar, int nonTerminal, int ** firstSet)
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
    //We have already calculated the follow set
    if(followSet[nonTerminal][enumTerminal+1] == 1)    
        return followSet[nonTerminal];
    
    //Cycle detected
    //But now, don't recurse. Just return whatever has been collected in the followset till now
    //Don't return all zeros (if A -> B and B -> A, then A must have "atleast" what B has and vice-versa)
    if(followSet[nonTerminal][enumTerminal+2] == 1)     
    {
        // int * arr = (int *)malloc(sizeof(int)*(enumTerminal+3));
        // memset(arr, 0, sizeof(int)*(enumTerminal+3));

        return followSet[nonTerminal];      
    }
    followSet[nonTerminal][enumTerminal+2] = 1;
    for(int i=0; i<grammar->size; i++)
    {
        //start from the first element of RHS 
        Node * trav = grammar->arr[i].head->next;
        while(trav != NULL)
        {
            if(trav->ele.tag == 1)
            {
                if(trav->ele.type.nt.enumcode == nonTerminal)   //We found the matching non-terminal
                {
                    trav = trav->next;
                    if(trav == NULL)  //Rule is of type A -> XB
                    {
                        setOR(followSet[nonTerminal], calculateFollowSet(grammar, grammar->arr[i].head->ele.type.nt.enumcode, followSet, firstSet));
                    }
                    else    //Rule is of type A -> XBY
                    {
                        while(trav != NULL)
                        {
                            if(trav->ele.tag == 2)    //If Y is a terminal
                            {
                                followSet[nonTerminal][trav->ele.type.t.enumcode] = 1;
                                break;
                            }
                            else   //If Y is a non-terminal
                            {
                                setOR(followSet[nonTerminal], firstSet[trav->ele.type.nt.enumcode]);
                                if (firstSet[trav->ele.type.nt.enumcode][epsilonENUM] == 1) //First set contains EPSILON
                                {
                                    
                                }
                                else
                                {
                                    break;
                                }
                                
                            }
                            trav = trav->next;
                        }

                        if(trav == NULL)
                        {
                            setOR(followSet[nonTerminal], calculateFollowSet(grammar, grammar->arr[i].head->ele.type.nt.enumcode, followSet, firstSet));
                        }
                    }
                }
            }
            if(trav != NULL)
                trav = trav->next;
        }
    }
    //We are done with this non-terminal
    
    followSet[nonTerminal][enumTerminal+2] = 1;     //Marked as already traversed
    followSet[nonTerminal][enumTerminal+1] = 1;     //Mark 1 as follow set already calculated
    return followSet[nonTerminal];
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
                parseTable[grammar->arr[i].head->ele.type.nt.enumcode][j] = i;
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

typedef struct treenode
{
    Element ele;
    struct treenode * sibling;
    struct treenode * child;
}TreeNode;

TreeNode * createNode(Element * ele)
{
    TreeNode * newNode = (TreeNode *)malloc(sizeof(TreeNode));
    newNode->ele = *ele;
    newNode->sibling = NULL;
    newNode->child = NULL;
    return newNode;
}

TreeNode * siblingInsert(TreeNode * head, TreeNode * node)      //Pass the pointer to the node ("head") whose sibling has to be "node"
{                                                               //Returns a pointer to the head of siblings. Assign it back to the head of the siblings
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

void insert(TreeNode * parent, TreeNode * newNode)
{
    parent->child = siblingInsert(parent->child, newNode);
}

TreeNode * getNode(Element * ele)
{
    // the ele we have got from the unwrapped version of the grammar node
    //we then wrap it in Tree Node
    TreeNode * newNode = (TreeNode *)malloc(sizeof(TreeNode));
    
    newNode->child = NULL;
    newNode->child = NULL;
    newNode->ele = *ele;
    return newNode;
}
void preOrder(TreeNode * root)
{
    if(root == NULL)
    {
        return;
    }

    if(root->ele.tag == 1)
    {
        printf("%s (%d)", root->ele.type.nt.str, root->ele.type.nt.enumcode);
    }
    else 
    {
        printf("%s (%d)", root->ele.type.t.str, root->ele.type.t.enumcode);
    }

    // TreeNode * trav = root;
    // preOrder(trav->child);

    // trav = trav->sibling;
    // preOrder(trav);    

    TreeNode * trav = root->child;
    preOrder(trav);

    while(trav != NULL)
    {
        trav = trav->sibling;
        preOrder(trav);
    }
}

typedef struct stacknode
{
    TreeNode * trnode;
    struct stacknode * next;
}StackNode;

typedef struct{
    StackNode * top;
    int size ;
}Stack;



TreeNode * pop(Stack * st);
Stack * push(Stack * st, TreeNode * trNode);


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
        if(trav->trnode->ele.tag == 1)
            printf("%s (%d)\n", trav->trnode->ele.type.nt.str, trav->trnode->ele.type.nt.enumcode);
        else
            printf("%s (%d)\n", trav->trnode->ele.type.t.str, trav->trnode->ele.type.t.enumcode);
        trav = trav->next;
    }
}

/*******************************************************************/

char ** tempo_tokens;


TreeNode * parser(Grammar * grammar, int ** parsetable)
{
    TreeNode * parseTree = NULL;
    Element * ele1 = (Element *)malloc(sizeof(Element));
    Element * ele2 = NULL;

    ele1->tag = 2;
    strcpy(ele1->type.t.str, "EOF");
    ele1->type.t.enumcode = enumTerminal;
    TreeNode * eof = getNode(ele1);

    ele2 = &(grammar[0].arr->head->ele);
    parseTree = getNode(ele2);

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
    printf("%s\n", tkn->token);

    int j;
    while(mainStack->size != 0)
    {
        //top of the stack is terminal

        if(mainStack->top->trnode->ele.tag == 2)
        {

        }
        if(mainStack->top->trnode->ele.tag == 2)    
        {
            //get the next token from the lexer if the top of stack is a non terminal
            //and the input matched it
            if(strcmp(tkn->token, mainStack->top->trnode->ele.type.t.str) == 0)
            {
                node = pop(mainStack);
                // index++;
                // tkn = tempo_tokens[index];
                tkn = getNextToken();
                printf("%s\n", tkn->token);
            }
            //throw an error
            else
            {
                printf("\nSyntactical Error - Irrelevant occurance of %s", tkn->token);
                return NULL;
                //Syntax Error
            }
        }
        else // the top of the stack is a non terminal
        {
            //pop the NT and find its enumerated code in hash table
            node = pop(mainStack);

            ele = hash_find(tkn->token, hash_tb);

            if(ele == NULL && (strcmp(tkn->token,"EOF") != 0))
            {
                //Problem in grammar parsing
                printf(" \n The top of the stack was an unidentified terminal ! This error should not occur actually");
                return NULL;
            }
            else
            {
                if(strcmp(tkn->token, "EOF") == 0)
                    j = enumTerminal;
                else
                    j = ele->type.t.enumcode;
                
                ruleNum = parsetable[node->ele.type.nt.enumcode][j];
                Node * trav;
                if(ruleNum==(-1))
                {
                    //error
                    printf("No rule in parse table ! Parsing error");
                    return NULL;
                }
                trav = grammar->arr[ruleNum].head->next;
                
                if(trav->ele.tag == 2 && trav->ele.type.t.enumcode == epsilonENUM)
                {
                    TreeNode * pshNode = getNode(&(trav->ele));
                    insert(node, pshNode);
                    continue;
                }
                //Push the RHS of the rule on auxillary stack and also insert the node in the parse tree
                while(trav != NULL)
                {
                    TreeNode * pshNode = getNode(&(trav->ele));
                    //push in reverse manner on the auxiliary stack
                    auxillaryStack = push(auxillaryStack, pshNode);
                    //insert in that orser in the tree now itself  
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


void seeTokenization()
{
    Token * tkn = NULL;

    tkn = getNextToken();

    while(strcmp(tkn->token, "EOF")!=0)
    {
        printf("%s\n", tkn->token);
        tkn = getNextToken();
    }

    printf("%s\n", tkn->token);
}
/*******************************************************************/
int main(int argc, char * argv[])
{
    // if(argc != 3)
    // {
    //     printf("Wrong number of args\n");
    // }
    // Grammar * grammar = read_grammar(argv[1]);

    // printGrammar(grammar);
    // map(grammar);

    // Stack * st = NULL;
    // Node * trav = grammar[0].arr->head;
    // printf("%d\n",grammar[0].arr->size);
    // while(trav != NULL)
    // {
    //     st = push(st, getNode(&(trav->ele)));
    //     trav = trav->next;
    // }
    // printStack(st);
    // pop(st);
    // pop(st);
    // pop(st);
    // // pop(st);
    // printf("Go again\n");
    // trav = grammar[0].arr->head;
    // while(trav != NULL)
    // {
    //     st = push(st, getNode(&(trav->ele)));
    //     trav = trav->next;
    // }
    // printStack(st);
    if(argc != 3)
    {
        printf("Wrong number of args\n");
    }
    Grammar * grammar = read_grammar(argv[1]);
    initializeLexer(argv[2]);
    // printGrammar(grammar);
    map(grammar);
    populate_keyhash();
    // seeTokenization();
    int ** firstSet = initializeFirst();
    int ** followSet = initializeFollow();
    // printf("%d\n", enumNonTerminal);
    for(int  i=0; i<enumNonTerminal; i++)
    {
        calculateFirstSet(grammar, i, firstSet);
    }
    
    for(int i=0; i<enumNonTerminal; i++)
    {
        calculateFollowSet(grammar, i, followSet, firstSet);
    }
    // printf("\n\n----------- FIRST SET -----------\n");
    // printFirst(firstSet);
    // printf("\n----------- FOLLOW SET -----------\n");
    // printFollow(followSet);

    int ** parseTable = intializeParseTable();
    createParseTable(grammar,parseTable,firstSet,followSet);
    // printParseTable(grammar,parseTable);

    //ID + ID + ID ; EOF
    tempo_tokens = (char **)malloc(sizeof(char*)*7);

    for(int i=0; i<7; i++)
    {
        tempo_tokens[i] = (char *)malloc(sizeof(char)*15);
        memset(tempo_tokens[i],'\0',sizeof(char)*15);
    }

    strcpy(tempo_tokens[0], "ID");
    strcpy(tempo_tokens[1], "PLUS");
    strcpy(tempo_tokens[2], "ID");
    strcpy(tempo_tokens[3], "PLUS");
    strcpy(tempo_tokens[4], "ID");
    strcpy(tempo_tokens[5], "SEMICOLON");
    strcpy(tempo_tokens[6], "EOF");
    // printGrammar(grammar);
    TreeNode * parseTree = parser(grammar, parseTable);
    preOrder(parseTree);
    printf("\n");
    return 0;
}