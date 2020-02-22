#ifndef LEXER_H_
#define LEXER_H_
#include "lexerDef.h"

void insertError(char * description, char* lexeme, int linenum, int tag);
void printErrorList(int whichOne);
void uppertoken(char *str);
void populate_keyhash();
int keyhash_find(char * str);
void readBuffer(FILE *fp, char *buffer);
void initializeLexer(char * filename);
Token * getNextToken();
int lexeme2int(char *s);
float lexeme2real(char *s);
void incrementPointer(FILE * fp, char ** buffer, char * buffer1, char * buffer2, int * reading, int * ptr);
void decrementPointer(FILE * fp, char ** buffer, char * buffer1, char * buffer2, int * reading, int * ptr);
char * fillLexeme(char * buffer1, char * buffer2, int reading, int toRead, int startptr, int ptr);
void getErrorToken(Token * tkn);
Token * nextToken(FILE * fp, char *buffer, char * buffer1, char * buffer2, int *startptr, int * reading, int * toRead);
void printToken(Token * tkn);

#endif