#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<ctype.h>

#define BUFFERSIZE 30
#define KEYWORDSIZE 32

char *keyhash[KEYWORDSIZE] ;

//converts to upper case
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
    while(i!=KEYWORDSIZE)
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
	char *tokenName = NULL;
	Token *tkn = (Token*) malloc(sizeof(Token));
	int state = 0;

	while(1)
	{
		switch(state)
		{
			case 0:
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
						state = 14;
						break;
						
					case '/':
						state = 15;
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
						if(buffer[ptr]==' ' || buffer[ptr]=='\t')
							state = 7;
						else if(isalpha(buffer[ptr]))
							state = 10;
						else if(isdigit(buffer[ptr]))
							state = 30;
						else
						{
							tokenName = (char *)malloc(sizeof(char)*4);
							tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
							char * lexeme = (char *)malloc(sizeof(char)*21);
							strcpy(lexeme, "Not a valid alphabet");		
							lexeme[20]='\0';
							tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
							tkn->value = (void *) NULL;
							*toRead = *reading;
							*startptr = ptr;
							return tkn;
						}
						break;
				}
				break;
			case 1:
				if(buffer[ptr]=='=')
					state = 2;
				else
				{
					tokenName = (char *)malloc(sizeof(char)*4);
					tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
					char * lexeme = (char *)malloc(sizeof(char)*19);
					strcpy(lexeme, "Expected ==, got =");		
					lexeme[18]='\0';
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *) NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;
				}
				break;
			case 2:
				lexeme = (char *)malloc(sizeof(char)*3);
				lexeme[0] = '='; lexeme[1] = '='; lexeme[2] = '\0';
				tokenName = (char *)malloc(sizeof(char)*3);
				strcpy(tokenName, "EQ");
				tokenName[2] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
			case 3:
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = '['; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*5);
				strcpy(tokenName, "SQBO");
				tokenName[4] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
			case 4:
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = ']'; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*5);
				strcpy(tokenName, "SQBC");
				tokenName[4] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
			case 5:
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
			case 6:
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = ')'; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*3);
				strcpy(tokenName, "BC");
				tokenName[2] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;				
				break;
			case 7:
				if(buffer[ptr]==' ' || buffer[ptr]=='\t')
					state = 7;
				else
				{
					*toRead = *reading;
					*startptr = ptr;
					return NULL;
				}
				break;
			case 9:
				lineNum++;
				*toRead = *reading;
				*startptr = ptr; 
				return NULL;
			case 10:
				if(isalnum(buffer[ptr]) || buffer[ptr]=='_')
					state = 10;
				else
				{
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
				break;
			case 13:
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = '+'; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*5);
				strcpy(tokenName, "PLUS");
				tokenName[4] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
			case 14:
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = '/'; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*4);
				strcpy(tokenName, "DIV");
				tokenName[3] = '\0';
				tkn->lexeme = lexeme;
				tkn->lineNum = lineNum;
				tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
			case 15:
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = '-'; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*6);
				strcpy(tokenName, "MINUS");
				tokenName[5] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				// printf("%s\t%d\n",tkn->lexeme, tkn->lineNum);
				return tkn;
				break;
			case 16:
				if(buffer[ptr]=='*')
					state = 18;
				else
				{
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
			case 18:
				if(buffer[ptr]=='*')
					state = 19;
				else if(buffer[ptr]=='\n')
				{
					state = 18;
					lineNum++;
				}
				else if(buffer[ptr]=='\0')
				{
					//ERROR HANDLING: Comment not terminated
					// tkn = (Token *)malloc(sizeof(Token));
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
			case 19:
				if(buffer[ptr]=='*')
				{
					*toRead = *reading;
					*startptr = ptr;
					return NULL;
				}
				else if(buffer[ptr]=='\n')
				{
					state = 18;
					lineNum++;
				}
				else if(buffer[ptr]=='\0')
				{
					//ERROR HANDLING: Comment not terminated
					// tkn = (Token *)malloc(sizeof(Token));
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
			case 21:
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = ','; lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*6);
				strcpy(tokenName, "COMMA");
				tokenName[5] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum;	tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
			case 22:
				if(buffer[ptr]=='<')
					state = 23;
				else if(buffer[ptr]=='=')
					state = 25;
				else 
				{
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
			case 23:
				if(buffer[ptr]=='<')
					state = 24;
				else
				{
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
			case 24:
				lexeme = (char *)malloc(sizeof(char)*4);
				lexeme[0] = '<'; lexeme[1] = '<'; lexeme[2] = '<'; lexeme[3] = '\0';
				tokenName = (char *)malloc(sizeof(char)*10);
				strcpy(tokenName, "DRIVERDEF");
				tokenName[9] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;
			case 25:
				lexeme = (char *)malloc(sizeof(char)*3);
				lexeme[0] = '<'; lexeme[1] = '=';lexeme[2] = '\0';
				tokenName = (char *)malloc(sizeof(char)*3);
				strcpy(tokenName, "LE");
				tokenName[2] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;
				break;						
			case 26:	
				if(buffer[ptr]=='>')
					state = 27;
				else if(buffer[ptr]=='=')
					state = 29;
				else 
				{
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
			case 27:
				if(buffer[ptr]=='>')
					state = 28;
				else
				{
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
			case 28:
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
			case 29:
				lexeme = (char *)malloc(sizeof(char)*3);
				lexeme[0] = '>'; lexeme[1] = '='; lexeme[2] = '\0';
				
				tokenName = (char *)malloc(sizeof(char)*3);
				strcpy(tokenName, "GE");
				tokenName[2] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
				*toRead = *reading;
				*startptr = ptr;
				return tkn;	
				break;			
			case 30:
				if(isdigit(buffer[ptr]))
					state = 30;
				else if(buffer[ptr]=='.')
					state = 31;
				else
				{
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
			case 31:
				if(isdigit(buffer[ptr]))
					state = 32;
				else if(buffer[ptr]=='.')
				{
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
					tokenName = (char *)malloc(sizeof(char)*4);
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
				break;		
			case 32:
				if(isdigit(buffer[ptr]))
					state = 32;
				else if(buffer[ptr]=='E')
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
			case 33:
				if(buffer[ptr]=='+' || buffer[ptr]=='-')
					state = 34;
				else if(isdigit(buffer[ptr]))
					state = 35;
				else
				{
					tokenName = (char *)malloc(sizeof(char)*4);
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
				break;
			case 34:
				if(isdigit(buffer[ptr]))
					state = 35;
				else
				{
					tokenName = (char *)malloc(sizeof(char)*4);
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
				break;
			case 35:
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
			case 38:
				
				if(buffer[ptr]=='=')
				{
					state = 39;
				}
				else
				{
					
					tokenName = (char *)malloc(sizeof(char)*4);
					tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
					char * lexeme = (char *)malloc(sizeof(char)*19);
					strcpy(lexeme, "Expected !=, got !");		
					lexeme[18]='\0';
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *) NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;					
				}
				break;
			case 39:
				lexeme = (char *) malloc(sizeof(char)*3);
				lexeme[0] = '!'; lexeme[1] = '='; lexeme[2] = '\0';
				tokenName = (char *)malloc(sizeof(char)*3);
				strcpy(tokenName, "NE");
				tokenName[2] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum;	tkn->token = tokenName;
				tkn->value = (void *) NULL;
				*toRead = *reading;
				*startptr = ptr;
				return tkn;				
				break;
			case 40:
				if(buffer[ptr]=='=')
					state = 41;
				else
				{
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
			case 41:
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
			case 43:
				if(buffer[ptr]=='.')
					state = 44;
				else
				{
					tokenName = (char *)malloc(sizeof(char)*4);
					tokenName[0] = 'E'; tokenName[1] = 'R'; tokenName[2] = 'R'; tokenName[3] = '\0';
					char * lexeme = (char *)malloc(sizeof(char)*19);
					strcpy(lexeme, "Expected .., got .");		
					lexeme[18]='\0';
					tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
					tkn->value = (void *) NULL;
					*toRead = *reading;
					*startptr = ptr;
					return tkn;												
				}
				break;
			case 44:
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
			case 45:
				lexeme = (char *)malloc(sizeof(char)*2);
				lexeme[0] = ';';	lexeme[1] = '\0';
				tokenName = (char *)malloc(sizeof(char)*8);
				strcpy(tokenName, "SEMICOL");
				tokenName[7] = '\0';
				tkn->lexeme = lexeme; tkn->lineNum = lineNum;	tkn->token = tokenName;
				tkn->value = (void *)NULL;
				// incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
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
		incrementPointer(fp, &buffer, buffer1, buffer2, reading, &ptr);
	}
	return NULL;
}

void printToken(Token * tkn)
{
	printf("%10s\t\t",tkn->lexeme);
	printf("%10s\t\t",tkn->token);
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
	printf("LEXEME\t\t\t  TOKEN\t\t   LineNum\t\t   Value\n");
	printf("---------------------------------------------------------------------------\n");
	Token ** stream = tknstr->Tokenized_code;
	for(int i=0; i < tknstr->num_tokens; i++)
	{
		Token * tkn = stream[i];
		printToken(tkn);
	}
	return 0;
}
