#ifndef CODEGENDEF_H_
#define CODEGENDEF_H_

typedef struct temporary
{
    int num;
    char* type;
    int offset;
    int width;
}temporary;

typedef struct quad
{
    char op[51];
    char arg1[51];
    char arg2[51];
    char result[51];
}quad;

typedef struct IRcode
{
    quad *ele;
    struct IRcode *next;
}IRcode;

typedef struct intermed
{
    IRcode code;
    struct temporary t;
}intermed;

#endif