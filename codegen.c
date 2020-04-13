#include "codegenDef.h"
#include "ast.h"
#include "symbolTableDef.h"
#include "symbolTable.h"
#include "semantics.h"
int tmpNum = 0;
int labelNum = 0;

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
        strcpy(op, "==\0");
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

void initQuad(quad *ele ,char* arg1, char* arg2, char* result)
{
    strcpy(ele->arg1,arg1);
    strcpy(ele->arg2,arg2);
    strcpy(ele->result,result);
}

void getLabel(char* l)
{
    sprintf(l, "L%d", labelNum);
    labelNum++;
}

void initializeFinalCode(intermed ** final)
{
    *final = (intermed *)malloc(sizeof(intermed));
    (*final)->code = (IRcode *)malloc(sizeof(IRcode));
    (*final)->code->ele = (quad *)malloc(sizeof(quad));
    (*final)->code->next = NULL;
}

//Put code2 at the end of code1
void mergeCode(IRcode ** code1, IRcode * code2)
{
    if(*code1 == NULL)
    {
        *code1 = code2;
        return;
    }

    IRcode * trav = *code1;
    while(trav->next != NULL)
        trav = trav->next;
    trav->next = code2;
}

void printCode(IRcode * code)
{
    IRcode * trav = code;

    while(trav != NULL)
    {
        if(strcmp(trav->ele->result, "\0"))
        {
            printf("%s ", trav->ele->result);
        }
        else
            printf("--- ");
        printf(" = ");
        if(strcmp(trav->ele->arg1, "\0"))
        {
            printf("%s ", trav->ele->arg1);
        }
        else
            printf("--- ");
        if(strcmp(trav->ele->op, "\0"))
        {
            printf("%s ", trav->ele->op);
        }
        else
            printf("--- ");
        if(strcmp(trav->ele->arg2, "\0"))
        {
            printf("%s ", trav->ele->arg2);
        }
        else
            printf("--- ");
        printf("\n");
        trav = trav->next;
    }
}

