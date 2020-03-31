/*  AST Node to be used for type checking/semantic rule verification
    
    ASSIGNOP
        - if left side is some var from output_list, update isassigned
        - especifically handle if both operands are arrays
    GET_VAL 
        - if the var is some var from output_list, update isassigned
    ASSIGNOPARR 
        - type, lower, uppper
    MODULEASSIGNOP and MODULECALL
        - left side matches with the output_list (type, number)
        - is the function declared/defined above the use
    FOR 
        - flag with the index variable
    WHILE
        - expression must be boolean type
    PLUS, MINUS, MUL, DIV, AND, OR, LT, LE, GT, GE, EQ, NE
        - Type checking (Both operands must be same)
        - No operand can be array
    
    SWITCH 
        - the switch var can't be real
        - check the default clause (shouldn't exist for boolean switch var, must exist for integer switch var)

    Stack Guys Peeps..!!!
        PROGRAM
        MODULE
        MODULEDEF
        MODULEDEC
        DRIVER
        WHILE
        SWITCH
        FOR
*/    

#ifndef SEMANTICSDEF_H_
#define SEMANTICSDEF_H_

#include "symbolTableDef.h"

typedef enum {ArrayType = 1, IdentifierType = 0} typeTag; 

typedef struct arrayType
{
    char *basicType;
    identifier *lowerBound;
    identifier *upperBound;
}arrayType;

typedef union whichType
{
    arrayType arr;
    char *type;
}whichType;

typedef struct type
{
    whichType tp;
    typeTag tag;
}type;

#endif