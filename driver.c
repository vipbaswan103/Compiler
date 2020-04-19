/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/


#include "parser.h"
#include "lexer.h"
#include "ast.h"
#include "symbolTable.h"
#include <time.h>
#include "semantics.h"
#include "codegen.h"
#include "nasmcode.h"






void seeTokenization()
{
    Token * tkn = NULL;

    tkn = getNextToken();
    int *x;
    float *y;
    
    printf(" LINE NUMBER \t\t LEXEME \t\t TOKEN \t\t\t VALUE \n"); 
    printf("----------------------------------------------------------------------------------------------\n");
    
    while(strcmp(tkn->token, "EOF")!=0)
    {
        if(strcmp(tkn->token, "NUM") == 0)
        {
            x = (int *)tkn->value;
            printf(" %-20d  |  %-20s  |  %-20s  |  %-20d\n", tkn->lineNum, tkn->lexeme, tkn->token, *x);
        }
        else if(strcmp(tkn->token, "RNUM") == 0)
        {
            y = (float *)tkn->value;
            printf(" %-20d  |  %-20s  |  %-20s  |  %-20lf\n", tkn->lineNum, tkn->lexeme, tkn->token, *y);
        }
        else
            printf(" %-20d  |  %-20s  |  %-20s  |  %-20s\n", tkn->lineNum, tkn->lexeme, tkn->token,"----");
        tkn = getNextToken();
    }

    printf("%s\n", tkn->token);
}


int parsememorycount(TreeNode * p)
{
    int sum = 0;
    
    if(p==NULL)
    return sum;
    
    TreeNode * temp = p->child;

    while(temp!=NULL)
    {
        sum = sum + parsememorycount(temp);
        temp=temp->sibling;
    }    
    sum = sum + sizeof(TreeNode);
    return sum;
}

int parsecount(TreeNode * p)
{
    int sum = 0;
    
    if(p==NULL)
    return sum;
    
    TreeNode * temp = p->child;
    sum = sum + 1;
    while(temp!=NULL)
    {
        sum = sum + parsecount(temp);
        temp=temp->sibling;
    }    
    
    return sum;
}

int astmemorycount(astNode * a)
{
    int sum = 0;
    
    if(a==NULL)
    return sum;
    
    astNode * temp = a->child;

    while(temp!=NULL)
    {
        sum = sum + astmemorycount(temp);
        temp=temp->sibling;
    }    
    sum = sum + sizeof(astNode) + sizeof(astEle);
    
    if(a->node->tag == Leaf)
        sum = sum + sizeof(leaf);
    else 
        sum = sum + sizeof(internal);
    
    return sum;
}

int astcount(astNode * a)
{
    int sum = 0;
    
    if(a==NULL)
    return sum;
    
    astNode * temp = a->child;

    while(temp!=NULL)
    {
        sum = sum + astcount(temp);
        temp=temp->sibling;
    }    
    sum = sum + 1;
    return sum;
}

void printArrays(symbolTable *root)
{
    if(root == NULL)
        return;

    // printf("***********************************************************************************************************************************\n");
    
    int index = 0;

    hashSym hashtb = root->hashtb;
    
    while(index<=hashtb.hashtbSize)
    {        
        linkedListSym* trav = &(hashtb.arr[index]);
        symbolTableNode *temp = trav->head;
        while(temp!=NULL)
        {
            
            if(temp->isParameter == 2 || temp->ele.tag==Module || temp->ele.tag==Identifier)
            {
                temp = temp->next;
                
                continue;
            }

            char scopeline[20];
            char static_dynamic[20];
            char range[20];
            
        
            if(temp->isParameter==1 || temp->isParameter==2)
                sprintf(scopeline,"%d to %d",root->child->lineNumStart, root->child->lineNumEnd);
            else 
                sprintf(scopeline,"%d to %d",root->lineNumStart, root->lineNumEnd);

            if(temp->ele.data.arr.isDynamic==0)
                strcpy(static_dynamic," Static ");
            else 
                strcpy(static_dynamic," Dynamic ");

            sprintf(range,"[%s - %s]", temp->ele.data.arr.lowerIndex->lexeme, temp->ele.data.arr.upperIndex->lexeme);
            

            printf("|| %-20s || %-10s || %-20s || %-20s || %-20s || %-10s ||\n", 
            root->symLexeme, scopeline,temp->ele.data.arr.lexeme, static_dynamic, range, temp->ele.data.arr.type);

            temp = temp->next;
        }
        index++;
    }
    
    
    // printf("***********************************************************************************************************************************\n\n");
    symbolTable * tmp = root->child;
    while(tmp != NULL)
    {
        printArrays(tmp);
        tmp = tmp->sibling;
    }
}



