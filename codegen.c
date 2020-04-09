#include "codegenDef.h"
#include "ast.h"

void getOp(char * name, char * op)
{
    if(!strcmp(name,"PLUS"))
        strcpy(op, "+\0");
    else if(!strcmp(name,"MINUS"))
        strcpy(op, "-\0");
    else if(!strcmp(name,"MUL"))
        strcpy(op, "*\0");
    else if(!strcmp(name,"DIV"))
        strcpy(op, "/\0");
    else if(!strcmp(name,"LT"))
        strcpy(op, "<\0");
    else if(!strcmp(name,"LE"))
        strcpy(op, "<=\0");
    else if(!strcmp(name,"GT"))
        strcpy(op, ">\0");
    else if(!strcmp(name,"GE"))
        strcpy(op, ">=\0");
    else if(!strcmp(name,"EQ"))
        strcpy(op, "=\0");
    else if(!strcmp(name,"NE"))
        strcpy(op, "!=\0");
    else if(!strcmp(name,"AND"))
        strcpy(op, "AND\0");
    else if(!strcmp(name,"OR"))
        strcpy(op, "OR\0");
}

void getTemporary(temporary * tmp)
{
    sprintf(tmp->name, "t%d", tmpNum);
    tmpNum++;
}

//Put code2 at the end of code1
IRcode * mergeCode(IRcode * code1, IRcode * code2)
{
    if(code1 == NULL)
    {
        return code2;
    }

    IRcode * trav = code1;
    while(trav->next != NULL)
        trav = trav->next;
    trav->next = code2;
    return code2;
}

intermed * generateIRCode(astNode * currentNode, quad * labels)
{
    if(currentNode == NULL)
        return NULL;
    
    if(!strcmp(currentNode->node->ele.internalNode->label, "PLUS") || 
    !strcmp(currentNode->node->ele.internalNode->label, "MINUS") ||
    !strcmp(currentNode->node->ele.internalNode->label, "MUL") ||
    !strcmp(currentNode->node->ele.internalNode->label, "DIV") ||
    !strcmp(currentNode->node->ele.internalNode->label, "LT") ||
    !strcmp(currentNode->node->ele.internalNode->label, "GT") ||
    !strcmp(currentNode->node->ele.internalNode->label, "LE") ||
    !strcmp(currentNode->node->ele.internalNode->label, "GE") ||
    !strcmp(currentNode->node->ele.internalNode->label, "EQ") ||
    !strcmp(currentNode->node->ele.internalNode->label, "NE") ||
    !strcmp(currentNode->node->ele.internalNode->label, "AND") ||
    !strcmp(currentNode->node->ele.internalNode->label, "OR") )
    {
        intermed* leftchild = generateIRCode(currentNode->child, NULL);
        intermed* rightchild = generateIRCode(currentNode->child->sibling, NULL);

        intermed* final = (intermed*) malloc(sizeof(intermed));
        getTemporary(&(final->t));
        final->code = (IRcode*)malloc(sizeof(IRcode));
        final->code->ele = (quad *)malloc(sizeof(quad));
        getOp(currentNode->node->ele.internalNode->label, final->code->ele->op);
        strcpy(final->code->ele.arg1, leftchild->t.name);
        strcpy(final->code->ele.arg2, rightchild->t.name);
        strcpy(final->code->ele.result, final->t.name);
        final->code->next = NULL;
        mergecode(leftchild->code, rightchild->code);
        mergeCode(leftchild->code, final->code);

        final->code = leftchild->code;
        free(leftchild);
        free(rightchild);
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "ASSIGNOP"))
    {
        
    }
}