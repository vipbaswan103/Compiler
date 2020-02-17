#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<ctype.h>

#define BUFFERSIZE 30

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

Token * nextToken(FILE * fp, char *buffer, char * buffer1, char * buffer2, int *startptr, int * reading, int * toRead);



void readBuffer(FILE *fp, char * buffer)
{
	int read = fread(buffer,1,BUFFERSIZE,fp);
	if(ferror(fp))
	{
		printf("Error in reading the file\n");
		exit(-1);
	}
	if(read<BUFFERSIZE)
		buffer[read]='\0';
}


TokenStream * lexer(char * filename, int * lexical_err)	//read prog into buffer
{
	char * buffer1 = (char *)malloc(sizeof(char) * BUFFERSIZE);
	char * buffer2 = (char *)malloc(sizeof(char) * BUFFERSIZE);
	int startptr=0;
	FILE * fp = fopen(filename,"r");

	if(fp == NULL)
	{
		printf("Error opening file");
		return NULL;
	}
	readBuffer(fp, buffer1);

	int size_tknStream = 10;
	Token *tkn;
	TokenStream *tknStream = (TokenStream*)malloc(sizeof(TokenStream)); 
	tknStream->Tokenized_code = (Token**)malloc(sizeof(Token*)*size_tknStream);
	tknStream->num_tokens = 0;   

	char * buffer;
	int reading = 1, toRead = 1;

	if(toRead == 1)
		buffer = buffer1;
	else
		buffer = buffer2;
	
	while(buffer[startptr]!='\0')
	{
		tkn = nextToken(fp, buffer, buffer1, buffer2, &startptr, &reading, &toRead);

		//Update the buffer to one which we are supposed to read next
		if(toRead == 1)
			buffer = buffer1;
		else
			buffer = buffer2;

		if(tkn == NULL)
			continue;
		else 
		{
			if(tknStream->num_tokens == size_tknStream)
			{
				size_tknStream *= 2;
				tknStream->Tokenized_code = (Token**)realloc(tknStream->Tokenized_code,sizeof(Token*)*size_tknStream);  
			}
			if(strcmp(tkn->token,"ERR"))
			{
				// printf("No error\n");
				tknStream->Tokenized_code[tknStream->num_tokens] = tkn;
				tknStream->num_tokens++;
			}
			else
			{
				*lexical_err = 1;
				printf("%s\n",tkn->lexeme); 
			}
		}
	}
	return tknStream;
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
		else if(s[i]=='E')
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
		len = BUFFERSIZE-startptr+1;
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
		// printf("Here: %c\n",buffer[ptr]);
		tkn = (Token *)malloc(sizeof(Token));
		if(buffer[ptr] == '.') //<digits>.
		{
			incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
			if(buffer[ptr] == '.')	//<digits>..
			{
				//RangeOp .. operator is encountered
				decrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);

				lexeme = fillLexeme(buffer1, buffer2, *reading, *toRead, *startptr, ptr);
				// int length = ptr - (*startptr) + 1;
				// lexeme = (char *)malloc(sizeof(char)*length);
				// int trav = *startptr, i=0;
				// while(i < length-1)
				// {
				// 	lexeme[i] = buffer[trav];
				// 	i++;
				// 	trav++;
				// }
				// lexeme[length - 1] = '\0';
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
				if(buffer[ptr] == 'E')		//<digits>.<digits>E
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
				// int length = ptr - (*startptr) + 1;
				// lexeme = (char *)malloc(sizeof(char)*length);

				// int i = 0, trav = *startptr;
				// //Put the number as it is in the lexeme
				// while(i < length-1)
				// {
				// 	lexeme[i] = buffer[trav];
				// 	trav++; i++;
				// }
				// lexeme[length-1] = '\0';

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
			// int length=0;
			// if(buffer[ptr] == '\0')
			// 	length = ptr - (*startptr) + 1;
			// else
			// 	length = ptr - (*startptr) + 1;
			// // printf("%d\n",ptr);
			// lexeme = (char *)malloc(sizeof(char)*length);
			// int trav = *startptr, i=0;

			// while(i < length)
			// {
			// 	lexeme[i] = buffer[trav];
			// 	i++; trav++;
			// }
			// lexeme[length-1] = '\0';
			
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

int main(int argc, char * argv[])
{
	populate_keyhash();
	if(argc != 2)
	{
		printf("Wrong number of arguments\n");
		exit(-1);
	}

	int error = 0;
	TokenStream * tknstr = lexer(argv[1], &error);

	//printf("%d\n",tknstr->num_tokens);
	printf("LEXEME\t\tTOKEN\t\tLineNum\t\tValue\n");
	printf("--------------------------------------------------------\n");
	Token ** stream = tknstr->Tokenized_code;
	for(int i=0; i < tknstr->num_tokens; i++)
	{
		Token * tkn = stream[i];
		printToken(tkn);
	}
	return 0;
}
