#ifndef CODEGENDEF_H_
#define CODEGENDEF_H_

#define STR_SIZE 51
extern int tmpNum;
extern int labelNum;

typedef enum {ID = 2, CONSTANT = 1, NONE = 0} argtype;

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
    argtype tag1;
    argtype tag2;
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