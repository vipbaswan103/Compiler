#include "codegenDef.h"
#include "ast.h"
#include "symbolTableDef.h"
#include "symbolTable.h"
#include "semantics.h"

int tmpNum = 0;
int labelNum = 0;
int currentOffset = 0;
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

void getTemporary(temporary * tmp, tableStack *tbStack, int temptype)
{
    sprintf(tmp->name, "_t%d", tmpNum);
    tmpNum++;
    symbolTableNode * newNode = (symbolTableNode *)malloc(sizeof(symbolTableNode));
    
    newNode->ele.data.id.type = (char *)malloc(sizeof(char)*51);
    newNode->ele.data.id.lexeme = (char *)malloc(sizeof(char)*51);
    //Integer
    if(temptype == 0)
    {
        newNode->width = INTEGER_SIZE;
        strcpy(newNode->ele.data.id.type, "INTEGER\0");
        strcpy(tmp->type, "INTEGER\0");
    }
    //REAL
    else if(temptype==1)
    {
        newNode->width = REAL_SIZE;
        strcpy(newNode->ele.data.id.type, "REAL\0");
        strcpy(tmp->type, "REAL\0");
    }
    //BOOLEAN
    else if(temptype==2)
    {
        newNode->width = BOOLEAN_SIZE;
        strcpy(newNode->ele.data.id.type, "BOOLEAN\0");
        strcpy(tmp->type, "BOOLEAN\0");
    }
    newNode->offset = currentOffset;
    currentOffset += newNode->width;
    
    newNode->lineNum  = -1;
    newNode->aux = -1;
    newNode->next = NULL;
    newNode->ele.tag = Identifier;
    strcpy(newNode->ele.data.id.lexeme,tmp->name);
    newNode->ele.data.id.value = NULL;
    newNode->ele.data.id.isIndex = -1;
    newNode->ele.data.id.isAssigned = 1;
    newNode->isParameter = 0;
    sym_hash_insert(newNode, &(tbStack->top->ele->hashtb));
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
    {
        // printf("%s %s %s %s\n", trav->ele->result, trav->ele->arg1, trav->ele->op, trav->ele->arg2);
        trav = trav->next;
    }
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
    !strcmp(currentNode->node->ele.internalNode->label, "MINUS"))
    {
        //Binary case
        if(currentNode->child->sibling != NULL)
        {
            intermed* leftchild = generateIRCode(currentNode->child, labels, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);
            intermed* final;
            initializeFinalCode(&final);
            
            int tempType;
            if(!strcmp(leftchild->t.type,"INTEGER"))
                tempType=0;     
            else if(!strcmp(leftchild->t.type,"REAL"))
                tempType=1;
            else if(!strcmp(leftchild->t.type,"BOOLEAN"))
                tempType=2;
            
            getTemporary(&(final->t), tbStack, tempType);

            initQuad(final->code->ele, leftchild->t.name, rightchild->t.name, final->t.name);
            getOp(currentNode->node->ele.internalNode->label, final->code->ele->op);
            // verify whether the node is a leaf (NUM or RNUM)
            // ID would be in ID_ARR so nit a leaf
            if(currentNode->child->sibling->node->tag == Leaf)
            {
                if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "NUM"))
                    final->code->ele->tag2 = NUM;
                else
                    final->code->ele->tag2 = RNUM;
            }
            else
                final->code->ele->tag2 = ID;
            
            if(currentNode->child->node->tag == Leaf)
            {
                if(!strcmp(currentNode->child->node->ele.leafNode->type, "NUM"))
                    final->code->ele->tag1 = NUM;
                else
                    final->code->ele->tag1 = RNUM;
            }
            else
                final->code->ele->tag1 = ID;
                
            mergeCode(&(leftchild->code), rightchild->code);
            mergeCode(&(leftchild->code), final->code);
            final->code = leftchild->code;

            free(leftchild);
            free(rightchild);
            
            return final;
        }
        //Unary case
        else
        {
            intermed* child = generateIRCode(currentNode->child, labels, tbStack);
            intermed* final;
            initializeFinalCode(&final);

            int tempType;
            if(!strcmp(child->t.type,"INTEGER"))
                tempType=0;     
            else if(!strcmp(child->t.type,"REAL"))
                tempType=1;
            else if(!strcmp(child->t.type,"BOOLEAN"))
                tempType=2;

            getTemporary(&(final->t), tbStack, tempType);
            
            initQuad(final->code->ele, child->t.name, "\0", final->t.name);
            getOp(currentNode->node->ele.internalNode->label, final->code->ele->op);
            if(currentNode->child->node->tag == Leaf)
            {
                if(!strcmp(currentNode->child->node->ele.leafNode->type, "NUM"))
                    final->code->ele->tag1 = NUM;
                else
                    final->code->ele->tag1 = RNUM;
            }
            else
                final->code->ele->tag1 = ID;
                
            mergeCode(&(child->code), final->code);
            final->code = child->code;
                
            free(child);
            return final;
        }
    }
    if(!strcmp(currentNode->node->ele.internalNode->label, "MUL") || 
    !strcmp(currentNode->node->ele.internalNode->label, "DIV"))
    {
        intermed* leftchild = generateIRCode(currentNode->child, labels, tbStack);
        intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);
        intermed* final;
        initializeFinalCode(&final);

        int tempType;
        if(!strcmp(leftchild->t.type,"INTEGER"))
            tempType=0;     
        else if(!strcmp(leftchild->t.type,"REAL"))
            tempType=1;
        else if(!strcmp(leftchild->t.type,"BOOLEAN"))
            tempType=2;
        getTemporary(&(final->t), tbStack, tempType);
        
        initQuad(final->code->ele, leftchild->t.name, rightchild->t.name, final->t.name);
        getOp(currentNode->node->ele.internalNode->label, final->code->ele->op);

        if(currentNode->child->sibling->node->tag == Leaf)
        {
            if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "NUM"))
                final->code->ele->tag2 = NUM;
            else
                final->code->ele->tag2 = RNUM;
        }
        else
            final->code->ele->tag2 = ID;
        
        if(currentNode->child->node->tag == Leaf)
        {
            if(!strcmp(currentNode->child->node->ele.leafNode->type, "NUM"))
                final->code->ele->tag1 = NUM;
            else
                final->code->ele->tag1 = RNUM;
        }
        else
            final->code->ele->tag1 = ID;

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
            intermed* leftchild = generateIRCode(currentNode->child, NULL, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, NULL, tbStack);
            
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

            if(currentNode->child->node->tag == Leaf)
            {
                if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "NUM"))
                    ifcode->ele->tag1 = NUM;
                else
                    ifcode->ele->tag1 = RNUM;
            }
            else
                ifcode->ele->tag1 = ID;
            
            if(currentNode->child->sibling->node->tag == Leaf)
            {
                if(!strcmp(currentNode->child->node->ele.leafNode->type, "NUM"))
                    ifcode->ele->tag2 = NUM;
                else
                    ifcode->ele->tag2 = RNUM;
            }
            else
                ifcode->ele->tag2 = ID;

            //goto label number 1, for jumping when condition is true
            //hence utilising arg1 of the "lables" quad
            initQuad(goto1->ele, labels->arg1, "\0", "\0");
            strcpy(goto1->ele->op, "goto\0");
            goto1->ele->tag1 = NONE;

            //goto label number 2, for jumping when condition is false
            //hence utilising arg2 of the "labels" quad
            initQuad(goto2->ele, labels->arg2, "\0", "\0");
            strcpy(goto2->ele->op, "goto\0");
            goto2->ele->tag1 = NONE;

            //merge all the codes in order
            // E1 code - E2 code - if E1 relop  E2 - goto true - goto false 
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
            // the label has nothing, just a normal boolean expressison
            intermed* leftchild = generateIRCode(currentNode->child, labels, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);

            intermed* final;
            initializeFinalCode(&final);
            // since it is a normal expression,

            // get a temporary and it's type is boolean
            getTemporary(&(final->t), tbStack, 2);

            
            initQuad(final->code->ele, leftchild->t.name, rightchild->t.name, final->t.name);
            getOp(currentNode->node->ele.internalNode->label, final->code->ele->op);
            
            if(currentNode->child->sibling->node->tag == Leaf)
            {
                if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "NUM"))
                    final->code->ele->tag2 = NUM;
                else
                    final->code->ele->tag2 = RNUM;
            }
            else
                final->code->ele->tag2 = ID;
            
            if(currentNode->child->node->tag == Leaf)
            {
                if(!strcmp(currentNode->child->node->ele.leafNode->type, "NUM"))
                    final->code->ele->tag1 = NUM;
                else
                    final->code->ele->tag1 = RNUM;
            }
            else
                final->code->ele->tag1 = ID;

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
            //assume the clause is B1 && B2

            quad *l = (quad *)malloc(sizeof(quad));
            //get a new label for the true condition jump from B1 to B2
            getLabel(l->arg1);
            //the false condition is a short cicuit to the false of the whole clause
            strcpy(l->arg2,labels->arg2);
            l->tag1 = NONE;
            l->tag2 = NONE;
            
            //call the left and right child with respective true and false labels
            intermed* leftchild = generateIRCode(currentNode->child, l, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);
            
            //synthesize the clause itself.
            intermed* final;
            initializeFinalCode(&final);
            getTemporary(&(final->t), tbStack, 2);

            IRcode *labCode = (IRcode *)malloc(sizeof(IRcode));
            labCode->ele = (quad *)malloc(sizeof(quad));
            labCode->next = NULL;

            //label statement for B2
            strcpy(labCode->ele->arg1, l->arg1);
            strcpy(labCode->ele->op, ":\0");
            labCode->ele->tag1 = NONE;

            //code order - left side code for B1, label of B2, code for B2
            //goto statements will come with relop sides not AND, OR, NOt
            mergeCode(&(leftchild->code), labCode);
            mergeCode(&(leftchild->code), rightchild->code);
            final->code = leftchild->code;
            free(leftchild);
            free(rightchild);

            return final;
        }
        else
        {
            //RHS of assignment statement cause labels are empty
            intermed* leftchild = generateIRCode(currentNode->child, labels, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);

            //initialise  the final code to be returned
            intermed* final;
            initializeFinalCode(&final);
            getTemporary(&(final->t), tbStack, 2);
            
            getOp(currentNode->node->ele.internalNode->label, final->code->ele->op);
            strcpy(final->code->ele->arg1, leftchild->t.name);
            strcpy(final->code->ele->arg2, rightchild->t.name);
            strcpy(final->code->ele->result, final->t.name);

            if(currentNode->child->sibling->node->tag == Leaf)
                final->code->ele->tag2 = BOOL;
            else
                final->code->ele->tag2 = ID;
            
            if(currentNode->child->node->tag == Leaf)
                final->code->ele->tag1 = BOOL;
            else
                final->code->ele->tag1 = ID;
                
            final->code->next = NULL;
            
            // order left - right - AND statement to combine the two into new temporary
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
        if(labels != NULL)
        {
            //inside w while condition
            quad *l = (quad *)malloc(sizeof(quad));
            //get a new label for when B1 is false
            getLabel(l->arg2);
            // the true label is that of overall B.true
            strcpy(l->arg1,labels->arg1);
            l->tag1 = NONE;
            l->tag2 = NONE;

            //call the left and right child with respective labels
            intermed* leftchild = generateIRCode(currentNode->child, l, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);
            
            //initialize the final to be returned
            intermed* final;
            initializeFinalCode(&final);
            getTemporary(&(final->t), tbStack, 2);

            IRcode *labCode = (IRcode *)malloc(sizeof(IRcode));
            labCode->ele = (quad *)malloc(sizeof(quad));
            strcpy(labCode->ele->op, ":\0");
            strcpy(labCode->ele->arg1, l->arg2);
            labCode->ele->tag1 = NONE;

            labCode->next = NULL;
            if(currentNode->child->sibling->node->tag == Leaf)
                final->code->ele->tag2 = BOOL;
            else
                final->code->ele->tag2 = ID;
            
            if(currentNode->child->node->tag == Leaf)
                final->code->ele->tag1 = BOOL;
            else
                final->code->ele->tag1 = ID;

            //merge in order
            // left/B1 - right/B2 label - B2 code
            //goto will be in relop stages, not here at the OR,AND, NOT level
            mergeCode(&(leftchild->code), labCode);
            mergeCode(&(leftchild->code), rightchild->code);
            final->code = leftchild->code;
            free(leftchild);
            free(rightchild);

            return final;
        }
        else
        {
            //simple boolean expression
            intermed* leftchild = generateIRCode(currentNode->child, labels, tbStack);
            intermed* rightchild = generateIRCode(currentNode->child->sibling, labels, tbStack);

            intermed* final;
            initializeFinalCode(&final);
            getTemporary(&(final->t), tbStack, 2);
            
            getOp(currentNode->node->ele.internalNode->label, final->code->ele->op);
            strcpy(final->code->ele->arg1, leftchild->t.name);
            strcpy(final->code->ele->arg2, rightchild->t.name);
            strcpy(final->code->ele->result, final->t.name);

            if(currentNode->child->sibling->node->tag == Leaf)
                final->code->ele->tag2 = BOOL;
            else
                final->code->ele->tag2 = ID;
            
            if(currentNode->child->node->tag == Leaf)
                final->code->ele->tag1 = BOOL;
            else
                final->code->ele->tag1 = ID;

            final->code->next = NULL;

            // merge in order
            //left - right - OR statement code
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
        
        // arg1 holds RHS temp, result holds LHS, op holds "="
        intermed* final;
        initializeFinalCode(&final);
        strcpy(final->code->ele->op, "=\0");
        initQuad(final->code->ele,
        rightchild->t.name,
        "\0",  // when arg2 is empty , we know that it is assignop and not array assign op
        currentNode->child->node->ele.leafNode->lexeme);
        
        final->code->next = NULL;

        // set tags for arg1 and arg2
        // RHS is some constant
        if(currentNode->child->sibling->node->tag == Leaf)
        {
            if(!strcmp(rightchild->t.type, "INTEGER"))
                final->code->ele->tag1 = NUM;
            else if(!strcmp(rightchild->t.type, "REAL"))
                final->code->ele->tag1 = RNUM;
            else if(!strcmp(rightchild->t.type, "BOOLEAN"))
                final->code->ele->tag1 = BOOL;
        }
        else
        {
            final->code->ele->tag1 = ID;
        }
        
        final->code->ele->tag2 = NONE;
        
        // right side expressions code first 
        // then the statement of assignment itself
        mergeCode(&(rightchild->code), final->code);
        final->code = rightchild->code;
        free(rightchild);

        return final;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "ASSIGNOPARR"))
    {
        //Child 1 - ID
        //Child 2 - Index
        //Child 3 - RHS
        
        intermed * tmp = (intermed *)malloc(sizeof(intermed));
        tmp->code = NULL;

        symbolTableNode * node = searchScope(tbStack, currentNode->child);

        if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type, "ID") ||
        node->ele.data.arr.isDynamic == 1)
        {
            // either the index is an ID
            // or the array itself is dynamic 

            IRcode * ifcode1 = (IRcode *)malloc(sizeof(IRcode));
            ifcode1->ele = (quad *)malloc(sizeof(quad));
            ifcode1->next = NULL;
            
            // create a quad for upper index check 
            // op is less tha equal, arg1 is the index (ID or NUM), arg2 is the upperindex (ID or NUM) 
            // again result capturesthe fact that it is an if condition
            strcpy(ifcode1->ele->op, "<=\0");
            initQuad(ifcode1->ele, currentNode->child->sibling->node->ele.leafNode->lexeme,
            node->ele.data.arr.upperIndex->lexeme, "if\0");
            
            // arg1 tag is set
            if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"NUM"))
                ifcode1->ele->tag1 = NUM;
            else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"ID"))
                ifcode1->ele->tag1 = ID;
            
            // arg2 tag is set
            if(!strcmp(node->ele.data.arr.upperIndex->type,"NUM"))
                ifcode1->ele->tag2 = NUM;
            else if(!strcmp(node->ele.data.arr.upperIndex->type,"ID"))
                ifcode1->ele->tag2 = ID;
            
            // create a goto statement for true condition
            IRcode * goto1 = (IRcode *)malloc(sizeof(IRcode));
            goto1->ele = (quad *)malloc(sizeof(quad));
            goto1->next = NULL;
            
            // new label for the true condition
            strcpy(goto1->ele->op, "goto\0");
            initQuad(goto1->ele, "\0", "\0", "\0");
            getLabel(goto1->ele->arg1);

            //create a goto statement for false condition
            IRcode * goto2 = (IRcode *)malloc(sizeof(IRcode));
            goto2->ele = (quad *)malloc(sizeof(quad));
            goto2->next = NULL;
            
            //RUNTIME_Error label for the false condition
            strcpy(goto2->ele->op, "goto\0");
            initQuad(goto2->ele, "RUNTIME_ERROR\0", "\0", "\0");

            // the newly created label for true condition needs a label statement
            // which is actually the label for the lower index check
            IRcode * label1 = (IRcode *)malloc(sizeof(IRcode));
            label1->ele = (quad *)malloc(sizeof(quad));
            label1->next = NULL;
            
            strcpy(label1->ele->op, ":\0");
            initQuad(label1->ele, goto1->ele->arg1, "\0", "\0");
            

            //FOLOW THE SAME PROCEDURE AS ABOVE for lower index check
            IRcode * ifcode2 = (IRcode *)malloc(sizeof(IRcode));
            ifcode2->ele = (quad *)malloc(sizeof(quad));
            ifcode2->next = NULL;
            
            strcpy(ifcode2->ele->op, ">=\0");
            initQuad(ifcode2->ele, currentNode->child->sibling->node->ele.leafNode->lexeme,
            node->ele.data.arr.lowerIndex->lexeme, "if\0");

            // arg1 tag is set
            if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"NUM"))
                ifcode2->ele->tag1 = NUM;
            else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"ID"))
                ifcode2->ele->tag1 = ID;
            
            // arg2 tag is set
            if(!strcmp(node->ele.data.arr.lowerIndex->type,"NUM"))
                ifcode2->ele->tag2 = NUM;
            else if(!strcmp(node->ele.data.arr.lowerIndex->type,"ID"))
                ifcode2->ele->tag2 = ID;

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

            //merging order
            mergeCode(&(tmp->code), ifcode1);
            mergeCode(&(tmp->code), goto1); //true has label as label1
            mergeCode(&(tmp->code), goto2); //false
            mergeCode(&(tmp->code), label1);
            mergeCode(&(tmp->code), ifcode2); 
            mergeCode(&(tmp->code), goto3); //true has label as label2
            mergeCode(&(tmp->code), goto4); //false
            mergeCode(&(tmp->code), label2); 
        }

        //evaluate the expression in IR code
        intermed* rightchild = generateIRCode(currentNode->child->sibling->sibling, NULL, tbStack);
        intermed* final;
        initializeFinalCode(&final); 
        strcpy(final->code->ele->op, "=\0");
        
        initQuad(final->code->ele,
        rightchild->t.name,
        currentNode->child->sibling->node->ele.leafNode->lexeme,
        currentNode->child->node->ele.leafNode->lexeme); //when arg2 is not empty we know that it is assignoparr
       
        if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"NUM"))
            final->code->ele->tag2 = NUM;
        else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"ID"))
            final->code->ele->tag2 = ID;

        if(currentNode->child->sibling->sibling->node->tag == Leaf)
        {
            if(!strcmp(rightchild->t.type, "INTEGER"))
                final->code->ele->tag1 = NUM;
            else if(!strcmp(rightchild->t.type, "REAL"))
                final->code->ele->tag1 = RNUM;
            else if(!strcmp(rightchild->t.type, "BOOLEAN"))
                final->code->ele->tag1 = BOOL;
        }
        else
        {
            final->code->ele->tag1 = ID;
        }

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
            //statements to help us in final pass for identifiying when to change the scope
            IRcode *scopeStart = (IRcode*)malloc(sizeof(IRcode));
            scopeStart->ele = (quad*)malloc(sizeof(quad));
            scopeStart->next = NULL;
            strcpy(scopeStart->ele->op,"SCOPESTARTMODULE");
            initQuad(scopeStart->ele,"\0","\0","\0");

            IRcode *scopeEnd = (IRcode*)malloc(sizeof(IRcode));
            scopeEnd->ele = (quad*)malloc(sizeof(IRcode));
            scopeEnd->next = NULL;
            strcpy(scopeEnd->ele->op,"SCOPEENDMODULE");
            initQuad(scopeEnd->ele,"\0","\0","\0");

            newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
            newTable->ele = st;
            newTable->next = NULL;
            sympush(tbStack, newTable);
            st = st->sibling;
            tmp = generateIRCode(trav, NULL, tbStack);
            
            //insert the statement before and after every module in this segment
            mergeCode(&(final->code),scopeStart);
            mergeCode(&(final->code),tmp->code);
            mergeCode(&(final->code),scopeEnd);

            trav = trav->sibling;
        }
        return final;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULES2"))
    {
        //EXACTLY SAME AS THE MODULES1 ABOVE
        
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
            IRcode *scopeStart = (IRcode*)malloc(sizeof(IRcode));
            scopeStart->ele = (quad*)malloc(sizeof(quad));
            scopeStart->next = NULL;
            strcpy(scopeStart->ele->op,"SCOPESTARTMODULE");
            initQuad(scopeStart->ele,"\0","\0","\0");

            IRcode *scopeEnd = (IRcode*)malloc(sizeof(IRcode));
            scopeEnd->ele = (quad*)malloc(sizeof(IRcode));
            scopeEnd->next = NULL;
            strcpy(scopeEnd->ele->op,"SCOPEENDMODULE");
            initQuad(scopeEnd->ele,"\0","\0","\0");

            newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
            newTable->ele = st;
            newTable->next = NULL;
            sympush(tbStack, newTable);
            st = st->sibling;
            tmp = generateIRCode(trav, NULL, tbStack);

            mergeCode(&(final->code),scopeStart);
            mergeCode(&(final->code),tmp->code);
            mergeCode(&(final->code),scopeEnd);
            
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
        
        // module is one level inside (remember)
        // there is modules1 and modules2, inside which there is modules
        // inside that is moduledef 
        currentOffset = tbStack->top->ele->currentOffset;
        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->ele = tbStack->top->ele->child;
        newTable->next = NULL;

        // Push the moduleDef's symbol table on the stack
        sympush(tbStack, newTable);
        // finalDef is just a label for the module
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
        
        // body has the code from the moduledef astNode
        body = generateIRCode(currentNode->child->sibling->sibling->sibling, l, tbStack);

        // label quad for the module's last label of RET 
        // done for "what if" while was the last statement of the module
        IRcode *labelCode = (IRcode *)malloc(sizeof(IRcode));
        labelCode->ele = (quad *)malloc(sizeof(quad));
        labelCode->next = NULL;
        strcpy(labelCode->ele->op, ":\0");
        initQuad(labelCode->ele, l->arg1, "\0", "\0");

        // return statement for the module
        IRcode* ret = (IRcode *)malloc(sizeof(IRcode)); 
        ret->next=NULL;        
        ret->ele = (quad *)malloc(sizeof(quad));
        strcpy(ret->ele->op,"RET\0");
        initQuad(ret->ele, "\0","\0","\0");

        // order 
        // label for module - body - next label - ret
        mergeCode(&(finaldef), body->code);
        mergeCode(&(finaldef), labelCode);
        mergeCode(&(finaldef), ret);
        
        body->code = finaldef;

        tbStack->top->ele->currentOffset = currentOffset;
        sympop(tbStack);
        return body;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "DRIVER"))
    {  
        // find the driver in trav
        symbolTable *trav = symbolTableRoot->child->sibling;
        while(strcmp(trav->symLexeme, "Driver"))
        {
            trav = trav->sibling;
        }

        IRcode *scopeStart = (IRcode*)malloc(sizeof(IRcode));
        scopeStart->ele = (quad*)malloc(sizeof(quad));
        scopeStart->next = NULL;
        strcpy(scopeStart->ele->op,"SCOPESTARTDRIVER");
        initQuad(scopeStart->ele,"\0","\0","\0");

        IRcode *scopeEnd = (IRcode*)malloc(sizeof(IRcode));
        scopeEnd->ele = (quad*)malloc(sizeof(IRcode));
        scopeEnd->next = NULL;
        strcpy(scopeEnd->ele->op,"SCOPEENDDRIVER");
        initQuad(scopeEnd->ele,"\0","\0","\0");

        tableStackEle *newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->ele = trav;
        newTable->next = NULL;
        // Push the driver's symbol table on the stack
        sympush(tbStack, newTable);
        
        newTable = (tableStackEle *)malloc(sizeof(tableStackEle));
        newTable->ele = trav->child;
        newTable->next = NULL;
        // Push the moduleDef's symbol table on the stack
        sympush(tbStack, newTable);
        
        // name of the module is the label for it
        IRcode* finaldef = (IRcode *)malloc(sizeof(IRcode)); 
        finaldef->next=NULL;   
        finaldef->ele = (quad*)malloc(sizeof(quad));      
        strcpy(finaldef->ele->op,":\0");

        initQuad(finaldef->ele, "main", "\0" ,"\0");   

        // create a label and pass it to statements inside the body
        // to tackle a possible while loop at the end of the body
        // think of it like ret's label
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

        // RET statement
        IRcode* ret = (IRcode *)malloc(sizeof(IRcode)); 
        ret->next=NULL;        
        ret->ele = (quad *)malloc(sizeof(quad));
        strcpy(ret->ele->op,"RET\0");
        initQuad(ret->ele, "\0","\0","\0");

        // merge the code in order
        // startscope, finaldef, body, ret's label, ret, scopeend
        mergeCode(&(scopeStart), finaldef);
        mergeCode(&(scopeStart), body->code);
        mergeCode(&(scopeStart), labelCode);
        // mergeCode(&(scopeStart), ret);
        mergeCode(&(scopeStart), scopeEnd);
        
        body->code = scopeStart;
                
                
        sympop(tbStack);
        return body;
    }
    
    else if(!strcmp(currentNode->node->ele.internalNode->label, "FOR"))
    {
        // For node has 3 Children
        // ID
        // Range 
        // Statements
        
        // REMEMBER : We don't have to insert a startscope and endscaope here
        // because it was inserted at the time the for, while or switch was encountered

        // Code for ID = <lowerBound> (set the id as the lower bound)
        IRcode * assignCode = (IRcode *)malloc(sizeof(IRcode));
        assignCode->ele = (quad *)malloc(sizeof(quad));
        assignCode->next = NULL;

        strcpy(assignCode->ele->op, "=\0");
        initQuad(assignCode->ele, currentNode->child->sibling->child->node->ele.leafNode->lexeme,
        "\0", currentNode->child->node->ele.leafNode->lexeme);
        assignCode->ele->tag1 = NUM;

        // Code for goto label 1, this is the loop incrementor goto
        IRcode * goto1 = (IRcode *)malloc(sizeof(IRcode));
        goto1->ele = (quad *)malloc(sizeof(quad));
        goto1->next = NULL;
        strcpy(goto1->ele->op, "goto\0");
        initQuad(goto1->ele,"\0","\0","\0");
        getLabel(goto1->ele->arg1);
        goto1->ele->tag1 = NONE;

        // Code for if
        IRcode *ifcode = (IRcode *)malloc(sizeof(IRcode));
        ifcode->ele = (quad *)malloc(sizeof(quad));
        ifcode->next = NULL;

        strcpy(ifcode->ele->op, "<=\0");
        initQuad(ifcode->ele, currentNode->child->node->ele.leafNode->lexeme, 
        currentNode->child->sibling->child->sibling->node->ele.leafNode->lexeme,
        "if\0");
        ifcode->ele->tag1 = ID;
        ifcode->ele->tag2 = NUM;

        // Code for goto label 2, the body of the for loop
        IRcode *goto2 = (IRcode *)malloc(sizeof(IRcode));
        goto2->ele = (quad *)malloc(sizeof(quad));
        goto2->next = NULL;

        strcpy(goto2->ele->op, "goto\0");
        initQuad(goto2->ele,"\0","\0","\0");
        getLabel(goto2->ele->arg1);
        goto2->ele->tag1 = NONE;

        //Code for goto label 3, goto statement with label as what we got from parent
        //goto3 has the next statement of the whole FOR loop
        IRcode *goto3 = (IRcode *)malloc(sizeof(IRcode));
        goto3->ele = (quad *)malloc(sizeof(quad));
        goto3->next = NULL;
        
        strcpy(goto3->ele->op, "goto\0");
        initQuad(goto3->ele,labels->arg1,"\0","\0");
        goto3->ele->tag1 = NONE;

        quad* stmtsLabel = (quad *)malloc(sizeof(quad));
        strcpy(stmtsLabel->op, "\0");
        initQuad(stmtsLabel, goto1->ele->arg1, "\0", "\0");
        stmtsLabel->tag1 = NONE;

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
                IRcode *scopeStart = (IRcode*)malloc(sizeof(IRcode));
                scopeStart->ele = (quad*)malloc(sizeof(quad));
                scopeStart->next = NULL;
                strcpy(scopeStart->ele->op,"SCOPESTART");
                initQuad(scopeStart->ele,"\0","\0","\0");
                scopeStart->ele->tag1 = NONE;

                IRcode *scopeEnd = (IRcode*)malloc(sizeof(IRcode));
                scopeEnd->ele = (quad*)malloc(sizeof(IRcode));
                scopeEnd->next = NULL;
                strcpy(scopeEnd->ele->op,"SCOPEEND");
                initQuad(scopeEnd->ele,"\0","\0","\0");
                scopeEnd->ele->tag1 = NONE;

                newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
                newNode->ele = st;
                newNode->next = NULL;
                sympush(tbStack, newNode);
                st = st->sibling;
                
                mergeCode(&(stmtsCode->code), scopeStart);
                
                //If there is some stmt after this stmt
                if(trav->sibling != NULL)
                {
                    // get a new label for the stmtsLabel
                    getLabel(stmtsLabel->arg1);
                    stmtCode = generateIRCode(trav, stmtsLabel, tbStack);

                    IRcode * labelCode = (IRcode *)malloc(sizeof(IRcode));
                    labelCode->ele = (quad *)malloc(sizeof(quad));
                    labelCode->next = NULL;
                    strcpy(labelCode->ele->op, ":\0");
                    initQuad(labelCode->ele, stmtsLabel->arg1, "\0", "\0");
                    labelCode->ele->tag1 = NONE;

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
                
                mergeCode(&(stmtsCode->code), scopeEnd);
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
        label1->ele->tag1 = NONE;

        //Label 2
        IRcode *label2 = (IRcode *)malloc(sizeof(IRcode));
        label2->ele = (quad *)malloc(sizeof(quad));
        label2->next = NULL;

        strcpy(label2->ele->op, ":\0");
        initQuad(label2->ele, goto2->ele->arg1, "\0", "\0");
        label2->ele->tag1 = NONE;

        //Increment ID
        IRcode *incr = (IRcode *)malloc(sizeof(IRcode));
        incr->ele = (quad *)malloc(sizeof(quad));
        incr->next = NULL;

        strcpy(incr->ele->op, "+\0");
        initQuad(incr->ele, currentNode->child->node->ele.leafNode->lexeme, "1\0",
        currentNode->child->node->ele.leafNode->lexeme);
        incr->ele->tag1 = ID;
        incr->ele->tag2 = NUM;

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
        l->tag1 = NONE;
        l->tag2 = NONE;

        IRcode *beginCode = (IRcode *)malloc(sizeof(IRcode));
        beginCode->ele = begin;
        beginCode->next = NULL;
        
        quad* stmtsLabel = (quad *)malloc(sizeof(quad));
        strcpy(stmtsLabel->op, "\0");
        initQuad(stmtsLabel, begin->arg1, "\0", "\0");
        stmtsLabel->tag1 = NONE;

        IRcode *goto1 = (IRcode *)malloc(sizeof(IRcode));
        goto1->ele = (quad *)malloc(sizeof(quad));
        strcpy(goto1->ele->op, "goto\0");
        initQuad(goto1->ele, begin->arg1, "\0", "\0");
        goto1->ele->tag1 = NONE;
        
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
                IRcode *scopeStart = (IRcode*)malloc(sizeof(IRcode));
                scopeStart->ele = (quad*)malloc(sizeof(quad));
                scopeStart->next = NULL;
                strcpy(scopeStart->ele->op,"SCOPESTART");
                initQuad(scopeStart->ele,"\0","\0","\0");
                scopeStart->ele->tag1 = NONE;
                scopeStart->ele->tag2 = NONE;

                IRcode *scopeEnd = (IRcode*)malloc(sizeof(IRcode));
                scopeEnd->ele = (quad*)malloc(sizeof(IRcode));
                scopeEnd->next = NULL;
                strcpy(scopeEnd->ele->op,"SCOPEEND");
                initQuad(scopeEnd->ele,"\0","\0","\0");
                scopeEnd->ele->tag1 = NONE;
                scopeEnd->ele->tag2 = NONE;

                newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
                newNode->ele = st;
                newNode->next = NULL;
                sympush(tbStack, newNode);
                st = st->sibling;
                
                // remember that it is the parent node's duty to enumerate the 
                // children with labels if they (any child) need the label of 
                // the right sibling

                mergeCode(&(stmtsCode->code), scopeStart);
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
                    labelCode->ele->tag1 = NONE;

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
                mergeCode(&(stmtsCode->code), scopeEnd);
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
        trueLabel->ele->tag1 = NONE;

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
                defaultLabel->tag1 = NONE;

                IRcode *gotoDefault = (IRcode *)malloc(sizeof(IRcode));
                gotoDefault->ele = (quad *)malloc(sizeof(quad));
                gotoDefault->next = NULL;
                strcpy(gotoDefault->ele->op, "goto\0");
                initQuad(gotoDefault->ele, defaultLabel->arg2, "\0", "\0");
                gotoDefault->ele->tag1 = NONE;

                mergeCode(&(cummulator->code), gotoDefault);

                caseStmts[len] = generateIRCode(trav, defaultLabel, tbStack);                       
            }
            else
            {
                quad* caseLabel = (quad *)malloc(sizeof(quad));
                getLabel(caseLabel->arg2);
                strcpy(caseLabel->arg1, labels->arg1);
                caseLabel->tag1 = NONE;

                IRcode *ifcode = (IRcode *)malloc(sizeof(IRcode));
                ifcode->ele = (quad *)malloc(sizeof(quad));
                ifcode->next = NULL;

                strcpy(ifcode->ele->op, "==\0");
                initQuad(ifcode->ele, idname, trav->child->node->ele.leafNode->lexeme,"if\0");
                ifcode->ele->tag1 = ID;

                if(!strcmp(trav->child->node->ele.leafNode->type, "NUM"))
                    ifcode->ele->tag2 = NUM;
                //Though RNUM isn't possible in case value (for semantically correct code)
                else if(!strcmp(trav->child->node->ele.leafNode->type, "RNUM"))
                    ifcode->ele->tag2 = RNUM;
                else if(!strcmp(trav->child->node->ele.leafNode->type, "TRUE") 
                    || !strcmp(trav->child->node->ele.leafNode->type, "FALSE"))
                    ifcode->ele->tag2 = BOOL;

                IRcode *gotoCase = (IRcode *)malloc(sizeof(IRcode));
                gotoCase->ele = (quad *)malloc(sizeof(quad));
                gotoCase->next = NULL;
                strcpy(gotoCase->ele->op, "goto\0");
                initQuad(gotoCase->ele, caseLabel->arg2, "\0", "\0");
                gotoCase->ele->tag1=NONE;
                gotoCase->ele->tag2=NONE;

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
        stmtsLabel->tag1 = NONE;

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
                IRcode *scopeStart = (IRcode*)malloc(sizeof(IRcode));
                scopeStart->ele = (quad*)malloc(sizeof(quad));
                scopeStart->next = NULL;
                strcpy(scopeStart->ele->op,"SCOPESTART");
                initQuad(scopeStart->ele,"\0","\0","\0");
                scopeStart->ele->tag1=NONE;
                scopeStart->ele->tag2=NONE;
                
                IRcode *scopeEnd = (IRcode*)malloc(sizeof(IRcode));
                scopeEnd->ele = (quad*)malloc(sizeof(IRcode));
                scopeEnd->next = NULL;
                strcpy(scopeEnd->ele->op,"SCOPEEND");
                initQuad(scopeEnd->ele,"\0","\0","\0");
                scopeEnd->ele->tag1 = NONE;
                scopeEnd->ele->tag2 = NONE;

                newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
                newNode->ele = st;
                newNode->next = NULL;
                sympush(tbStack, newNode);
                st = st->sibling;
                // remember that it is the parent node's duty to enumerate the 
                // children with labels if they (any child) need the label of 
                // the right sibling

                mergeCode(&(stmtsCode->code), scopeStart);
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
                    labelCode->ele->tag1 = NONE;

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
                mergeCode(&(stmtsCode->code), scopeEnd);
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
        labelCode->ele->tag1 = NONE;
        
        //Code for goto next
        IRcode *gotoNext = (IRcode *)malloc(sizeof(IRcode));
        gotoNext->ele = (quad *)malloc(sizeof(quad));
        gotoNext->next = NULL;
        strcpy(gotoNext->ele->op, "goto\0");
        initQuad(gotoNext->ele, labels->arg1,"\0", "\0");
        gotoNext->ele->tag1=NONE;
        gotoNext->ele->tag2=NONE;
        
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
        
        currentOffset = tbStack->top->ele->currentOffset;

        quad* stmtsLabel = (quad *)malloc(sizeof(quad));
        strcpy(stmtsLabel->op, "\0");
        initQuad(stmtsLabel, labels->arg1, "\0", "\0");
        stmtsLabel->tag1 = NONE;

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
                IRcode *scopeStart = (IRcode*)malloc(sizeof(IRcode));
                scopeStart->ele = (quad*)malloc(sizeof(quad));
                scopeStart->next = NULL;
                strcpy(scopeStart->ele->op,"SCOPESTART");
                initQuad(scopeStart->ele,"\0","\0","\0");
                scopeStart->ele->tag1 = NONE;
                scopeStart->ele->tag2 = NONE;
                
                IRcode *scopeEnd = (IRcode*)malloc(sizeof(IRcode));
                scopeEnd->ele = (quad*)malloc(sizeof(IRcode));
                scopeEnd->next = NULL;
                strcpy(scopeEnd->ele->op,"SCOPEEND");
                initQuad(scopeEnd->ele,"\0","\0","\0");
                scopeEnd->ele->tag1=NONE;
                scopeEnd->ele->tag2=NONE;

                newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
                newNode->ele = st;
                newNode->next = NULL;
                sympush(tbStack, newNode);
                st = st->sibling;

                // remember that it is the parent node's duty to enumerate the 
                // children with labels if they (any child) need the label of 
                // the right sibling

                mergeCode(&(stmtsCode->code), scopeStart);
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
                    labelCode->ele->tag1 = NONE;

                    mergeCode(&(stmtsCode->code), stmtCode->code);
                    mergeCode(&(stmtsCode->code), labelCode);
                }
                // If this is the last stmt
                else
                {
                    strcpy(stmtsLabel->arg1, labels->arg1);
                    stmtCode = generateIRCode(trav, stmtsLabel, tbStack);
                    mergeCode(&(stmtsCode->code), stmtCode->code);

                    // Now, there isn't any need for the "goto begin" code
                    isEnd = 1;
                }
                mergeCode(&(stmtsCode->code), scopeEnd);
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
        tbStack->top->ele->currentOffset = currentOffset;
        sympop(tbStack);
        return final;
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "PRINT"))
    {
        intermed * final = NULL;
        //print(<NUM/RNUM/TRUE/FALSE>)
        if(currentNode->child->node->tag == Leaf)
        {
            // this is a BOOL constant  or NUM or RUM

            final = (intermed *)malloc(sizeof(intermed));
            final->code = NULL;
                
            IRcode *print = (IRcode *)malloc(sizeof(IRcode));
            print->ele = (quad *)malloc(sizeof(quad));
            print->next = NULL;

            strcpy(print->ele->op, "printf\0");
            initQuad(print->ele, currentNode->child->node->ele.leafNode->lexeme, "\0","\0");

            if(!strcmp(currentNode->child->node->ele.leafNode->type,"NUM"))
                print->ele->tag1 = NUM;
            else if(!strcmp(currentNode->child->node->ele.leafNode->type,"RNUM"))
                print->ele->tag1 = RNUM;
            else if(!strcmp(currentNode->child->node->ele.leafNode->type,"TRUE") 
                || !strcmp(currentNode->child->node->ele.leafNode->type,"FALSE") )
                print->ele->tag1 = BOOL;

            print->ele->tag2 = NONE;
            mergeCode(&(final->code), print);
            return final;
        }
        else if(currentNode->child->child->sibling == NULL)
        {
            intermed * final = NULL;
            symbolTableNode * node = searchScope(tbStack, currentNode->child->child);
            // Case 1 : ID is an Array
            if(node->ele.tag == Array)
            {
                // we are assigning the temporary as the lowerbound of the range
                // then we print the element and then incremenet the value of the 
                // index, then again jump to the conditional check and break out of the loop
                // in case the condition is not satisfied

                final = (intermed *)malloc(sizeof(intermed));
                final->code = NULL;
                
                IRcode * assignCode = (IRcode *)malloc(sizeof(IRcode));
                assignCode->ele = (quad *)malloc(sizeof(quad));
                assignCode->next = NULL;
                
                strcpy(assignCode->ele->op, "=\0");
                initQuad(assignCode->ele, node->ele.data.arr.lowerIndex->lexeme,
                "\0", "\0");

                if(!strcmp(node->ele.data.arr.lowerIndex->type, "NUM"))
                    assignCode->ele->tag1 = NUM;
                else
                    assignCode->ele->tag1 = ID;

                assignCode->ele->tag2 = NONE;
                temporary tmp;
                getTemporary(&tmp, tbStack, 0);
                strcpy(assignCode->ele->result, tmp.name);
                
                //Code for goto label 1
                IRcode * goto1 = (IRcode *)malloc(sizeof(IRcode));
                goto1->ele = (quad *)malloc(sizeof(quad));
                goto1->next = NULL;
                strcpy(goto1->ele->op, "goto\0");
                getLabel(goto1->ele->arg1);
                strcpy(goto1->ele->arg2, "\0");
                strcpy(goto1->ele->result, "\0");
                goto1->ele->tag1 = NONE;
                goto1->ele->tag2 = NONE;

                //Code for if
                IRcode *ifcode = (IRcode *)malloc(sizeof(IRcode));
                ifcode->ele = (quad *)malloc(sizeof(quad));
                ifcode->next = NULL;

                strcpy(ifcode->ele->op, "<=\0");
                initQuad(ifcode->ele, assignCode->ele->result, 
                node->ele.data.arr.upperIndex->lexeme,
                "if\0");

                if(!strcmp(node->ele.data.arr.upperIndex->type, "NUM"))
                    ifcode->ele->tag2 = NUM;
                else
                    ifcode->ele->tag2 = ID;    
                ifcode->ele->tag1 = ID;

                //Code for goto label 2
                IRcode *goto2 = (IRcode *)malloc(sizeof(IRcode));
                goto2->ele = (quad *)malloc(sizeof(quad));
                goto2->next = NULL;

                strcpy(goto2->ele->op, "goto\0");
                getLabel(goto2->ele->arg1);
                strcpy(goto2->ele->arg2, "\0");
                strcpy(goto2->ele->result, "\0");
                goto2->ele->tag1 = NONE;
                goto2->ele->tag2 = NONE;

                //Code for goto lable 3
                IRcode *goto3 = (IRcode *)malloc(sizeof(IRcode));
                goto3->ele = (quad *)malloc(sizeof(quad));
                goto3->next = NULL;

                strcpy(goto3->ele->op, "goto\0");
                getLabel(goto3->ele->arg1);
                strcpy(goto3->ele->arg2, "\0");
                strcpy(goto3->ele->result, "\0");

                goto3->ele->tag1 = NONE;
                goto3->ele->tag2 = NONE;

                //Code for label 1
                IRcode * label1 = (IRcode *)malloc(sizeof(IRcode));
                label1->ele = (quad *)malloc(sizeof(quad));
                label1->next = NULL;
                label1->ele->tag1 = NONE;
                
                strcpy(label1->ele->op, ":\0");
                initQuad(label1->ele, goto1->ele->arg1, "\0", "\0");

                //Code for lable 2
                IRcode * label2 = (IRcode *)malloc(sizeof(IRcode));
                label2->ele = (quad *)malloc(sizeof(quad));
                label2->next = NULL;
                label2->ele->tag1 = NONE;
                
                strcpy(label2->ele->op, ":\0");
                initQuad(label2->ele, goto2->ele->arg1, "\0", "\0");
                label2->ele->tag2 = NONE;
                
                //Code for label 3
                IRcode * label3 = (IRcode *)malloc(sizeof(IRcode));
                label3->ele = (quad *)malloc(sizeof(quad));
                label3->next = NULL;
                label3->ele->tag1=NONE;
                
                strcpy(label3->ele->op, ":\0");
                initQuad(label3->ele, goto3->ele->arg1, "\0", "\0");

                //_t0 := A[_t1], code for assignment of the element from temporary we just took
                IRcode * assignEle = (IRcode *)malloc(sizeof(IRcode));
                assignEle->ele = (quad *)malloc(sizeof(quad));
                assignEle->next = NULL;

                strcpy(assignEle->ele->op, "=\0");
                initQuad(assignEle->ele, currentNode->child->child->node->ele.leafNode->lexeme, 
                assignCode->ele->result, "\0");

                temporary tmp2;
                int tempType;
                if(!strcmp(node->ele.data.arr.type,"INTEGER"))
                    tempType = 0;
                else if (!strcmp(node->ele.data.arr.type,"REAL"))
                    tempType = 1;
                else
                    tempType = 2;
                    
                getTemporary(&tmp2, tbStack, tempType);
                strcpy(assignEle->ele->result, tmp2.name);
                
                assignEle->ele->tag1 = ID;
                assignEle->ele->tag2 = ID;
                
                //Code for
                IRcode *pr = (IRcode *)malloc(sizeof(IRcode));
                pr->ele = (quad *)malloc(sizeof(quad));
                pr->next = NULL;

                strcpy(pr->ele->op, "print_output\0");
                
                //Code for printf, print(_t0)
                IRcode *print = (IRcode *)malloc(sizeof(IRcode));
                print->ele = (quad *)malloc(sizeof(quad));
                print->next = NULL;

                strcpy(print->ele->op, "printf_array\0");
                initQuad(print->ele, assignEle->ele->result, "\0","\0");
                print->ele->tag1 = ID;
                print->ele->tag2 = NONE;

                //Code for
                IRcode *pr_end = (IRcode *)malloc(sizeof(IRcode));
                pr_end->ele = (quad *)malloc(sizeof(quad));
                pr_end->next = NULL;

                strcpy(pr_end->ele->op, "printf_output_end\0");

                //Code for incrementation
                IRcode *incr = (IRcode *)malloc(sizeof(IRcode));
                incr->ele = (quad *)malloc(sizeof(quad));
                incr->next = NULL;

                strcpy(incr->ele->op, "+\0");
                initQuad(incr->ele, assignCode->ele->result, "1\0",
                assignCode->ele->result);
                incr->ele->tag1 = ID;
                incr->ele->tag2 = NUM;
            
                mergeCode(&(final->code), pr);
                mergeCode(&(final->code), assignCode);
                mergeCode(&(final->code), label1);
                mergeCode(&(final->code), ifcode);
                mergeCode(&(final->code), goto2);
                mergeCode(&(final->code), goto3);
                mergeCode(&(final->code), label2);
                mergeCode(&(final->code), assignEle);
                mergeCode(&(final->code), print);
                mergeCode(&(final->code), incr);
                mergeCode(&(final->code), goto1);
                mergeCode(&(final->code), label3);
                mergeCode(&(final->code), pr_end);
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
                final->code->ele->tag1 = ID;
                final->code->ele->tag2 = NONE;
                
                return final;
            }
        }

        //printing an array element
        else
        {
            intermed * final = NULL;
            final->code = NULL;

            intermed *child = generateIRCode(currentNode->child, labels, tbStack);

            IRcode * print  = (IRcode *)malloc(sizeof(IRcode));
            print->ele = (quad *)malloc(sizeof(quad));
            print->next = NULL;
            strcpy(print->ele->op, "printf\0");
            initQuad(print->ele, child->t.name, "\0", "\0");
            print->ele->tag1 = ID;
            print->ele->tag2 = NONE;
                       
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

            temporary tmp2;
            getTemporary(&tmp2, tbStack, 0);
            strcpy(assignCode->ele->result, tmp2.name);

            if(!strcmp(node->ele.data.arr.lowerIndex->type, "NUM"))
                assignCode->ele->tag1 = NUM;
            else
                assignCode->ele->tag1 = ID;
            assignCode->ele->tag2 = NONE;

            //Code for goto label 1
            IRcode * goto1 = (IRcode *)malloc(sizeof(IRcode));
            goto1->ele = (quad *)malloc(sizeof(quad));
            goto1->next = NULL;
            strcpy(goto1->ele->op, "goto\0");
            getLabel(goto1->ele->arg1);
            strcpy(goto1->ele->arg2, "\0");
            strcpy(goto1->ele->result, "\0");
            goto1->ele->tag1 = NONE;

            //Code for if
            IRcode *ifcode = (IRcode *)malloc(sizeof(IRcode));
            ifcode->ele = (quad *)malloc(sizeof(quad));
            ifcode->next = NULL;

            strcpy(ifcode->ele->op, "<=\0");
            initQuad(ifcode->ele, assignCode->ele->result, 
            node->ele.data.arr.upperIndex->lexeme,
            "if\0");

            ifcode->ele->tag1 = ID;
            if(!strcmp(node->ele.data.arr.upperIndex->type, "NUM"))
                ifcode->ele->tag2 = NUM;
            else
                ifcode->ele->tag2 = ID;

            //Code for goto label 2
            IRcode *goto2 = (IRcode *)malloc(sizeof(IRcode));
            goto2->ele = (quad *)malloc(sizeof(quad));
            goto2->next = NULL;

            strcpy(goto2->ele->op, "goto\0");
            getLabel(goto2->ele->arg1);
            strcpy(goto2->ele->arg2, "\0");
            strcpy(goto2->ele->result, "\0");
            goto2->ele->tag1 = NONE;
            goto2->ele->tag2 = NONE;

            //Code for goto lable 3
            IRcode *goto3 = (IRcode *)malloc(sizeof(IRcode));
            goto3->ele = (quad *)malloc(sizeof(quad));
            goto3->next = NULL;

            strcpy(goto3->ele->op, "goto\0");
            getLabel(goto3->ele->arg1);
            strcpy(goto3->ele->arg2, "\0");
            strcpy(goto3->ele->result, "\0");
            goto3->ele->tag1 = NONE;
            goto3->ele->tag2 = NONE;

            //Code for label 1
            IRcode * label1 = (IRcode *)malloc(sizeof(IRcode));
            label1->ele = (quad *)malloc(sizeof(quad));
            label1->next = NULL;
            
            strcpy(label1->ele->op, ":\0");
            initQuad(label1->ele, goto1->ele->arg1, "\0", "\0");
            label1->ele->tag1 = NONE;

            //Code for lable 2
            IRcode * label2 = (IRcode *)malloc(sizeof(IRcode));
            label2->ele = (quad *)malloc(sizeof(quad));
            label2->next = NULL;
            
            strcpy(label2->ele->op, ":\0");
            initQuad(label2->ele, goto2->ele->arg1, "\0", "\0");
            label2->ele->tag1 = NONE;

            //Code for label 3
            IRcode * label3 = (IRcode *)malloc(sizeof(IRcode));
            label3->ele = (quad *)malloc(sizeof(quad));
            label3->next = NULL;
            
            strcpy(label3->ele->op, ":\0");
            initQuad(label3->ele, goto3->ele->arg1, "\0", "\0");
            label3->ele->tag1 = NONE;

            //Code for incrementation
            IRcode *incr = (IRcode *)malloc(sizeof(IRcode));
            incr->ele = (quad *)malloc(sizeof(quad));
            incr->next = NULL;

            strcpy(incr->ele->op, "+\0");
            initQuad(incr->ele, assignCode->ele->result, "1\0",
            assignCode->ele->result);
            incr->ele->tag1 = ID;
            incr->ele->tag2 = NUM;
            
            //Code for scanf start
            IRcode *sc = (IRcode *)malloc(sizeof(IRcode));
            sc->ele = (quad *)malloc(sizeof(quad));
            sc->next = NULL;
            strcpy(sc->ele->op, "scanf_output\0");
            strcpy(sc->ele->arg1, currentNode->child->node->ele.leafNode->lexeme);

            //Code for scanf end
            IRcode *sc_end = (IRcode *)malloc(sizeof(IRcode));
            sc_end->ele = (quad *)malloc(sizeof(quad));
            sc_end->next = NULL;
            strcpy(sc_end->ele->op, "scanf_output_end\0");

            //Code for scanf
            IRcode *scan = (IRcode *)malloc(sizeof(IRcode));
            scan->ele = (quad *)malloc(sizeof(quad));
            scan->next = NULL;
            
            int tempType;
            if(!strcmp(node->ele.data.arr.type,"INTEGER"))
                tempType = 0;
            else if (!strcmp(node->ele.data.arr.type,"REAL"))
                tempType = 1;
            else
                tempType = 2;

            temporary tmp;
            getTemporary(&tmp, tbStack, tempType);
            initQuad(scan->ele, tmp.name, "\0", "\0");
            strcpy(scan->ele->op, "scanf_array\0");

            scan->ele->tag1 = ID;
            scan->ele->tag2 = NONE;

            //A[_t1] := _t0 , code for assignment of the element from temporary we just took
            IRcode * assignEle = (IRcode *)malloc(sizeof(IRcode));
            assignEle->ele = (quad *)malloc(sizeof(quad));
            assignEle->next = NULL;

            strcpy(assignEle->ele->op, "=\0");
            initQuad(assignEle->ele, scan->ele->arg1, 
            assignCode->ele->result, currentNode->child->node->ele.leafNode->lexeme);
                
            assignEle->ele->tag1 = ID;
            assignEle->ele->tag2 = ID;

            mergeCode(&(final->code), sc);
            mergeCode(&(final->code), assignCode); // assign the lower index
            mergeCode(&(final->code), label1); // start of the condition/loop
            mergeCode(&(final->code), ifcode); // check the condition
            mergeCode(&(final->code), goto2); // true condition (main body of the loop)
            mergeCode(&(final->code), goto3); // false condition
            mergeCode(&(final->code), label2); // main body of the loop's label
            mergeCode(&(final->code), scan);
            mergeCode(&(final->code), assignEle);
            mergeCode(&(final->code), incr);
            mergeCode(&(final->code), goto1);
            mergeCode(&(final->code), label3);
            mergeCode(&(final->code), sc_end);
            return final;
        }
        else
        {
            //Case when ID isn't an arrayID
            initializeFinalCode(&final); 
            strcpy(final->code->ele->op, "scanf\0");
            initQuad(final->code->ele, currentNode->child->node->ele.leafNode->lexeme, "\0", "\0");
            final->code->ele->tag1 = ID;
            final->code->ele->tag2 = NONE;
            return final;
        }
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "ID_ARR"))
    {
        //2 (max) children;
            //id_node
            //which_node

        // Checking if ID_ARR is just ID or  Array Type without an index
        if(currentNode->child->sibling == NULL)
        {
            // Not a Boolean comparison
            if(labels == NULL)
            {
                intermed *final = (intermed*)malloc(sizeof(intermed));
                final->code = NULL;
                symbolTableNode *type1 = searchScope(tbStack, currentNode->child);;
                strcpy(final->t.name, currentNode->child->node->ele.leafNode->lexeme);
                strcpy(final->t.type, type1->ele.data.id.type);
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
                ifcode->ele->tag1 = ID;
                ifcode->ele->tag2 = BOOL;
                
                IRcode *gotoTrueCase = (IRcode *)malloc(sizeof(IRcode));
                gotoTrueCase->ele = (quad *)malloc(sizeof(quad));
                gotoTrueCase->next = NULL;
                strcpy(gotoTrueCase->ele->op, "goto\0");
                initQuad(gotoTrueCase->ele, labels->arg1, "\0", "\0");
                gotoTrueCase->ele->tag1 = NONE;
                gotoTrueCase->ele->tag2 = NONE;
                
                
                IRcode *gotoFalseCase = (IRcode *)malloc(sizeof(IRcode));
                gotoFalseCase->ele = (quad *)malloc(sizeof(quad));
                gotoFalseCase->next = NULL;
                strcpy(gotoFalseCase->ele->op, "goto\0");
                initQuad(gotoFalseCase->ele, labels->arg2, "\0", "\0");
                gotoFalseCase->ele->tag1 = NONE;
                gotoFalseCase->ele->tag2 = NONE;
                                
                mergeCode(&(final->code), ifcode);
                mergeCode(&(final->code), gotoTrueCase);
                mergeCode(&(final->code), gotoFalseCase);
                return final;
            }
        }
        // ID_ARR is an Array element
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

                //set the type of the index, to help the final code generation of these conditions 
                if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"NUM"))
                    ifcode1->ele->tag1 = NUM;
                else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"ID"))
                    ifcode1->ele->tag1 = ID;

                if(!strcmp(node->ele.data.arr.upperIndex->type,"NUM"))
                    ifcode1->ele->tag2 = NUM;
                else if(!strcmp(node->ele.data.arr.upperIndex->type,"ID"))
                    ifcode1->ele->tag2 = ID;
                

                IRcode * goto1 = (IRcode *)malloc(sizeof(IRcode));
                goto1->ele = (quad *)malloc(sizeof(quad));
                goto1->next = NULL;
                
                strcpy(goto1->ele->op, "goto\0");
                initQuad(goto1->ele, "\0", "\0", "\0");
                getLabel(goto1->ele->arg1);

                goto1->ele->tag1=NONE;
                goto1->ele->tag2=NONE;

                IRcode * goto2 = (IRcode *)malloc(sizeof(IRcode));
                goto2->ele = (quad *)malloc(sizeof(quad));
                goto2->next = NULL;
                
                strcpy(goto2->ele->op, "goto\0");
                initQuad(goto2->ele, "RUNTIME_ERROR\0", "\0", "\0");

                goto2->ele->tag1=NONE;
                goto2->ele->tag2=NONE;

                IRcode * label1 = (IRcode *)malloc(sizeof(IRcode));
                label1->ele = (quad *)malloc(sizeof(quad));
                label1->next = NULL;
                
                strcpy(label1->ele->op, ":\0");
                initQuad(label1->ele, goto1->ele->arg1, "\0", "\0");

                label1->ele->tag1=NONE;
                label1->ele->tag2=NONE;

                IRcode * ifcode2 = (IRcode *)malloc(sizeof(IRcode));
                ifcode2->ele = (quad *)malloc(sizeof(quad));
                ifcode2->next = NULL;
                
                strcpy(ifcode2->ele->op, ">=\0");
                initQuad(ifcode2->ele, currentNode->child->sibling->node->ele.leafNode->lexeme,
                node->ele.data.arr.lowerIndex->lexeme, "if\0");

                if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"NUM"))
                    ifcode2->ele->tag1 = NUM;
                else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"ID"))
                    ifcode2->ele->tag1 = ID; 
                
                if(!strcmp(node->ele.data.arr.lowerIndex->type,"NUM"))
                    ifcode2->ele->tag2 = NUM;
                else if(!strcmp(node->ele.data.arr.lowerIndex->type,"ID"))
                    ifcode2->ele->tag2 = ID;


                IRcode * goto3 = (IRcode *)malloc(sizeof(IRcode));
                goto3->ele = (quad *)malloc(sizeof(quad));
                goto3->next = NULL;
                
                strcpy(goto3->ele->op, "goto\0");
                initQuad(goto3->ele, "\0", "\0", "\0");
                getLabel(goto3->ele->arg1);

                goto3->ele->tag1=NONE;
                goto3->ele->tag2=NONE;

                IRcode * goto4 = (IRcode *)malloc(sizeof(IRcode));
                goto4->ele = (quad *)malloc(sizeof(quad));
                goto4->next = NULL;
                
                strcpy(goto4->ele->op, "goto\0");
                initQuad(goto4->ele, "RUNTIME_ERROR\0", "\0", "\0");

                goto4->ele->tag1=NONE;
                goto4->ele->tag2=NONE;

                IRcode * label2 = (IRcode *)malloc(sizeof(IRcode));
                label2->ele = (quad *)malloc(sizeof(quad));
                label2->next = NULL;
                
                strcpy(label2->ele->op, ":\0");
                initQuad(label2->ele, goto3->ele->arg1, "\0", "\0");
                label2->ele->tag1=NONE;
                label2->ele->tag2=NONE;

                mergeCode(&(tmp->code), ifcode1);
                mergeCode(&(tmp->code), goto1);
                mergeCode(&(tmp->code), goto2);
                mergeCode(&(tmp->code), label1);
                mergeCode(&(tmp->code), ifcode2);
                mergeCode(&(tmp->code), goto3);
                mergeCode(&(tmp->code), goto4);
                mergeCode(&(tmp->code), label2);
            }
            intermed *final;
            if(labels != NULL)
            {
                // Case 1: Array element is a Part of while() condition
                final = (intermed*)malloc(sizeof(intermed));
                final->code = NULL;
                
                // create a new tempoary and assign it the array eleement
                IRcode * assign  = (IRcode *)malloc(sizeof(IRcode));
                assign->ele = (quad *)malloc(sizeof(quad));
                assign->next = NULL;
                strcpy(assign->ele->op, "=");

                int tempType;
                if(!strcmp(node->ele.data.arr.type,"INTEGER"))
                    tempType = 0;
                else if (!strcmp(node->ele.data.arr.type,"REAL"))
                    tempType = 1;
                else
                    tempType = 2;
                
                getTemporary(&(final->t), tbStack, tempType);
                strcpy(assign->ele->arg1, currentNode->child->node->ele.leafNode->lexeme); // array name
                strcpy(assign->ele->arg2, currentNode->child->sibling->node->ele.leafNode->lexeme);
                strcpy(assign->ele->result, final->t.name);

                assign->ele->tag1 = ID; // cause its an array name

                if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"ID"))
                    assign->ele->tag2 = ID;
                else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"NUM"))
                    assign->ele->tag2 = NUM;

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

                int tempType;
                if(!strcmp(node->ele.data.arr.type,"INTEGER"))
                    tempType = 0;
                else if (!strcmp(node->ele.data.arr.type,"REAL"))
                    tempType = 1;
                else
                    tempType = 2;

                getTemporary(&(final->t), tbStack, tempType);
                strcpy(final->code->ele->arg1, currentNode->child->node->ele.leafNode->lexeme);
                strcpy(final->code->ele->arg2, currentNode->child->sibling->node->ele.leafNode->lexeme);
                strcpy(final->code->ele->result, final->t.name);

                final->code->ele->tag1=  ID;
                
                if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"ID"))
                    final->code->ele->tag2 = ID;
                else if(!strcmp(currentNode->child->sibling->node->ele.leafNode->type,"NUM"))
                    final->code->ele->tag2 = NUM;

                tmp->t = final->t;
            }
            mergeCode(&(tmp->code), final->code);
            return tmp;
        }
    }
    else if(!strcmp(currentNode->node->ele.internalNode->label, "MODULECALL"))
    {
        astNode * trav = NULL;
        intermed * final = (intermed *)malloc(sizeof(intermed));
        final->code = NULL;

        int paramCount = 0;

        trav = currentNode->child->sibling->sibling->child;
        IRcode * inp_populate = NULL;
        while(trav != NULL)
        {
            symbolTableNode * ret = searchScope(tbStack, trav);

            if(ret->ele.tag == Array)
            {
                IRcode * upperIndex = (IRcode *)malloc(sizeof(IRcode));
                upperIndex->ele = (quad *)malloc(sizeof(quad));
                upperIndex->next = NULL;
                strcpy(upperIndex->ele->op, "param\0");
                initQuad(upperIndex->ele, ret->ele.data.arr.upperIndex->lexeme, "\0", "\0");

                mergeCode(&upperIndex, final->code);
                final->code = upperIndex;

                IRcode * lowerIndex = (IRcode *)malloc(sizeof(IRcode));
                lowerIndex->ele = (quad *)malloc(sizeof(quad));
                lowerIndex->next = NULL;
                strcpy(lowerIndex->ele->op, "param\0");
                initQuad(lowerIndex->ele, ret->ele.data.arr.lowerIndex->lexeme, "\0", "\0");

                mergeCode(&lowerIndex, final->code);
                final->code = lowerIndex;

                IRcode * param1 = (IRcode *)malloc(sizeof(IRcode));
                param1->ele = (quad *)malloc(sizeof(quad));
                param1->next = NULL;
                strcpy(param1->ele->op, "param\0");
                initQuad(param1->ele, trav->node->ele.leafNode->lexeme, "\0", "\0");

                mergeCode(&param1, final->code);
                final->code = param1;

                IRcode * param2 = (IRcode *)malloc(sizeof(IRcode));
                param2->ele = (quad *)malloc(sizeof(quad));
                param2->next = NULL;
                
                strcpy(param2->ele->op, "inp\0");
                initQuad(param2->ele, trav->node->ele.leafNode->lexeme, "\0", "\0");
                mergeCode(&(inp_populate), param2);

                IRcode * lowerIndex2 = (IRcode *)malloc(sizeof(IRcode));
                lowerIndex2->ele = (quad *)malloc(sizeof(quad));
                lowerIndex2->next = NULL;
                strcpy(lowerIndex2->ele->op, "inp\0");
                initQuad(lowerIndex2->ele, ret->ele.data.arr.lowerIndex->lexeme, "\0", "\0");

                mergeCode(&(inp_populate), lowerIndex2);

                IRcode * upperIndex2 = (IRcode *)malloc(sizeof(IRcode));
                upperIndex2->ele = (quad *)malloc(sizeof(quad));
                upperIndex2->next = NULL;
                strcpy(upperIndex2->ele->op, "inp\0");
                initQuad(upperIndex2->ele, ret->ele.data.arr.upperIndex->lexeme, "\0", "\0");

                mergeCode(&(inp_populate), upperIndex2);
            }
            else
            {
                IRcode * param = (IRcode *)malloc(sizeof(IRcode));
                param->ele = (quad *)malloc(sizeof(quad));
                param->next = NULL;
                strcpy(param->ele->op, "param\0");
                initQuad(param->ele, trav->node->ele.leafNode->lexeme, "\0", "\0");

                mergeCode(&param, final->code);
                final->code = param;

                IRcode * param2 = (IRcode *)malloc(sizeof(IRcode));
                param2->ele = (quad *)malloc(sizeof(quad));
                param2->next = NULL;
                
                strcpy(param2->ele->op, "inp\0");
                initQuad(param2->ele, trav->node->ele.leafNode->lexeme, "\0", "\0");
                mergeCode(&(inp_populate), param2);
            }
            paramCount++;
            trav = trav->sibling;
        }

        IRcode * out_populate = NULL;
        // set trav to be id_list for input to the function
        trav = currentNode->child->child;
        while(trav != NULL)
        {
            symbolTableNode * ret = searchScope(tbStack, trav);

            if(ret->ele.tag == Array)
            {
                IRcode * upperIndex = (IRcode *)malloc(sizeof(IRcode));
                upperIndex->ele = (quad *)malloc(sizeof(quad));
                upperIndex->next = NULL;
                strcpy(upperIndex->ele->op, "param\0");
                initQuad(upperIndex->ele, ret->ele.data.arr.upperIndex->lexeme, "\0", "\0");

                mergeCode(&upperIndex, final->code);
                final->code = upperIndex;

                IRcode * lowerIndex = (IRcode *)malloc(sizeof(IRcode));
                lowerIndex->ele = (quad *)malloc(sizeof(quad));
                lowerIndex->next = NULL;
                strcpy(lowerIndex->ele->op, "param\0");
                initQuad(lowerIndex->ele, ret->ele.data.arr.lowerIndex->lexeme, "\0", "\0");

                mergeCode(&lowerIndex, final->code);
                final->code = lowerIndex;

                IRcode * param1 = (IRcode *)malloc(sizeof(IRcode));
                param1->ele = (quad *)malloc(sizeof(quad));
                param1->next = NULL;
                strcpy(param1->ele->op, "param\0");
                initQuad(param1->ele, trav->node->ele.leafNode->lexeme, "\0", "\0");

                mergeCode(&param1, final->code);
                final->code = param1;

                IRcode * param2 = (IRcode *)malloc(sizeof(IRcode));
                param2->ele = (quad *)malloc(sizeof(quad));
                param2->next = NULL;
                
                strcpy(param2->ele->op, "inp\0");
                initQuad(param2->ele, trav->node->ele.leafNode->lexeme, "\0", "\0");
                mergeCode(&(out_populate), param2);

                IRcode * lowerIndex2 = (IRcode *)malloc(sizeof(IRcode));
                lowerIndex2->ele = (quad *)malloc(sizeof(quad));
                lowerIndex2->next = NULL;
                strcpy(lowerIndex2->ele->op, "inp\0");
                initQuad(lowerIndex2->ele, ret->ele.data.arr.lowerIndex->lexeme, "\0", "\0");

                mergeCode(&(out_populate), lowerIndex2);

                IRcode * upperIndex2 = (IRcode *)malloc(sizeof(IRcode));
                upperIndex2->ele = (quad *)malloc(sizeof(quad));
                upperIndex2->next = NULL;
                strcpy(upperIndex2->ele->op, "inp\0");
                initQuad(upperIndex2->ele, ret->ele.data.arr.upperIndex->lexeme, "\0", "\0");

                mergeCode(&(out_populate), upperIndex2);
            }
            else
            {
                IRcode * param = (IRcode *)malloc(sizeof(IRcode));
                param->ele = (quad *)malloc(sizeof(quad));
                param->next = NULL;
                
                strcpy(param->ele->op, "param\0");
                initQuad(param->ele, trav->node->ele.leafNode->lexeme, "\0", "\0");

                // reverse order of param
                mergeCode(&param, final->code);
                final->code = param;

                IRcode * param2 = (IRcode *)malloc(sizeof(IRcode));
                param2->ele = (quad *)malloc(sizeof(quad));
                param2->next = NULL;
                
                strcpy(param2->ele->op, "out\0");
                initQuad(param2->ele, trav->node->ele.leafNode->lexeme, "\0", "\0");
                
                mergeCode(&(out_populate), param2);   
            }
            paramCount++;
            trav = trav->sibling;
        }
        
        IRcode * trigger = (IRcode *)malloc(sizeof(IRcode));
        trigger->ele = (quad *)malloc(sizeof(quad));
        trigger->next = NULL;
        strcpy(trigger->ele->op, "trigger\0");
        // mergeCode(&(final->code), tmpCode);

        IRcode * call = (IRcode *)malloc(sizeof(IRcode));
        call->ele = (quad *)malloc(sizeof(quad));
        call->next = NULL;
        strcpy(call->ele->op, "call\0");
        initQuad(call->ele, currentNode->child->sibling->node->ele.leafNode->lexeme, "\0", "\0");
        sprintf(call->ele->arg2, "%d", paramCount);
        mergeCode(&(final->code), call);
        mergeCode(&(final->code), trigger);
        mergeCode(&(final->code), inp_populate);
        mergeCode(&(final->code), out_populate);
        return final;
    }

    // now we deal with constants
    else if((currentNode->node->tag == Leaf))
    {
        
        if(!strcmp(currentNode->node->ele.leafNode->type, "NUM") 
        || !strcmp(currentNode->node->ele.leafNode->type, "RNUM"))
        {
            // arithmetic constants
            intermed * final = (intermed *)malloc(sizeof(intermed));
            final->code = NULL;
            strcpy(final->t.name, currentNode->node->ele.leafNode->lexeme);
            if(!strcmp(currentNode->node->ele.leafNode->type, "NUM"))
                strcpy(final->t.type,"INTEGER");
            else if(!strcmp(currentNode->node->ele.leafNode->type, "RNUM"))
                strcpy(final->t.type,"REAL");
            return final;
        }
        else
        {
            // boolean constants
            if(labels == NULL)
            {
                // occuring in normal expressions
                intermed * final = (intermed *)malloc(sizeof(intermed));
                final->code = NULL;
                strcpy(final->t.name, currentNode->node->ele.leafNode->lexeme);
                strcpy(final->t.type,"BOOLEAN");
                return final;
            }
            else
            {
                // while like expressions
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

                goto1->ele->tag1=NONE;
                goto1->ele->tag2=NONE;
                
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
    else if(!strcmp(currentNode->node->ele.internalNode->label, "DECLARE"))
    {
        intermed *final = (intermed*)malloc(sizeof(intermed));
        final->code=NULL;
        if(!strcmp(currentNode->child->sibling->node->ele.internalNode->label,"ARRAY"))
        {
            //head of idList
            astNode* temp = currentNode->child->child;
            
            while(temp!=NULL)
            {
                IRcode * dec = malloc(sizeof(IRcode));
                dec->ele = (quad *)malloc(sizeof(quad));
                dec->next=NULL;
                strcpy(dec->ele->op,"declare");
                initQuad(dec->ele, "\0", "\0", "\0");
                strcpy(dec->ele->arg1, temp->node->ele.leafNode->lexeme);
                mergeCode(&(final->code),dec);
                temp = temp->sibling;
            }
        }
        return final;
    }
    else
    {
        intermed *final = (intermed*)malloc(sizeof(intermed));
        final->code = NULL;
        return final;
    }
}