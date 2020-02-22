#include "parser.h"
#include "lexer.h"
#include <time.h>

void seeTokenization()
{
    Token * tkn = NULL;

    tkn = getNextToken();

    while(strcmp(tkn->token, "EOF")!=0)
    {
        printf("%-20d %-20s %-20s\n", tkn->lineNum, tkn->lexeme, tkn->token);
        tkn = getNextToken();
    }

    printf("%s\n", tkn->token);
}
/*******************************************************************/
int main(int argc, char * argv[])
{
    if(argc != 3)
    {
        printf("Wrong number of args\n");
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
    printf("2. Both Lexical and Syntax Analysis are complete.");
    printf("3. Errors are reported with line numbers. This happens at the end of the compilation process (like it should)\n");
    printf("4. Syntax errors do not stop the process and parse tree is generated.\n");
    printf("5. All test cases run flawlessly!\n\n");
    printf("----------------------------------------------------------------------------------------------\n");
    
    int option=(-1);

    printf("\nOptions:\n");
    printf("Press 0 : Exit \n");
    printf("Press 1 : Removal of comments and display \n");
    printf("Press 2 : Tokens of lexeme are displayed \n");
    printf("Press 3 : Parsing tree generation and error correction \n");
    printf("Press 4 : Time Analysis of the lexer \n");
    printf("What would you like to do? Option: ");
    scanf("%d",&option);
    
    
    clock_t start_time, end_time;               //For time analysis of stage 1
    double total_CPU_time, total_CPU_time_in_seconds;
    
    while(option!=0)
    {
        switch(option)
        {
            case 1: //comment removal
                    populate_keyhash();
                    initializeLexer(argv[2]);
                    removeComments();
                    break;

            case 2: //tokens are printed
                    populate_keyhash();
                    initializeLexer(argv[2]);
                    seeTokenization();
                    if(LexHead != NULL)
                    {
                        printf("Lexical Errors:\n");
                        printErrorList(1);
                    }
                    else
                    {
                        printf("No lexical errors\n");
                    }
                    
                    break;

            case 3: //Parsing Tree and Output
                    fp = fopen("ParseTree_Output.txt","w");
                    if(fp == NULL)
                    {
                        printf("Error: Not able to open the file");
                        exit(-1);
                    }
                    populate_keyhash();
                    initializeLexer(argv[2]);
                    initializeParser();
                    grammar = read_grammar(argv[1]);
                    map(grammar);

                    firstSet = initializeFirst();
                    followSet = initializeFollow();
                    for(int  i=0; i<enumNonTerminal; i++)
                    {
                        calculateFirstSet(grammar, i, firstSet);
                    }
                    for(int i=0; i<enumNonTerminal; i++)
                    {
                        calculateFollowSet(grammar, i, followSet, firstSet);
                    }
                    parseTable = intializeParseTable();
                    createParseTable(grammar,parseTable,firstSet,followSet);
                    parseTree = parser(grammar, parseTable);
                    fprintf(fp,"%20s %20s %20s %20s %20s %20s %20s\n", "LEXEME", "LINENO", "TOKEN", "VALUE", "PARENT_NODE", "IS_LEAF", "CURR_NODE");
                    fprintf(fp,"%s\n","---------------------------------------------------------------------------------------------------------------------------------------------------------------------");
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
                    break;

            case 4: //Time analysis
                    start_time= clock();
                    //invoke lexer and parser here
                    populate_keyhash();
                    initializeLexer(argv[2]);
                    initializeParser();
                    grammar = read_grammar(argv[1]);
                    map(grammar);

                    firstSet = initializeFirst();
                    followSet = initializeFollow();
                    for(int  i=0; i<enumNonTerminal; i++)
                    {
                        calculateFirstSet(grammar, i, firstSet);
                    }
                    for(int i=0; i<enumNonTerminal; i++)
                    {
                        calculateFollowSet(grammar, i, followSet, firstSet);
                    }
                    parseTable = intializeParseTable();
                    createParseTable(grammar,parseTable,firstSet,followSet);
                    parseTree = parser(grammar, parseTable);

                    end_time= clock();
                    total_CPU_time= (end_time-start_time);
                    total_CPU_time_in_seconds = total_CPU_time/CLOCKS_PER_SEC;
                    
                    printf("Total CPU Time taken by Lexer and Parser : %lf \n", total_CPU_time);
                    printf("Total CPU Time in Seconds: %lf \n", total_CPU_time_in_seconds);

                    break;
        }

        printf("\nOptions:\n");
        printf("Press 0 : Exit \n");
        printf("Press 1 : Removal of comments and display \n");
        printf("Press 2 : Tokens of lexeme are displayed \n");
        printf("Press 3 : Parsing tree generation and error correction \n");
        printf("Press 4 : Time Analysis of the lexer \n");
        printf("What would you like to do? Option: ");
        scanf("%d",&option);
    }
    printf("\n ---------  END  ----------\n");
}