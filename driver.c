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
        parsecount(temp);
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
    sum = sum + sizeof(astNode);
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
        astmemorycount(temp);
        temp=temp->sibling;
    }    
    sum = sum + 1;
    return sum;
}

/*******************************************************************/
int main(int argc, char * argv[])
{
    if(argc != 4)
    {
        printf("Wrong number of args. Please give 2 file names. \n The first one should be the program and the other should be where you want to write the syntax tree output\n"); 
        return 0;
        
    }
    Grammar * grammar = NULL;
    int ** parseTable;
    TreeNode * parseTree = NULL;
    int ** firstSet = NULL;
    int ** followSet = NULL;
    FILE * fp;
    initializeKeyHash();

    printf("----------------------------------------------------------------------------------------------\n");
    printf("Hello! Welcome to Compiler CSF363. This is the compiler of group 33. \n");
    printf("We are Akshit Khanna, Aryan Mehra, Vipin Baswan and Swadesh Vaibhav.\n");
    printf("Our group has successfully COMPLETED ALL REQUIRED FEATURES of the lexical and syntax analysis \n");
    printf("1. FIRST and FOLLOW is completely automated.\n");
    printf("2. Both Lexical and Syntax Analysis are complete.\n");
    printf("3. Errors are reported with line numbers. This happens at the end of the compilation process (like it should)\n");
    printf("4. Syntax errors do not stop the process and parse tree is generated.\n");
    printf("5. All test cases run flawlessly!\n\n");
    printf("----------------------------------------------------------------------------------------------\n");
    
    int option=(-1);


    printf("----------------------------------------------------------------------------------------------\n");
    printf("\nOptions:\n");
    printf("Press 0 : Exit \n");
    printf("Press 1 : LEXER \n");
    printf("Press 2 : PARSER \n");
    printf("Press 3 : Parsing tree generation and error correction \n");
    printf("Press 4 : Time Analysis of the lexer \n");
    printf("What would you like to do? Option: ");
    scanf("%d",&option);
    printf("----------------------------------------------------------------------------------------------\n");
    
    
    //For time analysis of stage 1
    clock_t start_time, end_time;               
    double total_CPU_time, total_CPU_time_in_seconds;
    
    while(option!=0)
    {
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

            // case 2: // PARSER FINAL //inorder remove the fp
                    
            //         populate_keyhash();
            //         initializeLexer(argv[1]);
            //         initializeParser();

            //         grammar = read_grammar("grammar.txt");
            //         map(grammar);

            //         firstSet = initializeFirst();
            //         followSet = initializeFollow();
            //         calculateFirstEquations(grammar, firstSet, firstEquations);
            //         calculateFirstSet(grammar, firstSet, firstEquations);

            //         calculateFollowEquations(grammar, followSet, firstSet, followEquations);
            //         calculateFollowSet(grammar, followSet, followEquations);
                    
            //         parseTable = intializeParseTable();
            //         createParseTable(grammar,parseTable,firstSet,followSet);
            //         parseTree = parser(grammar, parseTable);
            //         printf("%15s %10s %15s %15s %20s %10s %20s\n", "LEXEME", "LINENO", "TOKEN", "VALUE", "PARENT_NODE", "IS_LEAF", "CURR_NODE");
            //         printf("%s\n","--------------------------------------------------------------------------------------------------------------------------------");
            //         inOrder(parseTree, NULL);
            //         printf("%s\n","--------------------------------------------------------------------------------------------------------------------------------");
            //         printf("%s\n","--------------------------------------------------------------------------------------------------------------------------------");
            //         printf("\n\n");

            //         if(LexHead != NULL)
            //         {
            //             printf("Lexical Errors:\n");
            //             printErrorList(1);
            //         }
            //         else
            //         {
            //             printf("No lexical errors!\n");
            //         }
                    
            //         if(SynHead != NULL)
            //         {
            //             printf("Syntax Errors:\n");
            //             printErrorList(2);
            //         }
            //         else
            //         {
            //             printf("Parsing was successfull......!\n");
            //         }
                    

            case 3: //Parsing Tree and Output
                    fp = fopen(argv[2],"w");
                    if(fp == NULL)
                    {
                        printf("Error: Not able to open the file");
                        exit(-1);
                    }
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
                    fprintf(fp,"%15s %10s %15s %15s %20s %10s %20s\n", "LEXEME", "LINENO", "TOKEN", "VALUE", "PARENT_NODE", "IS_LEAF", "CURR_NODE");
                    fprintf(fp,"%s\n","--------------------------------------------------------------------------------------------------------------------------------");
                    inOrder(fp, parseTree, NULL);
                    printf("\n\n");
                    if(LexHead != NULL)
                    {
                        printf("Lexical Errors:\n");
                        printErrorList(1);
                    }
                    else
                    {
                        printf("No lexical errors!\n");
                    }
                    
                    if(SynHead != NULL)
                    {
                        printf("Syntax Errors:\n");
                        printErrorList(2);
                    }
                    else
                    {
                        printf("Parsing was successfull......!\n");
                    }
                    fclose(fp);
                    astNode * ast = createAST(parseTree, NULL, NULL);
                    fp = fopen(argv[3], "w");
                    printAST(ast, fp);
                    initializeErrorList();
                    symbolTable *table = NULL;
                    formulation(ast, table);
                    printSymbolTable(symbolTableRoot);
                    tableStack *tbStack= (tableStack*)malloc(sizeof(tableStack));
                    tbStack->top = NULL;
                    tbStack->size = 0;
                    tbStack->bottom = NULL;
                    typeChecker(ast, tbStack);
                    printSemanticErrors();
                    
                    // quad * labels = (quad *)malloc(sizeof(quad));
                    tbStack->top = NULL;
                    tbStack->size = 0;
                    tbStack->bottom = NULL;
                    intermed * ircode = generateIRCode(ast, NULL, tbStack);
                    printCode(ircode->code);
                    printSymbolTable(symbolTableRoot);
                    tbStack->top = NULL;
                    tbStack->size = 0;
                    tbStack->bottom = NULL;
                    tableStackEle * newNode = (tableStackEle *)malloc(sizeof(tableStackEle));
                    newNode->ele = symbolTableRoot;
                    newNode->next = NULL;
                    sympush(tbStack, newNode);
                    symbolTable * symT = symbolTableRoot;
                    FILE * fpx = fopen("code.asm", "w");
                    pre_process(fpx);
                    nasmRecur(ircode->code, tbStack, symT, fpx);
                    // freeing memory not needed anymore
                    // freeprasetree(parseTree);
                    // freegrammar(grammar);
                    break;

            case 4: //Time analysis
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

                    end_time= clock();
                    total_CPU_time= (end_time-start_time);
                    
                    //time we get is in milliseconds
                    total_CPU_time_in_seconds = total_CPU_time/CLOCKS_PER_SEC;
                    
                    printf("Total CPU Time taken by Lexer and Parser : %lf \n", total_CPU_time);
                    printf("Total CPU Time in Seconds: %lf \n", total_CPU_time_in_seconds);

                    break;
                
            // case 5: // MEMORY ANALYSIS 
                    
            //         populate_keyhash();
            //         initializeLexer(argv[1]);
            //         initializeParser();

            //         grammar = read_grammar("grammar.txt");
            //         map(grammar);

            //         firstSet = initializeFirst();
            //         followSet = initializeFollow();
            //         calculateFirstEquations(grammar, firstSet, firstEquations);
            //         calculateFirstSet(grammar, firstSet, firstEquations);

            //         calculateFollowEquations(grammar, followSet, firstSet, followEquations);
            //         calculateFollowSet(grammar, followSet, followEquations);
                    
            //         parseTable = intializeParseTable();
            //         createParseTable(grammar,parseTable,firstSet,followSet);
            //         parseTree = parser(grammar, parseTable);
            //         printf("%15s %10s %15s %15s %20s %10s %20s\n", "LEXEME", "LINENO", "TOKEN", "VALUE", "PARENT_NODE", "IS_LEAF", "CURR_NODE");
            //         printf("%s\n","--------------------------------------------------------------------------------------------------------------------------------");
            //         inOrder(parseTree, NULL);
            //         printf("%s\n","--------------------------------------------------------------------------------------------------------------------------------");
            //         printf("%s\n","--------------------------------------------------------------------------------------------------------------------------------");
            //         printf("\n\n");

            //         if(LexHead != NULL)
            //         {
            //             printf("Lexical Errors:\n");
            //             printErrorList(1);
            //         }
            //         else
            //         {
            //             printf("No lexical errors!\n");
            //         }
                    
            //         if(SynHead != NULL)
            //         {
            //             printf("Syntax Errors:\n");
            //             printErrorList(2);
            //         }
            //         else
            //         {
            //             printf("Parsing was successfull......!\n");
            //         }
            //         fclose(fp);
            //         astNode * ast = createAST(parseTree, NULL, NULL);
            
            //         int pm = parsememorycount(parseTree);
            //         int pc = parsecount(parseTree);
            //         int am = astmemorycount(ast);
            //         int ac = astcount(ast);
            //         int compression_ratio = ((pm-am)/pm)*100;

            //         printf("Parse Tree number of allocated nodes are: %d \n ", pc);
            //         printf("Parse Tree allocated memory: %d \n ", pm);
            //         printf("AST number of allocated nodes are: %d \n ", ac);
            //         printf("AST allocated memory: %d \n ", am);
            //         printf("Compression ratio: %d \n", compression_ratio);

            default: 
            		printf("Wrong Option entered \n ");
        }

        printf("----------------------------------------------------------------------------------------------\n");
        printf("\nOptions:\n");
        printf("Press 0 : Exit \n");
        printf("Press 1 : LEXER \n");
        printf("Press 2 : PARSER \n");
        printf("Press 3 : Parsing tree generation and error correction \n");
        printf("Press 4 : Time Analysis of the lexer \n");
        printf("What would you like to do? Option: ");
        scanf("%d",&option);
        printf("----------------------------------------------------------------------------------------------\n");
    }
}
