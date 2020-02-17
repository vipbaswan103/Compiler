#include<stdio.h>
#include<stdlib.h>
#include<string.h>

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


int lineNum = 0;
// char *startPtr = NULL, *currentPtr = NULL;
// char *buffer;		//Twin Buffer


TokenStream * lexer(char *filename, int* lexical_err)	//read prog into buffer
{
	char *buffer;
	int startptr=0;
	FILE * fp = fopen(filename,"r");

	if(fp == NULL)
	{
		printf("Error opening file");
		return NULL;
	}

	fseek(fp,0L,SEEK_END);
	int len = ftell(fp);
	fseek(fp,0L,SEEK_SET);
	buffer = (char*)malloc(sizeof(char)*len);

	for(int i = 0 ; i<len ; i++)
	{
		fscanf(fp,"%c",&buffer[i]);
	}
	buffer[len-1] = '\0';		
	
	int size_tknStream = 10;
	Token *tkn;
	TokenStream *tknStream = (TokenStream*)malloc(sizeof(TokenStream)); 
	tknStream->Tokenized_code = (Token**)malloc(sizeof(Token*)*size_tknStream);
	tknStream->num_tokens = 0;                                                         
	while(buffer[startptr]!='\0')
	{
		if(nextToken(buffer,&startptr)==NULL)
			continue;
		else 
		{ 	

			if(tknStream->num_tokens == size_tknStream)
			{
				size_tknStream *= 2;
				tknStream->Tokenized_code = (Token**)realloc(tknStream->Tokenized_code,sizeof(Token*)*size_tknStream);  
			}
			tkn = nextToken(buffer,&startptr);
			if(strcmp(tkn->token,"ERR"))
			{
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
		i++;
		val = 10*val + (s[i]-'0');
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

Token * nextToken(char *buffer, int *startptr)
{
	int ptr = startptr;
	char *lexeme = NULL;
	Token *tkn = (Token*) malloc(sizeof(Token));
	
	if(isalpha(buffer[ptr]))
	{
		ptr++;
		while(buffer[ptr] != '\0' && (isalpnum(buffer[ptr]) || buffer[ptr] == '_'))
		{
			ptr++;
		}

		int lex_len = ptr-*startptr+1;
		lexeme = (char*) malloc(sizeof(char) * (lex_len));
		
		for(int i = *startptr ; i<ptr ; i++)
		{
			lexeme[i-*startptr] = buffer[i];
		}
		
		lexeme[lex_len-1] = '\0';
		*startptr = ptr;

		// TODO : Check from hashtable if keyword matching

		// If not matching
		tkn->token = (char*) malloc(sizeof(char) * 3);
		strcpy(tkn->token , "ID");
		tkn->lexeme = lexeme; tkn->value = NULL; tkn->lineNum = lineNum;
		
		// TODO : If keyword 
		tkn->token = (char*) malloc(sizeof(char) * ());
		strcpy(tkn->token , );
		tkn->lexeme = lexeme; tkn->value = NULL; tkn->lineNum = lineNum;
		
		return tkn;
	}

	/* 	SPACES	*/

	if(buffer[ptr] == ' ' || buffer[ptr] == '\t')
	{
		ptr++;

		//Ignoring as many spaces in a loop
		while(buffer[ptr] == ' ' || buffer[ptr] == '\t')
		{
			ptr++;
		}
		
		//We will start from the next non space/tab character at the beginning of the DFA
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
			ptr++;
		}
		
		//We will start at the beginning of the DFA, from a non new line character
		*startptr = ptr; 
		return NULL;
	}

	/* 	PLUS */
	
	else if(buffer[ptr] == '+')
	{
		//Simple single layer condition, no branches, no ambiguity

		tkn = (Token *)malloc(sizeof(Token));
		lexeme = (char *)malloc(sizeof(char)*2);
		lexeme[0] = '+'; lexeme[1] = '\0';
		char * tokenName = (char *)malloc(sizeof(char)*5);
		strcpy(tokenName, "PLUS");
		tokenName[4] = '\0';
		tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
		tkn->value = (void *)NULL;
		ptr++;
		*startptr = ptr;
		return tkn;
	}

	/* 	MINUS */

	else if(buffer[ptr] == '-')
	{
		//Simple single layer condition, no branches, no ambiguity
		
		tkn = (Token *)malloc(sizeof(Token));
		lexeme = (char *)malloc(sizeof(char)*2);
		lexeme[0] = '-'; lexeme[1] = '\0';
		char * tokenName = (char *)malloc(sizeof(char)*6);
		strcpy(tokenName, "MINUS");
		tokenName[5] = '\0';
		tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
		tkn->value = (void *)NULL;
		ptr++;
		*startptr = ptr;
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
		ptr++;
		*startptr = ptr;
		return tkn;
	}

	/* MUL and COMMMENTS */

	else if(buffer[ptr] == '*')
	{
		
		//The first character is *. 
		//I need to look ahead now.
		ptr++;


		if(buffer[ptr] != '*')
		{
			//If the nest is not a *. 
			//Treat the previous * as a multiply

			tkn = (Token *)malloc(sizeof(Token));
			lexeme = (char *)malloc(sizeof(char)*2);
			lexeme[0] = '*';	lexeme[1] = '\0';
			char * tokenName = (char *)malloc(sizeof(char)*4);
			strcpy(tokenName, "MUL");
			tokenName[3] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
			*startptr = ptr;
			return tkn;
		}

		
		else
		{
			//Since the next is also a *, start the comments. 
			ptr++;

			// Untill my program ends, look for end of comments  
			while(buffer[ptr] != '\0')
			{
				// I am looking for **. Ignore the rest.
				if(buffer[ptr] != '*' &&  buffer[ptr] != '\n')
				{
					ptr++;
					continue;
				}

				// If the line changes, move on but increment line count.
				else if(buffer[ptr] == '\n')
				{
					lineNum++;
					ptr++;
					continue;		
				}

				// Encountered a *. Look for one more.
				else if(buffer[ptr] == '*')
				{
					ptr++;
					if(buffer[ptr] == '*')
					{
						// Finally return NULL.
						ptr++;
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
				*startptr = ptr;
				return tkn;
			}	
			
			// Finally set the start to current pointer because here the comment terminated 
			*starptr = ptr;
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
		ptr++;
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
		ptr++;
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
		ptr++;
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
		ptr++;
		*startptr = ptr;
		return tkn;
	}
	
	/* NOT EQUAL */
	
	else if(buffer[ptr] == '!')
	{
		ptr++;
		if(buffer[ptr]=='=')
		{
			tkn = (Token *)malloc(sizeof(char))*3;
			lexeme[0] = '!'; lexeme[1] = '='; lexeme[2] = '\0';
			char * tokenName = (char *)sizeof(Token);
			lexeme = (char *) malloc(sizeof(malloc(sizeof(char)*3);
			strcpy(tokenName, "NE");
			tokenName[2] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum;	tkn->token = tokenName;
			tkn->value = (void *) NULL;
			
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
		
		ptr++;
		*startptr = ptr;
		return tkn;
		
	}
	
	/* Branch with = */
	
	else if(buffer[ptr] == '=')
	{
		tkn = (Token *)malloc(sizeof(Token));
		ptr++;
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
			ptr++;
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

		// my pointer points at the next xharacter that was not =. 
		
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
		ptr++;
		*startptr = ptr;
		return tkn;
	}


	else if(buffer[ptr] == '<')
	{
		ptr++;
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
			ptr++;
			*startptr = ptr;
			return tkn;
 		}
		else if(buffer[ptr] == '<')
		{
			// Definition << Symbol

			tkn = (Token *)malloc(sizeof(Token));
			lexeme = (char *)malloc(sizeof(char)*3);
			lexeme[0] = '<'; lexeme[1] = '<'; lexeme[2] = '\0';
			
			char * tokenName = (char *)malloc(sizeof(char)*4);
			strcpy(tokenName, "DEF");
			tokenName[3] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
			ptr++;
			*startptr = ptr;
			return tkn;
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
		ptr++;
		*startptr = ptr;
		return tkn;
	}
	else if(buffer[ptr] == '>')
	{
		ptr++;
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
			ptr++;
			*startptr = ptr;
			return tkn;
 		}
		else if(buffer[ptr] == '>')
		{
			//Definition ending >>

			tkn = (Token *)malloc(sizeof(Token));
			lexeme = (char *)malloc(sizeof(char)*3);
			lexeme[0] = '>'; lexeme[1] = '>'; lexeme[2] = '\0';
			
			char * tokenName = (char *)malloc(sizeof(char)*7);
			strcpy(tokenName, "ENDDEF");
			tokenName[6] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;
			ptr++;
			*startptr = ptr;
			return tkn;
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
			*startptr = ptr;
			return tkn;
		}
	}

	//Number parsing

	else if(isnum(buffer[ptr]))
	{
		ptr++;
		while(isnum(buffer[ptr]))
		{
			ptr++;
		}
		tkn = (Token *)malloc(sizeof(Token));
		if(buffer[ptr] == '.')
		{
			ptr++;
			if(buffer[ptr] == '.')
			{
				//RangeOp .. operator is encountered
				ptr--;
				int length = ptr - (*startptr) + 1;
				lexeme = (char *)malloc(sizeof(char)*length);
				int trav = *startptr, i=0;
				while(i < length-1)
				{
					lexeme[i] = buffer[trav];
					i++;
					trav++;
				}
				lexeme[length - 1] = '\0';
				char * tokenName = (char *)malloc(sizeof(char)*4);
				strcpy(tokenName, "NUM");
				tokenName[3] = '\0';

				int * result = (int *)malloc(sizeof(int));
				*result = lexeme2int(lexeme);
				tkn->lexeme = lexeme;
				tkn->token = tokenName;
				tkn->value = (void *)result;
				tkn->lineNum = lineNum;
				*startptr = ptr;
				return tkn;
			}
			else if(isnum(buffer[ptr]))
			{
				//Post decimal number has started
				ptr++;
				while(isnum(buffer[ptr]))
				{
					ptr++;
				}
				if(buffer[ptr] == 'E')
				{
					//The power of 10 is being parsed
					ptr++;
					if(isnum(buffer[ptr]))
					{
						//There is a number after the E symbol
						ptr++;
						while(isnum(buffer[ptr]))
						{
							ptr++;
						}
					}
					else if(buffer[ptr] == '+' || buffer[ptr] == '-')
					{
						//There is a sign after the E symbol
						ptr++;
						if(isnum(buffer[ptr]))
						{
							ptr++;
							while(isnum(buffer[ptr]))
							{
								ptr++;
							}
						}
						else
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
						return tkn;
					}
				}
				
				ptr--;
				int length = ptr - (*startptr) + 2;
				lexeme = (char *)malloc(sizeof(char)*length);

				int i = 0, trav = *startptr;
				//Put the number as it is in the lexeme
				while(i < length-1)
				{
					lexeme[i] = buffer[trav];
					trav++; i++;
				}
				lexeme[length-1] = '\0';

				tkn = (Token *)malloc(sizeof(Token));

				char * tokenName = (char *)malloc(sizeof(char)*5);
				strcpy(tokenName, "RNUM");
				tokenName[4] = '\0';
				
				//lexeme2real is returning a float value. So take it and caste to a void*

				tkn->lexeme = lexeme; tkn->token = tokenName;
				float * var = (float *)malloc(sizeof(float));
				*var = lexeme2real(lexeme);	tkn->value = (void *) var; tkn->lineNum = lineNum;

				return tkn;
			}
			else
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
				return tkn;
			}
		}
		else if(!isaplha(buffer[ptr]) &&  (buffer[ptr] != '_'))
		{
			//after the integers there is no dot but a word boundary
			//This "word boundary" may be a space, \n or \t, or 
			//anything but an underscore or alphanet
			
			int length=0;
			if(buffer[ptr] == '\0')
				length = ptr - (*startptr) + 1;
			else
			{
				ptr--;
				length = ptr - (*startptr) + 2;
			}
			lexeme = (char *)malloc(sizeof(char)*length);
			int trav = *startptr, i=0;

			while(i < length)
			{
				lexeme[i] = buffer[trav];
				i++; trav++;
			}
			lexeme[length-1] = '\0';
			
			char * tokenName = (char *)malloc(sizeof(char)*4);
			strcpy(tokenName, "NUM");
			tokenName[3] = '\0';

			int * result = (int *)malloc(sizeof(int));
			*result = lexeme2int(lexeme);

			tkn->lexeme = lexeme; tkn->token = tokenName; 
			tkn->value = (void *)result; tkn->lineNum = lineNum;
			*startptr = ptr;
			return tkn;
		}
		else if 
		{
			//ERROR code. This is the 123aryan or 123_. Ask mam how to give error
		}
	}
	
	else if(buffer[ptr] == '.')
	{
		tkn = (Token *)malloc(sizeof(Token));
		ptr++;
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
			ptr++;
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
		*startptr = ptr;
		return tkn;
	}
	else if(buffer[ptr] == ':')
	{
		tkn = (Token *)malloc(sizeof(Token));
		ptr++;
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
			ptr++;
		}
		else
		{
			// Colon :, simple branch
			lexeme = (char *)malloc(sizeof(char)*2);
			lexeme[0] = ':'; lexeme[1] = '\0';
			char *tokenName = (char *)malloc(sizeof(char)*6);
			strcpy(tokenName, "COLON");
			tokenName[5] = '\0';
			tkn->lexeme = lexeme; tkn->lineNum = lineNum; tkn->token = tokenName;
			tkn->value = (void *)NULL;		
		}
		
		//pointer at the next symbol after the definitely parsed branches, no error.
		*startptr = ptr;
		return tkn;
	}
}