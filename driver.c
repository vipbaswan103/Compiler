#include "parser.h"
#include "lexer.h"

void seeTokenization()
{
    Token * tkn = NULL;

    tkn = getNextToken();

    while(strcmp(tkn->token, "EOF")!=0)
    {
        printf("%s\t\t%s\t\t%d\n", tkn->lexeme, tkn->token, tkn->lineNum);
        tkn = getNextToken();
    }

    printf("%s\n", tkn->token);
}
/*******************************************************************/
int main(int argc, char * argv[])
{
    // if(argc != 3)
    // {
    //     printf("Wrong number of args\n");
    // }
    // Grammar * grammar = read_grammar(argv[1]);

    // printGrammar(grammar);
    // map(grammar);

    // Stack * st = NULL;
    // Node * trav = grammar[0].arr->head;
    // printf("%d\n",grammar[0].arr->size);
    // while(trav != NULL)
    // {
    //     st = push(st, getNode(&(trav->ele)));
    //     trav = trav->next;
    // }
    // printStack(st);
    // pop(st);
    // pop(st);
    // pop(st);
    // // pop(st);
    // printf("Go again\n");
    // trav = grammar[0].arr->head;
    // while(trav != NULL)
    // {
    //     st = push(st, getNode(&(trav->ele)));
    //     trav = trav->next;
    // }
    // printStack(st);
    if(argc != 3)
    {
        printf("Wrong number of args\n");
    }
    Grammar * grammar = read_grammar(argv[1]);
    initializeLexer(argv[2]);
    // printGrammar(grammar);
    map(grammar);
    populate_keyhash();
    // seeTokenization();
    int ** firstSet = initializeFirst();
    int ** followSet = initializeFollow();
    // printf("%d\n", enumNonTerminal);
    for(int  i=0; i<enumNonTerminal; i++)
    {
        calculateFirstSet(grammar, i, firstSet);
    }
    
    for(int i=0; i<enumNonTerminal; i++)
    {
        calculateFollowSet(grammar, i, followSet, firstSet);
    }
    // printf("\n\n----------- FIRST SET -----------\n");
    // printFirst(firstSet);
    // printf("\n----------- FOLLOW SET -----------\n");
    // printFollow(followSet);

    int ** parseTable = intializeParseTable();
    createParseTable(grammar,parseTable,firstSet,followSet);
    // printParseTable(grammar,parseTable);

    // printGrammar(grammar);
    TreeNode * parseTree = parser(grammar, parseTable);
    // printf("%20s %20s %20s %20s %20s %20s %20s\n", "LEXEME", "LINENO", "TOKEN", "VALUE", "PARENT_NODE", "IS_LEAF", "CURR_NODE");
    // printf("%s\n","---------------------------------------------------------------------------------------------------------------------------------------------------------------------");
    preOrder(parseTree, NULL);
    printf("\n\n");
    // printTokenStream(parseTree);
    printErrorList(1);
    printErrorList(2);
    return 0;
}