#ifndef CODEGENDEF_H_
#define CODEGENDEF_H_

#define STR_SIZE 51
int tmpNum = 0;

typedef struct temporary
{
    char name[STR_SIZE];
    char type[STR_SIZE];
    int offset;
    int width;
}temporary;

typedef struct quad
{
    char op[STR_SIZE];
    char arg1[STR_SIZE];
    char arg2[STR_SIZE];
    char result[STR_SIZE];
}quad;

typedef struct IRcode
{
    quad *ele;
    struct IRcode *next;
}IRcode;

typedef struct intermed
{
    IRcode * code;
    struct temporary t;
}intermed;

#endif