/*******************************************************************/
int main(int argc, char * argv[])
{
    if(argc != 3)
    {
        printf("Wrong number of args. Please give 2 file names. \n The first one should be the program and the other should be where you want to write the ASM code\n"); 
        return 0;
        
    }

    initializeKeyHash();

    printf("----------------------------------------------------------------------------------------------\n");
    printf("Hello! Welcome to Compiler CSF363. This is the compiler of group 33. \n");
    printf("We are Akshit Khanna, Aryan Mehra, Vipin Baswan and Swadesh Vaibhav.\n");
    printf("Our group has successfully COMPLETED ALL REQUIRED FEATURES \n");
    printf("----------------------------------------------------------------------------------------------\n\n");
    printf("       LEVEL 4 \n");
    
    printf("----------------------------------------------------------------------------------------------\n");
    
    int option=(-1);


    printf("----------------------------------------------------------------------------------------------\n");
    printf("\nOptions:\n");
    printf("Press 0 : EXIT \n");
    printf("Press 1 : LEXER: All tokens printed \n");
    printf("Press 2 : PARSER: Inorder traversal of ParseTree \n");
    printf("Press 3 : AST: Preorder Traversal of AST \n");
    printf("Press 4 : MEMORY: Compression Ratio Analysis\n");
    printf("Press 5 : SYMBOL TABLE\n");
    printf("Press 6 : Activation Record Size\n");
    printf("Press 7 : Static and Dynamic Arrays\n");
    printf("Press 8 : Error Reporting and total compile time\n");
    printf("Press 9 : Code Generation\n");   
    printf("What would you like to do? Option: ");
    scanf("%d",&option);
    printf("----------------------------------------------------------------------------------------------\n");
    
    //For time analysis of stage 1
    clock_t start_time, end_time;               
    double total_CPU_time, total_CPU_time_in_seconds;
    
    while(option!=0)
    {

        Grammar * grammar = NULL;
        int ** parseTable = NULL;
        TreeNode * parseTree = NULL;
        int ** firstSet = NULL;
        int ** followSet = NULL;
        FILE * fp;
        astNode* ast = NULL;
        intermed * ircode = NULL;
        FILE * fpx=NULL;
        symbolTable* symT = NULL;
        tableStackEle* newNode = NULL;
        tableStack* tbStack = NULL;

        switch(option)
        {
            case 1: //LEXER FINAL
            		printf("----------------------------------------------------------------------------------------------\n");
                    populate_keyhash();
                    initializeLexer(argv[1]);
                    seeTokenization();
                    printf("----------------------------------------------------------------------------------------------\n");
                    if(LexHead != NULL)
                    {
                        printf("Lexical Errors:\n");
                        printErrorList(1);
                    }
                    else
                    {
                        printf("No lexical errors\n");
                    }
                    printf("----------------------------------------------------------------------------------------------\n");
                    
                    break;

            case 2: // PARSER FINAL 
                    
                    populate_keyhash();
                    initializeLexer(argv[1]);
                    initializeParser();

                    grammar = read_grammar("grammar.txt");
                    map(grammar);

                    firstSet = initializeFirst();
                    followSet = initializeFollow();
                    calculateFirstEquations(grammar, firstSet, firstEquations);
                    calculateFirstSet(grammar, firstSet, firstEquations);

                    calculateFollowEquations(grammar, followSet, firstSet, followEquations);
                    calculateFollowSet(grammar, followSet, followEquations);
                    
                    parseTable = intializeParseTable();
                    createParseTable(grammar,parseTable,firstSet,followSet);
                    parseTree = parser(grammar, parseTable);
                    printf("|| %10s || %10s || %15s || %10s || %20s || %8s || %20s || \n", "LEXEME", "LINENO", "TOKEN", "VALUE", "PARENT_NODE", "IS_LEAF", "CURR_NODE");
                    printf("\n%s\n","--------------------------------------------------------------------------------------------------------------------------------");
                    inOrder(parseTree, NULL);
                    printf("%s\n","--------------------------------------------------------------------------------------------------------------------------------");
                    printf("%s\n","--------------------------------------------------------------------------------------------------------------------------------");
                    printf("\n\n");

                    if(LexHead != NULL || SynHead!=NULL)
                    {
                        printf("Syntactical Errors (lexical and syntax related):\n");
                        printErrorList(1);
                        printErrorList(2);
                        printf("----------------------------------------------------------------------------------------------\n");
                        printf("\nOptions:\n");
                        printf("Press 0 : Exit \n");
                        printf("Press 1 : LEXER \n");
                        printf("Press 2 : PARSER \n");
                        printf("Press 3 : AST PREORDER TRAVERSAL\n");
                        printf("Press 4 : Memory \n");
                        printf("Press 5 : Symbol Table\n");
                        printf("Press 6 : Activation Record Size\n");
                        printf("Press 7 : Static and Dynamic Arrays\n");
                        printf("Press 8 : Error Reporting and total compile time\n");
                        printf("Press 9 : Code Generation\n");   
                        printf("What would you like to do? Option: ");
                        scanf("%d",&option);
                        printf("----------------------------------------------------------------------------------------------\n");
                        continue;
                    }
                    break;
                    
            case 3: // AST print
                    populate_keyhash();
                    initializeLexer(argv[1]);
                    initializeParser();
                    
                    grammar = read_grammar("grammar.txt");
                    map(grammar);

                    firstSet = initializeFirst();
                    followSet = initializeFollow();
                    calculateFirstEquations(grammar, firstSet, firstEquations);
                    calculateFirstSet(grammar, firstSet, firstEquations);

                    calculateFollowEquations(grammar, followSet, firstSet, followEquations);
                    calculateFollowSet(grammar, followSet, followEquations);
                    
                    parseTable = intializeParseTable();
                    createParseTable(grammar,parseTable,firstSet,followSet);
                    parseTree = parser(grammar, parseTable);
                    
                    if(LexHead != NULL || SynHead!=NULL)
                    {
                        printf("Syntactical Errors (lexical and syntax related):\n");
                        printErrorList(1);
                        printErrorList(2);
                        printf("----------------------------------------------------------------------------------------------\n");
                        printf("\nOptions:\n");
                        printf("Press 0 : Exit \n");
                        printf("Press 1 : LEXER \n");
                        printf("Press 2 : PARSER \n");
                        printf("Press 3 : AST PREORDER TRAVERSAL\n");
                        printf("Press 4 : Memory \n");
                        printf("Press 5 : Symbol Table\n");
                        printf("Press 6 : Activation Record Size\n");
                        printf("Press 7 : Static and Dynamic Arrays\n");
                        printf("Press 8 : Error Reporting and total compile time\n");
                        printf("Press 9 : Code Generation\n");   
                        printf("What would you like to do? Option: ");
                        scanf("%d",&option);
                        printf("----------------------------------------------------------------------------------------------\n");
                        continue;
                    }

                    ast = createAST(parseTree, NULL, NULL);
                    
                    printf("The traversal is PREORDER.\n");
                    printf("|| %20s || %10s || %10s || %10s || %10s || \n", "LABEL/LEXEME", "NODE", "LINENO", "TYPE", "VALUE");
                    printf("%s\n","--------------------------------------------------------------------------------------------------------------------------------");
                    printAST(ast);
                    printf("%s\n","--------------------------------------------------------------------------------------------------------------------------------");
                    printf("%s\n","--------------------------------------------------------------------------------------------------------------------------------");
                    printf("\n\n");
                    
                    break;


            case 4: // MEMORY ANALYSIS 
                    
                    populate_keyhash();
                    initializeLexer(argv[1]);
                    initializeParser();

                    grammar = read_grammar("grammar.txt");
                    map(grammar);

                    firstSet = initializeFirst();
                    followSet = initializeFollow();
                    calculateFirstEquations(grammar, firstSet, firstEquations);
                    calculateFirstSet(grammar, firstSet, firstEquations);

                    calculateFollowEquations(grammar, followSet, firstSet, followEquations);
                    calculateFollowSet(grammar, followSet, followEquations);
                    
                    parseTable = intializeParseTable();
                    createParseTable(grammar,parseTable,firstSet,followSet);
                    parseTree = parser(grammar, parseTable);

                    if(LexHead != NULL || SynHead!=NULL)
                    {
                        printf("Syntactical Errors (lexical and syntax related):\n");
                        printErrorList(1);
                        printErrorList(2);
                        printf("----------------------------------------------------------------------------------------------\n");
                        printf("\nOptions:\n");
                        printf("Press 0 : Exit \n");
                        printf("Press 1 : LEXER \n");
                        printf("Press 2 : PARSER \n");
                        printf("Press 3 : AST PREORDER TRAVERSAL\n");
                        printf("Press 4 : Memory \n");
                        printf("Press 5 : Symbol Table\n");
                        printf("Press 6 : Activation Record Size\n");
                        printf("Press 7 : Static and Dynamic Arrays\n");
                        printf("Press 8 : Error Reporting and total compile time\n");
                        printf("Press 9 : Code Generation\n");   
                        printf("What would you like to do? Option: ");
                        scanf("%d",&option);
                        printf("----------------------------------------------------------------------------------------------\n");
                        continue;
                    }
                    
                    ast = createAST(parseTree, NULL, NULL);
            
                    int pm = parsememorycount(parseTree);
                    int pc = parsecount(parseTree);
                    int am = astmemorycount(ast);
                    int ac = astcount(ast);
                    float compression_ratio = (((float)pm-am)/pm)*100;

                    printf("Parse Tree number of allocated nodes are: %d \n", pc);
                    printf("Parse Tree allocated memory: %d \n", pm);
                    printf("AST number of allocated nodes are: %d \n", ac);
                    printf("AST allocated memory: %d \n", am);
                    printf("Compression ratio: %f \n", compression_ratio);
                    
                    break;

            case 5: //Symbol Table Printing
                    
                    populate_keyhash();
                    initializeLexer(argv[1]);
                    initializeParser();
                    
                    grammar = read_grammar("grammar.txt");
                    map(grammar);

                    firstSet = initializeFirst();
                    followSet = initializeFollow();
                    calculateFirstEquations(grammar, firstSet, firstEquations);
                    calculateFirstSet(grammar, firstSet, firstEquations);

                    calculateFollowEquations(grammar, followSet, firstSet, followEquations);
                    calculateFollowSet(grammar, followSet, followEquations);
                    
                    parseTable = intializeParseTable();
                    createParseTable(grammar,parseTable,firstSet,followSet);
                    parseTree = parser(grammar, parseTable);
                    
                    if(LexHead != NULL || SynHead!=NULL)
                    {
                        printf("Syntactical Errors (lexical and syntax related):\n");
                        printErrorList(1);
                        printErrorList(2);
                        printf("----------------------------------------------------------------------------------------------\n");
                        printf("\nOptions:\n");
                        printf("Press 0 : Exit \n");
                        printf("Press 1 : LEXER \n");
                        printf("Press 2 : PARSER \n");
                        printf("Press 3 : AST PREORDER TRAVERSAL\n");
                        printf("Press 4 : Memory \n");
                        printf("Press 5 : Symbol Table\n");
                        printf("Press 6 : Activation Record Size\n");
                        printf("Press 7 : Static and Dynamic Arrays\n");
                        printf("Press 8 : Error Reporting and total compile time\n");
                        printf("Press 9 : Code Generation\n");   
                        printf("What would you like to do? Option: ");
                        scanf("%d",&option);
                        printf("----------------------------------------------------------------------------------------------\n");
                        continue;
                    }
                    
                    ast = createAST(parseTree, NULL, NULL);
                    
                    initializeErrorList();
                    formulation(ast, NULL);
                    printf("|| %-20s | %-20s | %-15s | %-8s | %-8s | %-15s | %-15s | %-10s | %-8s | %-10s ||\n", 
                    "Variable", "Scope","Scope Lines", "Width", "Is Array", "Static/Dynamic", "Range", "Type", "Offset", "Nest Level");
                    printf("-----------------------------------------------------------------------------------------------------------------------------------\n");
    
                    printSymbolTable(symbolTableRoot,-1);
                    
                    break;
            

            case 6: //Activation Record Size
                    
                    populate_keyhash();
                    initializeLexer(argv[1]);
                    initializeParser();
                    
                    grammar = read_grammar("grammar.txt");
                    map(grammar);

                    firstSet = initializeFirst();
                    followSet = initializeFollow();
                    calculateFirstEquations(grammar, firstSet, firstEquations);
                    calculateFirstSet(grammar, firstSet, firstEquations);

                    calculateFollowEquations(grammar, followSet, firstSet, followEquations);
                    calculateFollowSet(grammar, followSet, followEquations);
                    
                    parseTable = intializeParseTable();
                    createParseTable(grammar,parseTable,firstSet,followSet);
                    parseTree = parser(grammar, parseTable);
                    
                    if(LexHead != NULL || SynHead!=NULL)
                    {
                        printf("Syntactical Errors (lexical and syntax related):\n");
                        printErrorList(1);
                        printErrorList(2);
                        printf("----------------------------------------------------------------------------------------------\n");
                        printf("\nOptions:\n");
                        printf("Press 0 : Exit \n");
                        printf("Press 1 : LEXER \n");
                        printf("Press 2 : PARSER \n");
                        printf("Press 3 : AST PREORDER TRAVERSAL\n");
                        printf("Press 4 : Memory \n");
                        printf("Press 5 : Symbol Table\n");
                        printf("Press 6 : Activation Record Size\n");
                        printf("Press 7 : Static and Dynamic Arrays\n");
                        printf("Press 8 : Error Reporting and total compile time\n");
                        printf("Press 9 : Code Generation\n");   
                        printf("What would you like to do? Option: ");
                        scanf("%d",&option);
                        printf("----------------------------------------------------------------------------------------------\n");
                        continue;
                    }
                    
                    ast = createAST(parseTree, NULL, NULL);
                    initializeErrorList();
                    formulation(ast, NULL);
                    activationRecordSize(symbolTableRoot,0);

                    break;

            case 7: //Array Printing
                    
                    populate_keyhash();
                    initializeLexer(argv[1]);
                    initializeParser();
                    
                    grammar = read_grammar("grammar.txt");
                    map(grammar);

                    firstSet = initializeFirst();
                    followSet = initializeFollow();
                    calculateFirstEquations(grammar, firstSet, firstEquations);
                    calculateFirstSet(grammar, firstSet, firstEquations);

                    calculateFollowEquations(grammar, followSet, firstSet, followEquations);
                    calculateFollowSet(grammar, followSet, followEquations);
                    
                    parseTable = intializeParseTable();
                    createParseTable(grammar,parseTable,firstSet,followSet);
                    parseTree = parser(grammar, parseTable);
                    
                    if(LexHead != NULL || SynHead!=NULL)
                    {
                        printf("Syntactical Errors (lexical and syntax related):\n");
                        printErrorList(1);
                        printErrorList(2);
                        printf("----------------------------------------------------------------------------------------------\n");
                        printf("\nOptions:\n");
                        printf("Press 0 : Exit \n");
                        printf("Press 1 : LEXER \n");
                        printf("Press 2 : PARSER \n");
                        printf("Press 3 : AST PREORDER TRAVERSAL\n");
                        printf("Press 4 : Memory \n");
                        printf("Press 5 : Symbol Table\n");
                        printf("Press 6 : Activation Record Size\n");
                        printf("Press 7 : Static and Dynamic Arrays\n");
                        printf("Press 8 : Error Reporting and total compile time\n");
                        printf("Press 9 : Code Generation\n");   
                        printf("What would you like to do? Option: ");
                        scanf("%d",&option);
                        printf("----------------------------------------------------------------------------------------------\n");
                        continue;
                    }
                    
                    ast = createAST(parseTree, NULL, NULL);
                    initializeErrorList();
                    formulation(ast, NULL);
                    
                    printf("|| %-20s || %-10s || %-20s || %-20s || %-20s || %-10s ||\n",
                    "Scope", "Scope Lines", "Variable", "Static/Dynamic","Range", "Type");
                    printf("---------------------------------------------------------------------------------------------------------------------------------------\n");
                    printArrays(symbolTableRoot);

                    break;

            

            case 8: 
                    //Errors and Time Analysis // Time Analysis
                    start_time= clock();
                    //invoke lexer and parser here
                    populate_keyhash();
                    initializeLexer(argv[1]);
                    initializeParser();
                    grammar = read_grammar("grammar.txt");
                    map(grammar);

                    firstSet = initializeFirst();
                    followSet = initializeFollow();
                    calculateFirstEquations(grammar, firstSet, firstEquations);
                    calculateFirstSet(grammar, firstSet, firstEquations);

                    calculateFollowEquations(grammar, followSet, firstSet, followEquations);
                    calculateFollowSet(grammar, followSet, followEquations);

                    parseTable = intializeParseTable();
                    createParseTable(grammar,parseTable,firstSet,followSet);
                    parseTree = parser(grammar, parseTable);

                    if(LexHead != NULL || SynHead!=NULL)
                    {
                        printf("Syntactical Errors (lexical and syntax related):\n");
                        printErrorList(1);
                        printErrorList(2);
                        printf("----------------------------------------------------------------------------------------------\n");
                        printf("\nOptions:\n");
                        printf("Press 0 : Exit \n");
                        printf("Press 1 : LEXER \n");
                        printf("Press 2 : PARSER \n");
                        printf("Press 3 : AST PREORDER TRAVERSAL\n");
                        printf("Press 4 : Memory \n");
                        printf("Press 5 : Symbol Table\n");
                        printf("Press 6 : Activation Record Size\n");
                        printf("Press 7 : Static and Dynamic Arrays\n");
                        printf("Press 8 : Error Reporting and total compile time\n");
                        printf("Press 9 : Code Generation\n");   
                        printf("What would you like to do? Option: ");
                        scanf("%d",&option);
                        printf("----------------------------------------------------------------------------------------------\n");
                        continue;
                    }
                    
                    ast = createAST(parseTree, NULL, NULL);
                    initializeErrorList();
                    formulation(ast, NULL);

                    tbStack= (tableStack*)malloc(sizeof(tableStack));
                    tbStack->top = NULL;
                    tbStack->size = 0;
                    tbStack->bottom = NULL;
                    typeChecker(ast, tbStack);

                    if(semErrorList->head!=NULL) 
                    {
                        printf("\nWe found %d Semantic Errors:\n", semErrorList->numErrors);
                        printSemanticErrors();
                        printf("----------------------------------------------------------------------------------------------\n");
                        printf("\nOptions:\n");
                        printf("Press 0 : Exit \n");
                        printf("Press 1 : LEXER \n");
                        printf("Press 2 : PARSER \n");
                        printf("Press 3 : AST PREORDER TRAVERSAL\n");
                        printf("Press 4 : Memory \n");
                        printf("Press 5 : Symbol Table\n");
                        printf("Press 6 : Activation Record Size\n");
                        printf("Press 7 : Static and Dynamic Arrays\n");
                        printf("Press 8 : Error Reporting and total compile time\n");
                        printf("Press 9 : Code Generation\n");   
                        printf("What would you like to do? Option: ");
                        scanf("%d",&option);
                        printf("----------------------------------------------------------------------------------------------\n");
                        continue;
                    }

                    tbStack->top = NULL; tbStack->size = 0; tbStack->bottom = NULL;
                    ircode = generateIRCode(ast, NULL, tbStack);
                    tbStack->top = NULL; tbStack->size = 0; tbStack->bottom = NULL;
                    newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
                    newNode->ele = symbolTableRoot;
                    newNode->next = NULL;
                    sympush(tbStack, newNode);
                    symT = symbolTableRoot;
                    fpx = fopen(argv[2], "w");
                    pre_process(fpx);
                    nasmRecur(ircode->code, tbStack, symT, fpx);

                    end_time= clock();
                    total_CPU_time= (end_time-start_time);
                    
                    //time we get is in milliseconds
                    total_CPU_time_in_seconds = total_CPU_time/CLOCKS_PER_SEC;
                    
                    printf("Total CPU Time taken by integrated compiler : %lf \n", total_CPU_time);
                    printf("Total CPU Time for integrated compilation in seconds: %lf \n", total_CPU_time_in_seconds);

                    break;
            
            case 9: //Code Generation
                    
                    populate_keyhash();
                    initializeLexer(argv[1]);
                    initializeParser();
                    
                    grammar = read_grammar("grammar.txt");
                    map(grammar);

                    firstSet = initializeFirst();
                    followSet = initializeFollow();
                    calculateFirstEquations(grammar, firstSet, firstEquations);
                    calculateFirstSet(grammar, firstSet, firstEquations);

                    calculateFollowEquations(grammar, followSet, firstSet, followEquations);
                    calculateFollowSet(grammar, followSet, followEquations);
                    
                    parseTable = intializeParseTable();
                    createParseTable(grammar,parseTable,firstSet,followSet);
                    parseTree = parser(grammar, parseTable);
                    

                    if(LexHead != NULL || SynHead!=NULL)
                    {
                        printf("Syntactic Errors (lexical and syntax related):\n");
                        printErrorList(1);
                        printErrorList(2);

                        printf("\nNOTE: Since there are lexical/syntactic errors, no code generation or semantic checks take place.\n");
                        printf("----------------------------------------------------------------------------------------------\n");
                        printf("\nOptions:\n");
                        printf("Press 0 : Exit \n");
                        printf("Press 1 : LEXER \n");
                        printf("Press 2 : PARSER \n");
                        printf("Press 3 : AST PREORDER TRAVERSAL\n");
                        printf("Press 4 : Memory \n");
                        printf("Press 5 : Symbol Table\n");
                        printf("Press 6 : Activation Record Size\n");
                        printf("Press 7 : Static and Dynamic Arrays\n");
                        printf("Press 8 : Error Reporting and total compile time\n");
                        printf("Press 9 : Code Generation\n");   
                        printf("What would you like to do? Option: ");
                        scanf("%d",&option);
                        printf("----------------------------------------------------------------------------------------------\n");
                        continue;
                    }
                    
                    ast = createAST(parseTree, NULL, NULL);
                    initializeErrorList();
                    formulation(ast, NULL);
                    
                    tbStack= (tableStack*)malloc(sizeof(tableStack));
                    tbStack->top = NULL;
                    tbStack->size = 0;
                    tbStack->bottom = NULL;
                    typeChecker(ast, tbStack);
                    
                    if(semErrorList->head!=NULL) 
                    {
                        printf("\nWe found %d Semantic Errors:\n", semErrorList->numErrors);
                        printSemanticErrors();
                        printf("\nNOTE: Since there are semantic errors, no code generation takes place.\n");
                        printf("----------------------------------------------------------------------------------------------\n");
                        printf("\nOptions:\n");
                        printf("Press 0 : Exit \n");
                        printf("Press 1 : LEXER \n");
                        printf("Press 2 : PARSER \n");
                        printf("Press 3 : AST PREORDER TRAVERSAL\n");
                        printf("Press 4 : Memory \n");
                        printf("Press 5 : Symbol Table\n");
                        printf("Press 6 : Activation Record Size\n");
                        printf("Press 7 : Static and Dynamic Arrays\n");
                        printf("Press 8 : Error Reporting and total compile time\n");
                        printf("Press 9 : Code Generation\n");   
                        printf("What would you like to do? Option: ");
                        scanf("%d",&option);
                        printf("----------------------------------------------------------------------------------------------\n");
                        continue;
                    }
                    
                    tbStack->top = NULL; tbStack->size = 0; tbStack->bottom = NULL;
                    ircode = generateIRCode(ast, NULL, tbStack);
                    tbStack->top = NULL; tbStack->size = 0; tbStack->bottom = NULL;
                    newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
                    newNode->ele = symbolTableRoot;
                    newNode->next = NULL;
                    sympush(tbStack, newNode);
                    symT = symbolTableRoot;
                    fpx = fopen(argv[2], "w");
                    pre_process(fpx);
                    nasmRecur(ircode->code, tbStack, symT, fpx);
                    
                    break;
                
            

            default: 
            		printf("Wrong Option entered \n ");
        }

        printf("----------------------------------------------------------------------------------------------\n");
        printf("\nOptions:\n");
        printf("Press 0 : Exit \n");
        printf("Press 1 : LEXER \n");
        printf("Press 2 : PARSER \n");
        printf("Press 3 : AST PREORDER TRAVERSAL\n");
        printf("Press 4 : Memory \n");
        printf("Press 5 : Symbol Table\n");
        printf("Press 6 : Activation Record Size\n");
        printf("Press 7 : Static and Dynamic Arrays\n");
        printf("Press 8 : Error Reporting and total compile time\n");
        printf("Press 9 : Code Generation\n");   
        printf("What would you like to do? Option: ");
        scanf("%d",&option);
        printf("----------------------------------------------------------------------------------------------\n");
    }
}