// the finction takes in the ASTNode to work on
// along with the labels that give the next/true/false labels 
// tbStack is the current stack element
intermed * generateIRCode(astNode * currentNode, quad * labels, tableStack * tbStack)
{
    
    //not reachable
    if(currentNode == NULL)
    {
        intermed * final = (intermed *)malloc(sizeof(intermed));
        final->code = NULL;
        return final;
    }
    

    //arithmetic operators
    if(!strcmp(currentNode->node->ele.internalNode->label, "PLUS") || 
    !strcmp(currentNode->node->ele.internalNode->label, "MINUS") ||
    !strcmp(currentNode->node->ele.internalNode->label, "MUL") ||
    !strcmp(currentNode->node->ele.internalNode->label, "DIV"))
    {
        intermed* leftchild = generateIRCode(currentNode->child, labels, tbStack);
        intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);
        intermed* final;
        initializeFinalCode(&final);
        getTemporary(&(final->t));
        
        initQuad(final->code->ele, leftchild->t.name, rightchild->t.name, final->t.name);
        getOp(currentNode->node->ele.internalNode->label, final->code->ele->op);
        mergeCode(&(leftchild->code), rightchild->code);
        mergeCode(&(leftchild->code), final->code);
        final->code = leftchild->code;

        free(leftchild);
        free(rightchild);

        return final;
    }

    //boolean operators 
    else if (!strcmp(currentNode->node->ele.internalNode->label, "LT") ||
    !strcmp(currentNode->node->ele.internalNode->label, "GT") ||
    !strcmp(currentNode->node->ele.internalNode->label, "LE") ||
    !strcmp(currentNode->node->ele.internalNode->label, "GE") ||
    !strcmp(currentNode->node->ele.internalNode->label, "EQ") ||
    !strcmp(currentNode->node->ele.internalNode->label, "NE"))
    {
        if(labels != NULL)
        {
            // the labels have something, this means that the boolean expression is 
            //bring used for a "if else" like clause, with labels containing the 
            //place for false and true jumps
            intermed* leftchild = generateIRCode(currentNode->child, labels, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);
            
            //ifcode will have the conditional code's quads
            IRcode* ifcode = (IRcode *)malloc(sizeof(IRcode));
            ifcode->ele = (quad *)malloc(sizeof(quad));
            //jump instruction for the true case
            IRcode* goto1 = (IRcode *)malloc(sizeof(IRcode));
            goto1->ele = (quad *)malloc(sizeof(quad));
            //jump instruction for the false case
            IRcode* goto2 = (IRcode *)malloc(sizeof(IRcode));
            goto2->ele = (quad *)malloc(sizeof(quad));
            
            //quad had arg1 and arg2 and result captures the fact that it is an "if" condition
            initQuad(ifcode->ele, leftchild->t.name, rightchild->t.name, "if\0");
            getOp(currentNode->node->ele.internalNode->label, ifcode->ele->op);
            
            //goto label number 1, for jumping when condition is true
            //hence utilising arg1 of the "lables" quad
            initQuad(goto1->ele, labels->arg1, "\0", "\0");
            strcpy(goto1->ele->op, "goto\0");
            
            //goto label number 2, for jumping when condition is false
            //hence utilising arg2 of the "labels" quad
            initQuad(goto2->ele, labels->arg2, "\0", "\0");
            strcpy(goto2->ele->op, "goto\0");
            
            //merge all the codes in order
            mergeCode(&(leftchild->code), rightchild->code);
            mergeCode(&(leftchild->code), ifcode);
            mergeCode(&(leftchild->code), goto1);
            mergeCode(&(leftchild->code), goto2);
            
            intermed* final = (intermed *)malloc(sizeof(intermed));
            final->code = leftchild->code;
            free(leftchild);
            free(rightchild);
            return final;
        }
        else
        {
            // the label has not's
            intermed* leftchild = generateIRCode(currentNode->child, labels, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);

            intermed* final;
            initializeFinalCode(&final);
            getTemporary(&(final->t));
            
            initQuad(final->code->ele, leftchild->t.name, rightchild->t.name, final->t.name);
            getOp(currentNode->node->ele.internalNode->label, final->code->ele->op);
            final->code->next = NULL;
            mergeCode(&(leftchild->code), rightchild->code);
            mergeCode(&(leftchild->code), final->code);
            // free(final->code->ele);
            // free(final->code);
            final->code = leftchild->code;
            free(leftchild);
            free(rightchild);

            return final;
        }
    }

    // (a AND b) AND (c AND d) 
    // a L1: b L2: c L3: d
    // t1 = t0  AND t2
    // t1 = t0 L1: t2
    
    else if(!strcmp(currentNode->node->ele.internalNode->label, "AND"))
    {
        if(labels != NULL)
        {
            //inside while
            quad *l = (quad *)malloc(sizeof(quad));
            getLabel(l->arg1);
            strcpy(l->arg2,labels->arg2);
            intermed* leftchild = generateIRCode(currentNode->child, l, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);
            
            intermed* final;
            initializeFinalCode(&final);
            getTemporary(&(final->t));

            IRcode *labCode = (IRcode *)malloc(sizeof(IRcode));
            labCode->ele = (quad *)malloc(sizeof(quad));
            
            strcpy(labCode->ele->arg1, l->arg1);
            strcpy(labCode->ele->op, ":\0");
            
            labCode->next = NULL;
            
            mergeCode(&(leftchild->code), labCode);
            mergeCode(&(leftchild->code), rightchild->code);
            final->code = leftchild->code;
            free(leftchild);
            free(rightchild);

            return final;
        }
        else
        {
            //RHS of assignment statement
            intermed* leftchild = generateIRCode(currentNode->child, labels, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);

            intermed* final;
            initializeFinalCode(&final);
            getTemporary(&(final->t));
            
            getOp(currentNode->node->ele.internalNode->label, final->code->ele->op);
            strcpy(final->code->ele->arg1, leftchild->t.name);
            strcpy(final->code->ele->arg2, rightchild->t.name);
            strcpy(final->code->ele->result, final->t.name);
            final->code->next = NULL;
            mergeCode(&(leftchild->code), rightchild->code);
            mergeCode(&(leftchild->code), final->code);
            final->code = leftchild->code;
            free(leftchild);
            free(rightchild);

            return final;
        }
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "OR"))
    {
        //Correct this code 
        if(labels != NULL)
        {
            quad *l = (quad *)malloc(sizeof(quad));
            getLabel(l->arg2);
            strcpy(l->arg1,labels->arg1);
            intermed* leftchild = generateIRCode(currentNode->child, l, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);
            
            intermed* final;
            initializeFinalCode(&final);
            getTemporary(&(final->t));

            IRcode *labCode = (IRcode *)malloc(sizeof(IRcode));
            labCode->ele = (quad *)malloc(sizeof(quad));
            strcpy(labCode->ele->op, ":\0");
            strcpy(labCode->ele->arg1, l->arg2);
            labCode->next = NULL;
            mergeCode(&(leftchild->code), labCode);
            mergeCode(&(leftchild->code), rightchild->code);
            final->code = leftchild->code;
            free(leftchild);
            free(rightchild);

            return final;
        }
        else
        {
            intermed* leftchild = generateIRCode(currentNode->child, labels, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);

            intermed* final;
            initializeFinalCode(&final);
            getTemporary(&(final->t));
            
            getOp(currentNode->node->ele.internalNode->label, final->code->ele->op);
            strcpy(final->code->ele->arg1, leftchild->t.name);
            strcpy(final->code->ele->arg2, rightchild->t.name);
            strcpy(final->code->ele->result, final->t.name);
            final->code->next = NULL;
            mergeCode(&(leftchild->code), rightchild->code);
            mergeCode(&(leftchild->code), final->code);
            final->code = leftchild->code;
            free(leftchild);
            free(rightchild);

            return final;
        }
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "ASSIGNOP"))
    {
        intermed* rightchild = generateIRCode(currentNode->child->sibling, NULL, tbStack);
        
        intermed* final;
        initializeFinalCode(&final);
        strcpy(final->code->ele->op, "=\0");
        strcpy(final->code->ele->result , currentNode->child->node->ele.leafNode->lexeme);
        strcpy(final->code->ele->arg1 , rightchild->t.name);
        
        //when arg2 is empty , we know that it is assignop
        strcpy(final->code->ele->arg2 , "\0");
        final->code->next = NULL;
        mergeCode(&(rightchild->code), final->code);
        final->code = rightchild->code;
        free(rightchild);

        return final;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "ASSIGNOPARR"))
    {
        intermed * tmp = (intermed *)malloc(sizeof(intermed));
        tmp->code = NULL;

        symbolTableNode * node = searchScope(tbStack, currentNode->child);

        if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "ID") ||
        node->ele.data.arr.isDynamic == 1)
        {
            IRcode * ifcode1 = (IRcode *)malloc(sizeof(IRcode));
            ifcode1->ele = (quad *)malloc(sizeof(quad));
            ifcode1->next = NULL;
            
            strcpy(ifcode1->ele->op, "<=\0");
            initQuad(ifcode1->ele, currentNode->child->sibling->node->ele.leafNode->lexeme,
            node->ele.data.arr.upperIndex->lexeme, "if\0");

            IRcode * goto1 = (IRcode *)malloc(sizeof(IRcode));
            goto1->ele = (quad *)malloc(sizeof(quad));
            goto1->next = NULL;
            
            strcpy(goto1->ele->op, "goto\0");
            initQuad(goto1->ele, "\0", "\0", "\0");
            getLabel(goto1->ele->arg1);

            IRcode * goto2 = (IRcode *)malloc(sizeof(IRcode));
            goto2->ele = (quad *)malloc(sizeof(quad));
            goto2->next = NULL;
            
            strcpy(goto2->ele->op, "goto\0");
            initQuad(goto2->ele, "RUNTIME_ERROR\0", "\0", "\0");

            IRcode * label1 = (IRcode *)malloc(sizeof(IRcode));
            label1->ele = (quad *)malloc(sizeof(quad));
            label1->next = NULL;
            
            strcpy(label1->ele->op, ":\0");
            initQuad(label1->ele, goto1->ele->arg1, "\0", "\0");

            IRcode * ifcode2 = (IRcode *)malloc(sizeof(IRcode));
            ifcode2->ele = (quad *)malloc(sizeof(quad));
            ifcode2->next = NULL;
            
            strcpy(ifcode2->ele->op, ">=\0");
            initQuad(ifcode2->ele, currentNode->child->sibling->node->ele.leafNode->lexeme,
            node->ele.data.arr.lowerIndex->lexeme, "if\0");

            IRcode * goto3 = (IRcode *)malloc(sizeof(IRcode));
            goto3->ele = (quad *)malloc(sizeof(quad));
            goto3->next = NULL;
            
            strcpy(goto3->ele->op, "goto\0");
            initQuad(goto3->ele, "\0", "\0", "\0");
            getLabel(goto3->ele->arg1);

            IRcode * goto4 = (IRcode *)malloc(sizeof(IRcode));
            goto4->ele = (quad *)malloc(sizeof(quad));
            goto4->next = NULL;
            
            strcpy(goto4->ele->op, "goto\0");
            initQuad(goto4->ele, "RUNTIME_ERROR\0", "\0", "\0");

            IRcode * label2 = (IRcode *)malloc(sizeof(IRcode));
            label2->ele = (quad *)malloc(sizeof(quad));
            label2->next = NULL;
            
            strcpy(label2->ele->op, ":\0");
            initQuad(label2->ele, goto3->ele->arg1, "\0", "\0");

            mergeCode(&(tmp->code), ifcode1);
            mergeCode(&(tmp->code), goto1);
            mergeCode(&(tmp->code), goto2);
            mergeCode(&(tmp->code), label1);
            mergeCode(&(tmp->code), ifcode2);
            mergeCode(&(tmp->code), goto3);
            mergeCode(&(tmp->code), goto4);
            mergeCode(&(tmp->code), label2);
        }

        intermed* rightchild = generateIRCode(currentNode->child->sibling->sibling, NULL, tbStack);
        intermed* final;
        initializeFinalCode(&final); 
        strcpy(final->code->ele->op, "=\0");
        strcpy(final->code->ele->result , currentNode->child->node->ele.leafNode->lexeme);
        strcpy(final->code->ele->arg1 , currentNode->child->sibling->node->ele.leafNode->lexeme);
        
        //when arg2 is not empty we know that it is assignoparr
        strcpy(final->code->ele->arg2 , rightchild->t.name);
        final->code->next = NULL;
        mergeCode(&(rightchild->code), tmp->code);
        mergeCode(&(rightchild->code), final->code);
        final->code = rightchild->code;
        free(rightchild);

        return final;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULES1"))
    {
        astNode *trav = currentNode->child;

        intermed* final = (intermed *)malloc(sizeof(intermed));
        final->code = NULL;
        intermed* tmp;

        tableStackEle *newTable = NULL;
        symbolTable *st = symbolTableRoot->child->sibling;
        
        while(trav != NULL)
        {
            newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
            newTable->ele = st;
            newTable->next = NULL;
            sympush(tbStack, newTable);
            st = st->sibling;
            tmp = generateIRCode(trav, NULL, tbStack);
            mergeCode(&(final->code),tmp->code);
            trav = trav->sibling;
        }
        return final;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULES2"))
    {
        astNode *trav = currentNode->child;

        intermed* final = (intermed *)malloc(sizeof(intermed));
        final->code = NULL;
        intermed* tmp;

        tableStackEle *newTable = NULL;
        symbolTable *st = symbolTableRoot->child->sibling;
        while(strcmp(st->symLexeme, "Driver"))
        {
            st = st->sibling;
        }
        st = st->sibling;

        while(trav!=NULL)
        {
            newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
            newTable->ele = st;
            newTable->next = NULL;
            sympush(tbStack, newTable);
            st = st->sibling;
            tmp = generateIRCode(trav, NULL, tbStack);
            mergeCode(&(final->code),tmp->code);
            trav = trav->sibling;
        }
        return final;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULE"))
    {
        //4 children:
        //      1) ID_node
        //      2) input_list
        //      3) ret
        //      4) moduledef
        
        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->ele = tbStack->top->ele->child;
        newTable->next = NULL;

        //Push the moduleDef's symbol table on the stack
        sympush(tbStack, newTable);

        IRcode* finaldef = (IRcode *)malloc(sizeof(IRcode)); 
        finaldef->next=NULL;   
        finaldef->ele = (quad*)malloc(sizeof(quad));      
        strcpy(finaldef->ele->op,":\0");

        initQuad(finaldef->ele, currentNode->child->node->ele.leafNode->lexeme, "\0" ,"\0");
        
        quad *l = (quad *)malloc(sizeof(quad));
        strcpy(l->op, "\0");
        initQuad(l, "\0", "\0", "\0");
        getLabel(l->arg1);

        intermed* body = NULL;
        
        body = generateIRCode(currentNode->child->sibling->sibling->sibling, l, tbStack);
        
        IRcode *labelCode = (IRcode *)malloc(sizeof(IRcode));
        labelCode->ele = (quad *)malloc(sizeof(quad));
        labelCode->next = NULL;
        strcpy(labelCode->ele->op, ":\0");
        initQuad(labelCode->ele, l->arg1, "\0", "\0");

        IRcode* ret = (IRcode *)malloc(sizeof(IRcode)); 
        ret->next=NULL;        
        ret->ele = (quad *)malloc(sizeof(quad));
        strcpy(ret->ele->op,"RET\0");
        initQuad(ret->ele, "\0","\0","\0");

        mergeCode(&(finaldef), body->code);
        mergeCode(&(finaldef), labelCode);
        mergeCode(&(finaldef), ret);
        
        body->code = finaldef;
        
        sympop(tbStack);

        return body;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "DRIVER"))
    {  
        symbolTable *trav = symbolTableRoot->child->sibling;
        while(strcmp(trav->symLexeme, "Driver"))
        {
            trav = trav->sibling;
        }

        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->ele = trav;
        newTable->next = NULL;
        //Push the driver's symbol table on the stack
        sympush(tbStack, newTable);
        
        newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->ele = trav->child;
        newTable->next = NULL;
        //Push the moduleDef's symbol table on the stack
        sympush(tbStack, newTable);

        IRcode* finaldef = (IRcode *)malloc(sizeof(IRcode)); 
        finaldef->next=NULL;   
        finaldef->ele = (quad*)malloc(sizeof(quad));      
        strcpy(finaldef->ele->op,":\0");

        initQuad(finaldef->ele, "main", "\0" ,"\0");   

        quad *l = (quad *)malloc(sizeof(quad));
        strcpy(l->op, "\0");
        initQuad(l, "\0", "\0", "\0");
        getLabel(l->arg1);

        intermed* body = generateIRCode(currentNode->child, l, tbStack);

        IRcode *labelCode = (IRcode *)malloc(sizeof(IRcode));
        labelCode->ele = (quad *)malloc(sizeof(quad));
        labelCode->next = NULL;
        strcpy(labelCode->ele->op, ":\0");
        initQuad(labelCode->ele, l->arg1, "\0", "\0");

        IRcode* ret = (IRcode *)malloc(sizeof(IRcode)); 
        ret->next=NULL;        
        ret->ele = (quad *)malloc(sizeof(quad));
        strcpy(ret->ele->op,"RET\0");
        initQuad(ret->ele, "\0","\0","\0");

        mergeCode(&(finaldef), body->code);
        mergeCode(&(finaldef), labelCode);
        mergeCode(&(finaldef), ret);
        
        body->code = finaldef;
                
        sympop(tbStack);
        return body;
    }
    //print
    //get_value
    //for   -Done
    //while -Done
    //switch    -Done
    //assign    -Done
    //assignoparr   -Done
    //modulcall     -Done
    //Case  -Done
    //Default -Done
    else if(!strcmp(currentNode->node->ele.internalNode->label, "FOR"))
    {
        // For - 3 Children
        // ID
        // Range 
        // Statements

        //Code for ID = <lowerBound>
        IRcode * assignCode = (IRcode *)malloc(sizeof(IRcode));
        assignCode->ele = (quad *)malloc(sizeof(quad));
        assignCode->next = NULL;

        strcpy(assignCode->ele->op, "=\0");
        initQuad(assignCode->ele, currentNode->child->sibling->child->node->ele.leafNode->lexeme,
        "\0", currentNode->child->node->ele.leafNode->lexeme);

        //Code for goto label 1
        IRcode * goto1 = (IRcode *)malloc(sizeof(IRcode));
        goto1->ele = (quad *)malloc(sizeof(quad));
        goto1->next = NULL;
        strcpy(goto1->ele->op, "goto\0");
        getLabel(goto1->ele->arg1);
        strcpy(goto1->ele->arg2, "\0");
        strcpy(goto1->ele->result, "\0");

        //Code for if
        IRcode *ifcode = (IRcode *)malloc(sizeof(IRcode));
        ifcode->ele = (quad *)malloc(sizeof(quad));
        ifcode->next = NULL;

        strcpy(ifcode->ele->op, "<=\0");
        initQuad(ifcode->ele, currentNode->child->node->ele.leafNode->lexeme, 
        currentNode->child->sibling->child->sibling->node->ele.leafNode->lexeme,
        "if\0");

        //Code for goto label 2
        IRcode *goto2 = (IRcode *)malloc(sizeof(IRcode));
        goto2->ele = (quad *)malloc(sizeof(quad));
        goto2->next = NULL;

        strcpy(goto2->ele->op, "goto\0");
        getLabel(goto2->ele->arg1);
        strcpy(goto2->ele->arg2, "\0");
        strcpy(goto2->ele->result, "\0");

        //Code for goto lable 3
        IRcode *goto3 = (IRcode *)malloc(sizeof(IRcode));
        goto3->ele = (quad *)malloc(sizeof(quad));
        goto3->next = NULL;

        strcpy(goto3->ele->op, "goto\0");
        strcpy(goto3->ele->arg1, labels->arg1);
        strcpy(goto3->ele->arg2, "\0");
        strcpy(goto3->ele->result, "\0");

        quad* stmtsLabel = (quad *)malloc(sizeof(quad));
        strcpy(stmtsLabel->op, "\0");
        initQuad(stmtsLabel, goto1->ele->arg1, "\0", "\0");

        astNode * trav = currentNode->child->sibling;
        intermed* stmtsCode, *stmtCode;

        stmtsCode = (intermed*)malloc(sizeof(intermed));
        stmtsCode->code = NULL;

        int isEnd = 0;
        tableStackEle *newNode = NULL;
        symbolTable *st = tbStack->top->ele->child;
        while(trav != NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") ||
            !strcmp(trav->node->ele.internalNode->label, "WHILE") ||
            !strcmp(trav->node->ele.internalNode->label, "SWITCH"))
            {
                newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
                newNode->ele = st;
                newNode->next = NULL;
                sympush(tbStack, newNode);
                st = st->sibling;
                //If there is some stmt after this stmt
                if(trav->sibling != NULL)
                {
                    getLabel(stmtsLabel->arg1);
                    stmtCode = generateIRCode(trav, stmtsLabel, tbStack);

                    IRcode * labelCode = (IRcode *)malloc(sizeof(IRcode));
                    labelCode->ele = (quad *)malloc(sizeof(quad));
                    labelCode->next = NULL;
                    strcpy(labelCode->ele->op, ":\0");
                    initQuad(labelCode->ele, stmtsLabel->arg1, "\0", "\0");
                    
                    mergeCode(&(stmtsCode->code), stmtCode->code);
                    mergeCode(&(stmtsCode->code), labelCode);
                }
                //If this is the last stmt
                else
                {
                    strcpy(stmtsLabel->arg1, goto1->ele->arg1);
                    stmtCode = generateIRCode(trav, stmtsLabel, tbStack);
                    mergeCode(&(stmtsCode->code), stmtCode->code);

                    //Now, there isn't any need for the "goto begin" code
                    isEnd = 1;
                }
            }
            else
            {
                stmtCode = generateIRCode(trav, NULL, tbStack);
                mergeCode(&(stmtsCode->code), stmtCode->code);
            }
            trav = trav->sibling;
        }   

        //Label 1
        IRcode *label1 = (IRcode *)malloc(sizeof(IRcode));
        label1->ele = (quad *)malloc(sizeof(quad));
        label1->next = NULL;

        strcpy(label1->ele->op, ":\0");
        initQuad(label1->ele, goto1->ele->arg1, "\0", "\0");

        //Label 2
        IRcode *label2 = (IRcode *)malloc(sizeof(IRcode));
        label2->ele = (quad *)malloc(sizeof(quad));
        label2->next = NULL;

        strcpy(label2->ele->op, ":\0");
        initQuad(label2->ele, goto2->ele->arg1, "\0", "\0");

        //Increment ID
        IRcode *incr = (IRcode *)malloc(sizeof(IRcode));
        incr->ele = (quad *)malloc(sizeof(quad));
        incr->next = NULL;

        strcpy(incr->ele->op, "+\0");
        initQuad(incr->ele, currentNode->child->node->ele.leafNode->lexeme, "1\0",
        currentNode->child->node->ele.leafNode->lexeme);


        intermed * final = (intermed*)malloc(sizeof(intermed));
        final->code = NULL;
        mergeCode(&(final->code), assignCode);
        mergeCode(&(final->code), label1);
        mergeCode(&(final->code), ifcode);
        mergeCode(&(final->code), goto2);
        mergeCode(&(final->code), goto3);
        mergeCode(&(final->code), label2);
        mergeCode(&(final->code), stmtsCode->code);
        mergeCode(&(final->code), incr);
        if(isEnd == 0)
            mergeCode(&(final->code), goto1);
        
        sympop(tbStack);
        return final;
    }

    // while(x)
    //  y = 1;

    // L1: condi check
    // L2: 

    else if(!strcmp(currentNode->node->ele.internalNode->label, "WHILE"))
    {
        //every time we begin, we should check the condition
        quad *begin = (quad *)malloc(sizeof(quad));
        getLabel(begin->arg1);
        strcpy(begin->op, ":\0");

        quad *l = (quad *)malloc(sizeof(quad));
        //B.true is the new label 
        getLabel(l->arg1);
        //while's next is in arg1, which is the B.false
        strcpy(l->arg2, labels->arg1);
        
        IRcode *beginCode = (IRcode *)malloc(sizeof(IRcode));
        beginCode->ele = begin;
        beginCode->next = NULL;
        
        quad* stmtsLabel = (quad *)malloc(sizeof(quad));
        strcpy(stmtsLabel->op, "\0");
        initQuad(stmtsLabel, begin->arg1, "\0", "\0");
        
        IRcode *goto1 = (IRcode *)malloc(sizeof(IRcode));
        goto1->ele = (quad *)malloc(sizeof(quad));
        strcpy(goto1->ele->op, "goto\0");
        initQuad(goto1->ele, begin->arg1, "\0", "\0");

        intermed * exprCode = generateIRCode(currentNode->child, l, tbStack);

        astNode * trav = currentNode->child->sibling;
        intermed* stmtsCode, *stmtCode;

        stmtsCode = (intermed*)malloc(sizeof(intermed));
        stmtsCode->code = NULL;

        //isEnd = 0 means there is need for "goto begin" stmt at the end of this loop
        int isEnd = 0;
        
        tableStackEle * newNode = NULL;
        symbolTable * st = tbStack->top->ele->child;
        while(trav != NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") ||
            !strcmp(trav->node->ele.internalNode->label, "WHILE") ||
            !strcmp(trav->node->ele.internalNode->label, "SWITCH"))
            {
                newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
                newNode->ele = st;
                newNode->next = NULL;
                sympush(tbStack, newNode);
                st = st->sibling;
                
                // remember that it is the parent node's duty to enumerate the 
                // children with labels if they (any child) need the label of 
                // the right sibling

                //If there is some stmt after this stmt
                if(trav->sibling != NULL)
                {
                    getLabel(stmtsLabel->arg1);
                    stmtCode = generateIRCode(trav, stmtsLabel, tbStack);

                    IRcode * labelCode = (IRcode *)malloc(sizeof(IRcode));
                    labelCode->ele = (quad *)malloc(sizeof(quad));
                    labelCode->next = NULL;
                    strcpy(labelCode->ele->op, ":\0");
                    initQuad(labelCode->ele, stmtsLabel->arg1, "\0", "\0");
                    
                    mergeCode(&(stmtsCode->code), stmtCode->code);
                    mergeCode(&(stmtsCode->code), labelCode);
                }
                //If this is the last stmt
                else
                {
                    strcpy(stmtsLabel->arg1, begin->arg1);
                    stmtCode = generateIRCode(trav, stmtsLabel, tbStack);
                    mergeCode(&(stmtsCode->code), stmtCode->code);

                    //Now, there isn't any need for the "goto begin" code
                    isEnd = 1;
                }
            }
            else
            {
                stmtCode = generateIRCode(trav, NULL, tbStack);
                mergeCode(&(stmtsCode->code), stmtCode->code);
            }
            trav = trav->sibling;
        }

        IRcode *trueLabel = (IRcode *)malloc(sizeof(IRcode));
        trueLabel->ele = (quad *)malloc(sizeof(quad));
        trueLabel->next = NULL;
        strcpy(trueLabel->ele->op, ":\0");
        initQuad(trueLabel->ele, l->arg1, "\0", "\0");

        intermed * final = (intermed*)malloc(sizeof(intermed));
        final->code = NULL;
        mergeCode(&(final->code), beginCode);
        mergeCode(&(final->code), exprCode->code);
        mergeCode(&(final->code), trueLabel);
        mergeCode(&(final->code), stmtsCode->code);
        if(isEnd == 0)
            mergeCode(&(final->code), goto1);
        sympop(tbStack);
        return final;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "SWITCH"))
    {
        // ID Case Statements Default 
        char * idname = currentNode->child->node->ele.leafNode->lexeme;     

        astNode* trav = currentNode->child->sibling;
        int len = 0;
        while(trav != NULL)
        {
            len++;
            trav = trav->sibling;
        }
        intermed* cummulator = (intermed*)malloc(sizeof(intermed));
        cummulator->code = NULL;
        
        intermed** caseStmts = (intermed**) malloc(sizeof(intermed*)*len);
        intermed* switchcode;
        trav = currentNode->child->sibling;
        len = 0;
        while(trav!=NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label,"DEFAULT"))
            {
                quad* defaultLabel = (quad *)malloc(sizeof(quad));
                getLabel(defaultLabel->arg2);
                //arg1 is the next label after the switch construct itself
                strcpy(defaultLabel->arg1, labels->arg1);
                                
                IRcode *gotoDefault = (IRcode *)malloc(sizeof(IRcode));
                gotoDefault->ele = (quad *)malloc(sizeof(quad));
                gotoDefault->next = NULL;
                strcpy(gotoDefault->ele->op, "goto\0");
                initQuad(gotoDefault->ele, defaultLabel->arg2, "\0", "\0");

                mergeCode(&(cummulator->code), gotoDefault);

                caseStmts[len] = generateIRCode(trav, defaultLabel, tbStack);                       
            }
            else
            {
                quad* caseLabel = (quad *)malloc(sizeof(quad));
                getLabel(caseLabel->arg2);
                strcpy(caseLabel->arg1, labels->arg1);

                IRcode *ifcode = (IRcode *)malloc(sizeof(IRcode));
                ifcode->ele = (quad *)malloc(sizeof(quad));
                ifcode->next = NULL;

                strcpy(ifcode->ele->op, "==\0");
                initQuad(ifcode->ele, idname, trav->child->node->ele.leafNode->lexeme,"if\0");
                
                IRcode *gotoCase = (IRcode *)malloc(sizeof(IRcode));
                gotoCase->ele = (quad *)malloc(sizeof(quad));
                gotoCase->next = NULL;
                strcpy(gotoCase->ele->op, "goto\0");
                initQuad(gotoCase->ele, caseLabel->arg2, "\0", "\0");

                mergeCode(&(cummulator->code), ifcode);
                mergeCode(&(cummulator->code), gotoCase);

                caseStmts[len] = generateIRCode(trav, caseLabel, tbStack);       
            }   
            trav = trav->sibling;
            len++;     
        }
    
        for(int i = 0 ; i < len ; i++)
        {
            mergeCode(&(cummulator->code), caseStmts[i]->code);
        }
        free(caseStmts);
        sympop(tbStack);
        return cummulator;
    }

    else if(!strcmp(currentNode->node->ele.internalNode->label, "CASE") ||
    !strcmp(currentNode->node->ele.internalNode->label, "DEFAULT"))
    {
        quad* stmtsLabel = (quad *)malloc(sizeof(quad));
        strcpy(stmtsLabel->op, "\0");
        initQuad(stmtsLabel, labels->arg1, "\0", "\0");

        astNode * trav = NULL;
        if(!strcmp(currentNode->node->ele.internalNode->label, "DEFAULT"))
            trav = currentNode->child;
        else
            trav = currentNode->child->sibling;
            
        intermed* stmtsCode, *stmtCode;
        stmtsCode = (intermed *)malloc(sizeof(intermed));
        
        //isEnd = 0 means there is need for "goto begin" stmt at the end of this loop
        int isEnd = 0;
        tableStackEle * newNode = NULL;
        symbolTable * st = tbStack->top->ele->child;
        while(trav != NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") ||
            !strcmp(trav->node->ele.internalNode->label, "WHILE") ||
            !strcmp(trav->node->ele.internalNode->label, "SWITCH"))
            {
                newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
                newNode->ele = st;
                newNode->next = NULL;
                sympush(tbStack, newNode);
                st = st->sibling;
                // remember that it is the parent node's duty to enumerate the 
                // children with labels if they (any child) need the label of 
                // the right sibling

                //If there is some stmt after this stmt
                if(trav->sibling != NULL)
                {
                    getLabel(stmtsLabel->arg1);
                    stmtCode = generateIRCode(trav, stmtsLabel, tbStack);

                    IRcode * labelCode = (IRcode *)malloc(sizeof(IRcode));
                    labelCode->ele = (quad *)malloc(sizeof(quad));
                    labelCode->next = NULL;
                    strcpy(labelCode->ele->op, ":\0");
                    initQuad(labelCode->ele, stmtsLabel->arg1, "\0", "\0");
                    
                    mergeCode(&(stmtsCode->code), stmtCode->code);
                    mergeCode(&(stmtsCode->code), labelCode);
                }
                //If this is the last stmt
                else
                {
                    strcpy(stmtsLabel->arg1, labels->arg1);
                    stmtCode = generateIRCode(trav, stmtsLabel, tbStack);
                    mergeCode(&(stmtsCode->code), stmtCode->code);

                    //Now, there isn't any need for the "goto begin" code
                    isEnd = 1;
                }
            }
            else
            {
                stmtCode = generateIRCode(trav, NULL, tbStack);
                mergeCode(&(stmtsCode->code), stmtCode->code);
            }
            trav = trav->sibling;
        }

        //Code for label of the case
        IRcode *labelCode = (IRcode *)malloc(sizeof(IRcode));
        labelCode->ele = (quad *)malloc(sizeof(quad));
        labelCode->next = NULL;
        strcpy(labelCode->ele->op, ":\0");
        initQuad(labelCode->ele, labels->arg2,"\0", "\0");

        //Code for goto next
        IRcode *gotoNext = (IRcode *)malloc(sizeof(IRcode));
        gotoNext->ele = (quad *)malloc(sizeof(quad));
        gotoNext->next = NULL;
        strcpy(gotoNext->ele->op, "goto\0");
        initQuad(gotoNext->ele, labels->arg1,"\0", "\0");

        //Code for case
        intermed *final;
        final = (intermed *)malloc(sizeof(intermed));
        final->code = NULL;

        mergeCode(&(final->code), labelCode);
        mergeCode(&(final->code), stmtsCode->code);
        mergeCode(&(final->code), gotoNext);

        return final;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULEDEF"))
    {
        // Statements as Children - Sibling lists
        
        quad* stmtsLabel = (quad *)malloc(sizeof(quad));
        strcpy(stmtsLabel->op, "\0");
        initQuad(stmtsLabel, labels->arg1, "\0", "\0");

        astNode * trav = currentNode->child;
        intermed* stmtsCode, *stmtCode;

        stmtsCode = (intermed*)malloc(sizeof(intermed));
        stmtsCode->code = NULL;
        //isEnd = 0 means there is need for "goto begin" stmt at the end of this loop
        int isEnd = 0;
        
        tableStackEle * newNode = NULL;
        symbolTable * st = tbStack->top->ele->child;
        while(trav != NULL)
        {
            if(!strcmp(trav->node->ele.internalNode->label, "FOR") ||
            !strcmp(trav->node->ele.internalNode->label, "WHILE") ||
            !strcmp(trav->node->ele.internalNode->label, "SWITCH"))
            {
                newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
                newNode->ele = st;
                newNode->next = NULL;
                sympush(tbStack, newNode);
                st = st->sibling;

                // remember that it is the parent node's duty to enumerate the 
                // children with labels if they (any child) need the label of 
                // the right sibling

                //If there is some stmt after this stmt
                if(trav->sibling != NULL)
                {
                    getLabel(stmtsLabel->arg1);
                    stmtCode = generateIRCode(trav, stmtsLabel, tbStack);

                    IRcode * labelCode = (IRcode *)malloc(sizeof(IRcode));
                    labelCode->ele = (quad *)malloc(sizeof(quad));
                    labelCode->next = NULL;
                    strcpy(labelCode->ele->op, ":\0");
                    initQuad(labelCode->ele, stmtsLabel->arg1, "\0", "\0");
                    
                    mergeCode(&(stmtsCode->code), stmtCode->code);
                    mergeCode(&(stmtsCode->code), labelCode);
                }
                //If this is the last stmt
                else
                {
                    strcpy(stmtsLabel->arg1, labels->arg1);
                    stmtCode = generateIRCode(trav, stmtsLabel, tbStack);
                    mergeCode(&(stmtsCode->code), stmtCode->code);

                    //Now, there isn't any need for the "goto begin" code
                    isEnd = 1;
                }
            }
            else
            {
                stmtCode = generateIRCode(trav, NULL, tbStack);
                mergeCode(&(stmtsCode->code), stmtCode->code);
            }
            trav = trav->sibling;
        }

        intermed * final = (intermed *)malloc(sizeof(intermed));
        final->code = stmtsCode->code;
        sympop(tbStack);
        return final;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "PRINT"))
    {
        // If ID_ARR is not accessing an array element
        if(currentNode->child->child->sibling == NULL)
        {
            // Case 1 : ID is an Array
            symbolTableNode * node = searchScope(tbStack, currentNode->child->child);
            intermed * final = NULL;
            if(node->ele.tag == Array)
            {
                final = (intermed *)malloc(sizeof(intermed));
                final->code = NULL;
                
                IRcode * assignCode = (IRcode *)malloc(sizeof(IRcode));
                assignCode->ele = (quad *)malloc(sizeof(quad));
                assignCode->next = NULL;
                
                strcpy(assignCode->ele->op, "=\0");
                initQuad(assignCode->ele, node->ele.data.arr.lowerIndex->lexeme,
                "\0", "\0");

                temporary tmp;
                getTemporary(&tmp);
                strcpy(assignCode->ele->result, tmp.name);

                //Code for goto label 1
                IRcode * goto1 = (IRcode *)malloc(sizeof(IRcode));
                goto1->ele = (quad *)malloc(sizeof(quad));
                goto1->next = NULL;
                strcpy(goto1->ele->op, "goto\0");
                getLabel(goto1->ele->arg1);
                strcpy(goto1->ele->arg2, "\0");
                strcpy(goto1->ele->result, "\0");

                //Code for if
                IRcode *ifcode = (IRcode *)malloc(sizeof(IRcode));
                ifcode->ele = (quad *)malloc(sizeof(quad));
                ifcode->next = NULL;

                strcpy(ifcode->ele->op, "<=\0");
                initQuad(ifcode->ele, assignCode->ele->result, 
                node->ele.data.arr.upperIndex->lexeme,
                "if\0");

                //Code for goto label 2
                IRcode *goto2 = (IRcode *)malloc(sizeof(IRcode));
                goto2->ele = (quad *)malloc(sizeof(quad));
                goto2->next = NULL;

                strcpy(goto2->ele->op, "goto\0");
                getLabel(goto2->ele->arg1);
                strcpy(goto2->ele->arg2, "\0");
                strcpy(goto2->ele->result, "\0");

                //Code for goto lable 3
                IRcode *goto3 = (IRcode *)malloc(sizeof(IRcode));
                goto3->ele = (quad *)malloc(sizeof(quad));
                goto3->next = NULL;

                strcpy(goto3->ele->op, "goto\0");
                getLabel(goto3->ele->arg1);
                strcpy(goto3->ele->arg2, "\0");
                strcpy(goto3->ele->result, "\0");

                //Code for label 1
                IRcode * label1 = (IRcode *)malloc(sizeof(IRcode));
                label1->ele = (quad *)malloc(sizeof(quad));
                label1->next = NULL;
                
                strcpy(label1->ele->op, ":\0");
                initQuad(label1->ele, goto1->ele->arg1, "\0", "\0");

                //Code for lable 2
                IRcode * label2 = (IRcode *)malloc(sizeof(IRcode));
                label2->ele = (quad *)malloc(sizeof(quad));
                label2->next = NULL;
                
                strcpy(label2->ele->op, ":\0");
                initQuad(label2->ele, goto2->ele->arg1, "\0", "\0");

                //Code for label 3
                IRcode * label3 = (IRcode *)malloc(sizeof(IRcode));
                label3->ele = (quad *)malloc(sizeof(quad));
                label3->next = NULL;
                
                strcpy(label3->ele->op, ":\0");
                initQuad(label3->ele, goto3->ele->arg1, "\0", "\0");

                //Code for incrementation
                IRcode *incr = (IRcode *)malloc(sizeof(IRcode));
                incr->ele = (quad *)malloc(sizeof(quad));
                incr->next = NULL;

                strcpy(incr->ele->op, "+\0");
                initQuad(incr->ele, assignCode->ele->result, "1\0",
                assignCode->ele->result);

                //Code for scanf
                IRcode *print = (IRcode *)malloc(sizeof(IRcode));
                print->ele = (quad *)malloc(sizeof(quad));
                print->next = NULL;

                strcpy(print->ele->op, "printf\0");
                initQuad(print->ele, assignCode->ele->result, "\0",
                currentNode->child->child->node->ele.leafNode->lexeme);

                mergeCode(&(final->code), assignCode);
                mergeCode(&(final->code), label1);
                mergeCode(&(final->code), ifcode);
                mergeCode(&(final->code), goto2);
                mergeCode(&(final->code), goto3);
                mergeCode(&(final->code), label2);
                mergeCode(&(final->code), print);
                mergeCode(&(final->code), incr);
                mergeCode(&(final->code), goto1);
                mergeCode(&(final->code), label3);

                return final;
            }
            else
            {
                // Case 2 : ID is not an Array
                initializeFinalCode(&final);
                strcpy(final->code->ele->op, "printf\0");
                strcpy(final->code->ele->arg1, currentNode->child->child->node->ele.leafNode->lexeme);
                strcpy(final->code->ele->arg2, "\0");
                strcpy(final->code->ele->result, "\0");
                return final;
            }
        }
        else
        {
            intermed * final = NULL;
            final->code = NULL;

            intermed *child = generateIRCode(currentNode->child, labels, tbStack);

            // IRcode * assign  = (IRcode *)malloc(sizeof(IRcode));
            // assign->ele = (quad *)malloc(sizeof(quad));
            // assign->next = NULL;
            // strcpy(assign->ele->op, "=");
            // getTemporary(&(child->t));
            // strcpy(assign->ele->arg1, currentNode->child->child->node->ele.leafNode->lexeme);
            // strcpy(assign->ele->arg2, currentNode->child->child->sibling->node->ele.leafNode->lexeme);
            // strcpy(assign->ele->result, child->t.name);
            IRcode * print  = (IRcode *)malloc(sizeof(IRcode));
            print->ele = (quad *)malloc(sizeof(quad));
            print->next = NULL;
            strcpy(print->ele->op, "printf\0");
            initQuad(print->ele, child->t.name, "\0", "\0");
                       
            mergeCode(&(final->code), child->code);
            mergeCode(&(final->code), print);
            
            return final;
        }
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "GET_VAL"))
    {
        intermed* final;
               
        //TODO: Case when ID is an arrayID (to be checked using symbol table)
        symbolTableNode * node = searchScope(tbStack, currentNode->child);

        if(node->ele.tag == Array)
        {
            final = (intermed *)malloc(sizeof(intermed));
            final->code = NULL;
            
            IRcode * assignCode = (IRcode *)malloc(sizeof(IRcode));
            assignCode->ele = (quad *)malloc(sizeof(quad));
            assignCode->next = NULL;
            
            strcpy(assignCode->ele->op, "=\0");
            initQuad(assignCode->ele, node->ele.data.arr.lowerIndex->lexeme,
            "\0", "\0");

            temporary tmp;
            getTemporary(&tmp);
            strcpy(assignCode->ele->result, tmp.name);

            //Code for goto label 1
            IRcode * goto1 = (IRcode *)malloc(sizeof(IRcode));
            goto1->ele = (quad *)malloc(sizeof(quad));
            goto1->next = NULL;
            strcpy(goto1->ele->op, "goto\0");
            getLabel(goto1->ele->arg1);
            strcpy(goto1->ele->arg2, "\0");
            strcpy(goto1->ele->result, "\0");

            //Code for if
            IRcode *ifcode = (IRcode *)malloc(sizeof(IRcode));
            ifcode->ele = (quad *)malloc(sizeof(quad));
            ifcode->next = NULL;

            strcpy(ifcode->ele->op, "<=\0");
            initQuad(ifcode->ele, assignCode->ele->result, 
            node->ele.data.arr.upperIndex->lexeme,
            "if\0");

            //Code for goto label 2
            IRcode *goto2 = (IRcode *)malloc(sizeof(IRcode));
            goto2->ele = (quad *)malloc(sizeof(quad));
            goto2->next = NULL;

            strcpy(goto2->ele->op, "goto\0");
            getLabel(goto2->ele->arg1);
            strcpy(goto2->ele->arg2, "\0");
            strcpy(goto2->ele->result, "\0");

            //Code for goto lable 3
            IRcode *goto3 = (IRcode *)malloc(sizeof(IRcode));
            goto3->ele = (quad *)malloc(sizeof(quad));
            goto3->next = NULL;

            strcpy(goto3->ele->op, "goto\0");
            getLabel(goto3->ele->arg1);
            strcpy(goto3->ele->arg2, "\0");
            strcpy(goto3->ele->result, "\0");

            //Code for label 1
            IRcode * label1 = (IRcode *)malloc(sizeof(IRcode));
            label1->ele = (quad *)malloc(sizeof(quad));
            label1->next = NULL;
            
            strcpy(label1->ele->op, ":\0");
            initQuad(label1->ele, goto1->ele->arg1, "\0", "\0");

            //Code for lable 2
            IRcode * label2 = (IRcode *)malloc(sizeof(IRcode));
            label2->ele = (quad *)malloc(sizeof(quad));
            label2->next = NULL;
            
            strcpy(label2->ele->op, ":\0");
            initQuad(label2->ele, goto2->ele->arg1, "\0", "\0");

            //Code for label 3
            IRcode * label3 = (IRcode *)malloc(sizeof(IRcode));
            label3->ele = (quad *)malloc(sizeof(quad));
            label3->next = NULL;
            
            strcpy(label3->ele->op, ":\0");
            initQuad(label3->ele, goto3->ele->arg1, "\0", "\0");

            //Code for incrementation
            IRcode *incr = (IRcode *)malloc(sizeof(IRcode));
            incr->ele = (quad *)malloc(sizeof(quad));
            incr->next = NULL;

            strcpy(incr->ele->op, "+\0");
            initQuad(incr->ele, assignCode->ele->result, "1\0",
            assignCode->ele->result);

            //Code for scanf
            IRcode *scan = (IRcode *)malloc(sizeof(IRcode));
            scan->ele = (quad *)malloc(sizeof(quad));
            scan->next = NULL;

            strcpy(scan->ele->op, "scanf\0");
            initQuad(scan->ele, assignCode->ele->result, "\0",
            currentNode->child->node->ele.leafNode->lexeme);

            mergeCode(&(final->code), assignCode);
            mergeCode(&(final->code), label1);
            mergeCode(&(final->code), ifcode);
            mergeCode(&(final->code), goto2);
            mergeCode(&(final->code), goto3);
            mergeCode(&(final->code), label2);
            mergeCode(&(final->code), scan);
            mergeCode(&(final->code), incr);
            mergeCode(&(final->code), goto1);
            mergeCode(&(final->code), label3);

            return final;
        }
        else
        {
            //Case when ID isn't an arrayID
            initializeFinalCode(&final); 
            strcpy(final->code->ele->op, "scanf\0");
            initQuad(final->code->ele, currentNode->child->node->ele.leafNode->lexeme, "\0", "\0");
            return final;
        }
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "ID_ARR"))
    {
        //2 (max) children;
            //id_node
            //which_node
        // Checking if ID_ARR is just ID
        if(currentNode->child->sibling == NULL)
        {
            // Not a Boolean comparison
            if(labels == NULL)
            {
                intermed *final = (intermed*)malloc(sizeof(intermed));
                final->code = NULL;
                strcpy(final->t.name,currentNode->child->node->ele.leafNode->lexeme);
                return final;
            }
            // Used in Boolean Comparison
            else
            {
                intermed *final = (intermed*)malloc(sizeof(intermed));
                final->code = NULL;
                IRcode *ifcode = (IRcode *)malloc(sizeof(IRcode));
                ifcode->ele = (quad *)malloc(sizeof(quad));
                ifcode->next = NULL;

                strcpy(ifcode->ele->op, "==\0");
                initQuad(ifcode->ele, currentNode->child->node->ele.leafNode->lexeme, "TRUE\0","if\0");
                
                IRcode *gotoTrueCase = (IRcode *)malloc(sizeof(IRcode));
                gotoTrueCase->ele = (quad *)malloc(sizeof(quad));
                gotoTrueCase->next = NULL;
                strcpy(gotoTrueCase->ele->op, "goto\0");
                initQuad(gotoTrueCase->ele, labels->arg1, "\0", "\0");
                
                IRcode *gotoFalseCase = (IRcode *)malloc(sizeof(IRcode));
                gotoFalseCase->ele = (quad *)malloc(sizeof(quad));
                gotoFalseCase->next = NULL;
                strcpy(gotoFalseCase->ele->op, "goto\0");
                initQuad(gotoFalseCase->ele, labels->arg2, "\0", "\0");
                                
                mergeCode(&(final->code), ifcode);
                mergeCode(&(final->code), gotoTrueCase);
                mergeCode(&(final->code), gotoFalseCase);
                return final;
            }
        }
        // ID_ARR is of Array Type
        else
        {
            //Add the dynamic type (bound, in our case) checking code
                //IF:
                //1) Either the array is dynamic
                //2) Index is an ID rather than being a NUM (doesn't matter whether array is static or dynamic)
            intermed * tmp = (intermed *)malloc(sizeof(intermed));
            tmp->code = NULL;
            
            symbolTableNode * node = searchScope(tbStack, currentNode->child);
        
            if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "ID") ||
            node->ele.data.arr.isDynamic == 1)
            {
                IRcode * ifcode1 = (IRcode *)malloc(sizeof(IRcode));
                ifcode1->ele = (quad *)malloc(sizeof(quad));
                ifcode1->next = NULL;
                
                strcpy(ifcode1->ele->op, "<=\0");
                initQuad(ifcode1->ele, currentNode->child->sibling->node->ele.leafNode->lexeme,
                node->ele.data.arr.upperIndex->lexeme, "if\0");

                IRcode * goto1 = (IRcode *)malloc(sizeof(IRcode));
                goto1->ele = (quad *)malloc(sizeof(quad));
                goto1->next = NULL;
                
                strcpy(goto1->ele->op, "goto\0");
                initQuad(goto1->ele, "\0", "\0", "\0");
                getLabel(goto1->ele->arg1);

                IRcode * goto2 = (IRcode *)malloc(sizeof(IRcode));
                goto2->ele = (quad *)malloc(sizeof(quad));
                goto2->next = NULL;
                
                strcpy(goto2->ele->op, "goto\0");
                initQuad(goto2->ele, "RUNTIME_ERROR\0", "\0", "\0");

                IRcode * label1 = (IRcode *)malloc(sizeof(IRcode));
                label1->ele = (quad *)malloc(sizeof(quad));
                label1->next = NULL;
                
                strcpy(label1->ele->op, ":\0");
                initQuad(label1->ele, goto1->ele->arg1, "\0", "\0");

                IRcode * ifcode2 = (IRcode *)malloc(sizeof(IRcode));
                ifcode2->ele = (quad *)malloc(sizeof(quad));
                ifcode2->next = NULL;
                
                strcpy(ifcode2->ele->op, ">=\0");
                initQuad(ifcode2->ele, currentNode->child->sibling->node->ele.leafNode->lexeme,
                node->ele.data.arr.lowerIndex->lexeme, "if\0");

                IRcode * goto3 = (IRcode *)malloc(sizeof(IRcode));
                goto3->ele = (quad *)malloc(sizeof(quad));
                goto3->next = NULL;
                
                strcpy(goto3->ele->op, "goto\0");
                initQuad(goto3->ele, "\0", "\0", "\0");
                getLabel(goto3->ele->arg1);

                IRcode * goto4 = (IRcode *)malloc(sizeof(IRcode));
                goto4->ele = (quad *)malloc(sizeof(quad));
                goto4->next = NULL;
                
                strcpy(goto4->ele->op, "goto\0");
                initQuad(goto4->ele, "RUNTIME_ERROR\0", "\0", "\0");

                IRcode * label2 = (IRcode *)malloc(sizeof(IRcode));
                label2->ele = (quad *)malloc(sizeof(quad));
                label2->next = NULL;
                
                strcpy(label2->ele->op, ":\0");
                initQuad(label2->ele, goto3->ele->arg1, "\0", "\0");

                mergeCode(&(tmp->code), ifcode1);
                mergeCode(&(tmp->code), goto1);
                mergeCode(&(tmp->code), goto2);
                mergeCode(&(tmp->code), label1);
                mergeCode(&(tmp->code), ifcode2);
                mergeCode(&(tmp->code), goto3);
                mergeCode(&(tmp->code), goto4);
                mergeCode(&(tmp->code), label2);
            }

            //Case 1: Part of while() condition
            intermed *final;
            if(labels != NULL)
            {
                final = (intermed*)malloc(sizeof(intermed));
                final->code = NULL;
                
                IRcode * assign  = (IRcode *)malloc(sizeof(IRcode));
                assign->ele = (quad *)malloc(sizeof(quad));
                assign->next = NULL;
                strcpy(assign->ele->op, "=");
                getTemporary(&(final->t));
                strcpy(assign->ele->arg1, currentNode->child->node->ele.leafNode->lexeme);
                strcpy(assign->ele->arg2, currentNode->child->sibling->node->ele.leafNode->lexeme);
                strcpy(assign->ele->result, final->t.name);

                IRcode *ifcode = (IRcode *)malloc(sizeof(IRcode));
                ifcode->ele = (quad *)malloc(sizeof(quad));
                ifcode->next = NULL;

                strcpy(ifcode->ele->op, "==\0");
                initQuad(ifcode->ele, final->t.name, "TRUE\0","if\0");
                
                IRcode *goto1 = (IRcode *)malloc(sizeof(IRcode));
                goto1->ele = (quad *)malloc(sizeof(quad));
                goto1->next = NULL;
                strcpy(goto1->ele->op, "goto\0");
                initQuad(goto1->ele, labels->arg1, "\0", "\0");

                IRcode *goto2 = (IRcode *)malloc(sizeof(IRcode));
                goto2->ele = (quad *)malloc(sizeof(quad));
                goto2->next = NULL;
                strcpy(goto2->ele->op, "goto\0");
                initQuad(goto2->ele, labels->arg2, "\0", "\0");

                mergeCode(&(final->code), assign);
                mergeCode(&(final->code), ifcode);
                mergeCode(&(final->code), goto1);
                mergeCode(&(final->code), goto2);
            }
            else
            {
                initializeFinalCode(&final);
                strcpy(final->code->ele->op, "=");
                getTemporary(&(final->t));
                strcpy(final->code->ele->arg1, currentNode->child->node->ele.leafNode->lexeme);
                strcpy(final->code->ele->arg2, currentNode->child->sibling->node->ele.leafNode->lexeme);
                strcpy(final->code->ele->result, final->t.name);
                tmp->t = final->t;
            }
            mergeCode(&(tmp->code), final->code);
            return tmp;
        }
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULECALL"))
    {
        astNode * trav = currentNode->child->sibling->sibling->child;

        intermed * final = (intermed *)malloc(sizeof(intermed));
        final->code = NULL;

        int paramCount = 0;
        while(trav != NULL)
        {
            IRcode * param = (IRcode *)malloc(sizeof(IRcode));
            param->ele = (quad *)malloc(sizeof(quad));
            param->next = NULL;
            strcpy(param->ele->op, "param\0");
            initQuad(param->ele, trav->node->ele.leafNode->lexeme, "\0", "\0");

            mergeCode(&param, final->code);
            final->code = param;
            paramCount++;
            trav = trav->sibling;
        }
        
        IRcode * call = (IRcode *)malloc(sizeof(IRcode));
        call->ele = (quad *)malloc(sizeof(quad));
        call->next = NULL;
        strcpy(call->ele->op, "\0");
        initQuad(call->ele, currentNode->child->sibling->node->ele.leafNode->lexeme, "\0", "call\0");
        sprintf(call->ele->arg2, "%d", paramCount);

        mergeCode(&(final->code), call);
        return final;
    }
    else if((currentNode->node->tag == Leaf))
    {
        if(!strcmp(currentNode->node->ele.leafNode->type, "NUM") 
        || !strcmp(currentNode->node->ele.leafNode->type, "RNUM"))
        {
            intermed * final = (intermed *)malloc(sizeof(intermed));
            final->code = NULL;
            strcpy(final->t.name, currentNode->node->ele.leafNode->lexeme);
            return final;
        }
        else
        {
            if(labels == NULL)
            {
                intermed * final = (intermed *)malloc(sizeof(intermed));
                final->code = NULL;
                strcpy(final->t.name, currentNode->node->ele.leafNode->lexeme);
                return final;
            }
            else
            {
                intermed * final = (intermed *)malloc(sizeof(intermed));
                final->code = NULL;
                IRcode *goto1;
                
                if(!strcmp(currentNode->node->ele.leafNode->type, "TRUE"))
                {
                    goto1 = (IRcode *)malloc(sizeof(IRcode));
                    goto1->ele = (quad *)malloc(sizeof(quad));
                    goto1->next = NULL;
                    strcpy(goto1->ele->op, "goto\0");
                    initQuad(goto1->ele, labels->arg1, "\0", "\0");
                }
                else
                {
                    goto1 = (IRcode *)malloc(sizeof(IRcode));
                    goto1->ele = (quad *)malloc(sizeof(quad));
                    goto1->next = NULL;
                    strcpy(goto1->ele->op, "goto\0");
                    initQuad(goto1->ele, labels->arg2, "\0", "\0");
                }

                mergeCode(&(final->code), goto1);
                return final;
            }
        }
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "PROGRAM"))
    {

        //Push scope of Program on top of the stack
        tableStackEle * newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->next = NULL;
        newTable->ele = symbolTableRoot;
        sympush(tbStack,newTable);

        // creating and pushing the moduledec table
        newTable= (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->next = NULL;
        newTable->ele = symbolTableRoot->child;
        sympush(tbStack, newTable);

        intermed * final = (intermed *)malloc(sizeof(intermed));
        final->code = NULL;

        intermed * mods1 = generateIRCode(currentNode->child->sibling, labels, tbStack);
        intermed * driver = generateIRCode(currentNode->child->sibling->sibling, labels, tbStack);
        intermed * mods2 = generateIRCode(currentNode->child->sibling->sibling->sibling, labels, tbStack);

        mergeCode(&(final->code), mods1->code);
        mergeCode(&(final->code), driver->code);
        mergeCode(&(final->code), mods2->code);


        //Pop the ModuleDec symbol table
        newTable = sympop(tbStack);
        // if(newTable!=NULL) free(newTable);

        //Pop the Program symbol table
        newTable = sympop(tbStack);
        // if(newTable!=NULL) free(newTable);
        return final;
    }
    else
    {
        intermed *final = (intermed*)malloc(sizeof(intermed));
        final->code = NULL;
        return final;
    }
}