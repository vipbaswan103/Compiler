#ifndef LEXER_H_
#define LEXER_H_
#include "lexerDef.h"

int hash_func(char *str);
Element* hash_find(char * str, Hashtable * hash_tb);
void hash_insert(Element * ele, Hashtable * hash_tb);
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
void removeComments();
void removeCommentsUtil(FILE * fp, char *buffer, char * buffer1, char * buffer2, int *startptr, int * reading, int * toRead);
char * fillLexeme(char * buffer1, char * buffer2, int reading, int toRead, int startptr, int ptr);
void getErrorToken(Token * tkn);
Token * nextToken(FILE * fp, char *buffer, char * buffer1, char * buffer2, int *startptr, int * reading, int * toRead);
void printToken(Token * tkn);
void initializeKeyHash();
#endif