/* 
	GROUP 33
	Aryan Mehra 2017A7PS0077P
	Akshit Khanna 2017A7PS0023P
   	Vipin Baswan 2017A7PS0429P
   	Swadesh Vaibhav 2017A7PS0030P
*/

#include "ast.h"
#include "parserDef.h"

// function to make the internal (non leaf node) of the AST
astNode* makeASTnode(char * label, astNode ** childs, int size)
{
    // create a node and set its parameters according to an internal node
    astNode *parent = (astNode*)malloc(sizeof(astNode));
    parent->node = (astEle*)malloc(sizeof(astEle));
    parent->node->tag = Internal;
    parent->node->ele.internalNode = (internal *) malloc(sizeof(internal));
    parent->node->ele.internalNode->label = label;
    parent->sibling = NULL;
    parent->child = childs[0];
    // depending on how many children you are passing
    // keep that as the children of the node
    // afterall, makenode combines all the nodes
    if(childs==NULL)
    {
        printf("Trying to create an internal node, with no children.. Debugging error");
    }
    else
    {
        // simply get all the children in the list
        astNode *temp;
        temp = childs[0];
        for(int i=1 ; i<size ; i++)
        {
            while(temp->sibling!=NULL)
                temp =temp->sibling;

            temp->sibling = childs[i];
            temp = childs[i];
        }
    }
    return parent;    
}


// special function to make the leaf node
astNode* makeLeafNode(TreeNode * leafnode)
{
    // take as input the parse tree node and create a new leaf node
    astNode * newNode = (astNode *)malloc(sizeof(astNode));
    newNode->child = NULL;
    newNode->sibling = NULL;
    newNode->node = (astEle*)malloc(sizeof(astEle));
    newNode->node->tag = Leaf;
    newNode->node->ele.leafNode = (leaf *) malloc(sizeof(leaf));
    newNode->node->ele.leafNode->lexeme = leafnode->ele.leaf.tkn.lexeme;
    newNode->node->ele.leafNode->lineNum = leafnode->ele.leaf.tkn.lineNum;
    newNode->node->ele.leafNode->type = leafnode->ele.leaf.tkn.token;
    newNode->node->ele.leafNode->value = leafnode->ele.leaf.tkn.value;

    return newNode;
}

// takes a list and node to add at the end of the list
astNode * concatenate(astNode * head, astNode * newNode)
{
    if(head == NULL)
    {
        head = newNode;
        return head;
    }

    astNode * trav = head;
    while(trav->sibling != NULL)
    {
        trav = trav->sibling;
    }

    trav->sibling = newNode;
    newNode->sibling = NULL;
    return head;
}

// makelistnode function called when you create 
astNode * makeListNode(char * label, astNode * list)
{
    astNode * parent = (astNode *)malloc(sizeof(astNode));
    parent->node = (astEle *)malloc(sizeof(astEle));
    parent->node->tag = Internal;
    parent->node->ele.internalNode->label = label;
    parent->child = list;
    parent->sibling = NULL;
    return parent;
}

// takes a list and assign the first elements linNum as startNum, last ones lineNum as end 
void getLineNums(astNode * list, int * start, int * end)
{
    astNode * tmp = list;
    if(tmp == NULL)
    {
        *start = -1;
        *end = -1;
        return;
    }
    if(tmp->node->tag == Internal)
        *start = tmp->node->ele.internalNode->lineNumStart;
    else
        *start = tmp->node->ele.leafNode->lineNum;
    
    while(tmp->sibling!=NULL)
        tmp = tmp->sibling;

    if(tmp->node->tag == Internal)
        *end = tmp->node->ele.internalNode->lineNumEnd;
    else
        *end = tmp->node->ele.leafNode->lineNum;
}


astNode * createAST(TreeNode *parseNode, astNode *inh, astNode **syn)
{
    //Non-Terminal
    if(parseNode->tag == 1)
    {
        // printf("Non Terminal - %s\n",parseNode->ele.nonleaf.nt.str);

        astNode* arrASTnodes[4];

        if(!strcmp(parseNode->ele.nonleaf.nt.str,"program"))
        {

            // Rule 1
            // <program> -> <moduleDeclarations> <otherModules> <driverModule> <otherModules>            // Rule 1
            astNode *programNode;
            astNode *modDecSyn = NULL;
            
            //recurcively call the function on the children
            //pass the inhereted and synthesised attributes as needed
            // mostly synthesised attricbutes are child nodes themselves
            createAST(parseNode->child, NULL, &modDecSyn);
            arrASTnodes[0] = modDecSyn; 
            astNode * modDec = makeASTnode("MODULEDEC",arrASTnodes ,1);
            getLineNums(modDecSyn, &(modDec->node->ele.internalNode->lineNumStart), &(modDec->node->ele.internalNode->lineNumEnd));

            astNode *othMod1_syn = NULL, *othMod2_syn = NULL;
            astNode *othMod1_node = NULL, *othMod2_node = NULL;
            createAST(parseNode->child->sibling, NULL, &othMod1_syn);
            astNode *driverMod_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
            createAST(parseNode->child->sibling->sibling->sibling, NULL, &othMod2_syn);
            
            arrASTnodes[0] = othMod1_syn;
            othMod1_node = makeASTnode("MODULES1", arrASTnodes, 1);
            arrASTnodes[0] = othMod2_syn;
            othMod2_node = makeASTnode("MODULES2", arrASTnodes, 1);
            getLineNums(othMod1_syn, &(othMod1_node->node->ele.internalNode->lineNumStart), &(othMod1_node->node->ele.internalNode->lineNumEnd));
            getLineNums(othMod2_syn, &(othMod2_node->node->ele.internalNode->lineNumStart), &(othMod2_node->node->ele.internalNode->lineNumEnd));
            // we have all 4 children to create a program AST node
            arrASTnodes[0] = modDec;
            arrASTnodes[1] = othMod1_node;
            arrASTnodes[2] = driverMod_node;
            arrASTnodes[3] = othMod2_node;
            programNode = makeASTnode("PROGRAM", arrASTnodes, 4);
            
            // TODO : Check it
            if(modDec->child != NULL)
                programNode->node->ele.internalNode->lineNumStart = modDec->node->ele.internalNode->lineNumStart;
            else if(othMod1_node->child != NULL)    
                programNode->node->ele.internalNode->lineNumStart = othMod1_node->node->ele.internalNode->lineNumStart;
            else
                programNode->node->ele.internalNode->lineNumStart = driverMod_node->node->ele.internalNode->lineNumStart;

            if(othMod2_node->child != NULL)
                programNode->node->ele.internalNode->lineNumEnd = othMod2_node->node->ele.internalNode->lineNumEnd;
            else
                programNode->node->ele.internalNode->lineNumEnd = driverMod_node->node->ele.internalNode->lineNumEnd;
                    
            return programNode;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str,"moduleDeclarations"))
        {   
            //Rule 2
            // <moduleDeclarations1> -> <moduleDeclaration><moduleDeclarations2>
            // moduleDeclarations2.inh = concatenate(moduleDeclarattion.node, moduleDeclarations1.inh)
            // moduleDecalarations1.syn = moduleDeclarations2.syn
            if(parseNode->child->tag == 1)
            {
                astNode *modDecsSyn,*modDecsInh;
                astNode *modDec = createAST(parseNode->child, NULL, NULL);
                modDecsInh = concatenate(inh, modDec);
                createAST(parseNode->child->sibling, modDecsInh, &modDecsSyn);
                *syn = modDecsSyn;
            }
            // Rule 3 
            // <moduleDeclarations> -> ε
            else
            {
                *syn = inh;
            }
            return NULL;
        }
        
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "moduleDeclaration"))
        {
            // Rule 4 
            // <moduleDeclaration> ---- DECLARE MODULE ID SEMICOL
            return createAST(parseNode->child->sibling->sibling, NULL, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "otherModules"))
        {
            // Rule 5
            // <otherModules> ---- <module> <otherModules> 
            if(parseNode->child->tag == 1)
            {
                astNode *otherMod2_syn = NULL, *otherMod2_inh = NULL;
                astNode *moduleNode = createAST(parseNode->child, NULL, NULL);
                otherMod2_inh = concatenate(inh, moduleNode);
                createAST(parseNode->child->sibling, otherMod2_inh, &otherMod2_syn);
                *syn = otherMod2_syn;
            }
            // Rule 6
            // <otherModules> ---- ε
            else
            {
                *syn = inh;   
            }
            return NULL;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "driverModule"))
        {
            // Rule 7
            // <driverModule> ---- DRIVERDEF DRIVER PROGRAM DRIVERENDDEF <moduleDef>
            astNode *modDef_node = createAST(parseNode->child->sibling->sibling->sibling->sibling, NULL, NULL);
            arrASTnodes[0] = modDef_node;
            astNode *driverNode = makeASTnode("DRIVER", arrASTnodes, 1);
            driverNode->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
            driverNode->node->ele.internalNode->lineNumEnd = driverNode->child->node->ele.internalNode->lineNumEnd;
            return driverNode;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "module"))
        {
            // Rule 8
            // <module> ---- DEF MODULE ID ENDDEF TAKES INPUT SQBO <input_plist> SQBC SEMICOL <ret><moduleDef>
            astNode *ID_node = NULL, *inpList_node = NULL, *ret_node = NULL, *modDef_node = NULL, *mod_node = NULL;
            ID_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
            inpList_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
            ret_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
            modDef_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling, NULL, NULL);

            // if ret is NULL make it a makeshift internal node to let it be a part of chain
            astNode * ret = ret_node;
            if(ret == NULL)
            {
                arrASTnodes[0] = ret_node;
                ret = makeASTnode("OUTPUT_LIST",arrASTnodes,1);
            }

            arrASTnodes[0] = ID_node;
            arrASTnodes[1] = inpList_node;
            arrASTnodes[2] = ret;
            arrASTnodes[3] = modDef_node;
            astNode * modNode = makeASTnode("MODULE", arrASTnodes, 4);
            modNode->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
            modNode->node->ele.internalNode->lineNumEnd = modNode->child->sibling->sibling->sibling->node->ele.internalNode->lineNumEnd;
            return modNode;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "ret"))
        {
            // Rule 9
            // <ret> ---- RETURNS SQBO <output_plist> SQBC SEMICOL 
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "RETURNS"))
                return createAST(parseNode->child->sibling->sibling, NULL, NULL);
            
            // Rule 10
            // <ret> ---- ε
            else 
                return NULL;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "input_plist"))
        {
            //Rule 11
            //<input_plist> ---- ID COLON  <dataType> <input_plist2>
            astNode *input2Inh = NULL, *input2Syn = NULL;
            astNode *IDNode = createAST(parseNode->child, NULL, NULL);
            astNode *datatypeNode = createAST(parseNode->child->sibling->sibling, NULL, NULL);
            input2Inh = concatenate(IDNode, datatypeNode);
            createAST(parseNode->child->sibling->sibling->sibling, input2Inh, &input2Syn);
            arrASTnodes[0] = input2Syn;
            astNode *inputNode = makeASTnode("INPUT_LIST", arrASTnodes, 1);
            getLineNums(input2Syn, &(inputNode->node->ele.internalNode->lineNumStart), &(inputNode->node->ele.internalNode->lineNumEnd));
            return inputNode;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "input_plist2"))
        {
            
            //Rule 12
            //<input_plist2>  ---- COMMA ID COLON <dataType> <input_plist2>

            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "COMMA"))
            {
                astNode * ID_node = createAST(parseNode->child->sibling, NULL, NULL);
                astNode * dataType_node = createAST(parseNode->child->sibling->sibling->sibling, NULL, NULL);
                astNode *inpList22_inh = NULL, *inpList22_syn = NULL;
                inpList22_inh = concatenate(inh, ID_node);
                inpList22_inh = concatenate(inh, dataType_node);
                createAST(parseNode->child->sibling->sibling->sibling->sibling, inpList22_inh, &inpList22_syn);
                *syn = inpList22_syn;
            }

            // Rule 13
            // <input_plist2>  ---- ε

            else
            {
                *syn = inh;
            }
            return NULL;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "output_plist"))
        {
            // Rule 14
            // <output_plist> ---- ID COLON <type> <output_plist2>

            //Rule 14
            //<output_plist> ---- ID COLON <type> <output_plist2>
            astNode * ID_node = createAST(parseNode->child, NULL, NULL);
            astNode * dataType_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
            astNode *outList2_inh = NULL, *outList2_syn = NULL;
            outList2_inh = concatenate(ID_node, dataType_node);
            createAST(parseNode->child->sibling->sibling->sibling, outList2_inh, &outList2_syn);
            arrASTnodes[0] = outList2_syn;
            astNode *output_node = makeASTnode("OUTPUT_LIST", arrASTnodes, 1);
            getLineNums(outList2_syn, &(output_node->node->ele.internalNode->lineNumStart), &(output_node->node->ele.internalNode->lineNumEnd));
            return output_node;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "output_plist2"))
        {
            //Rule 15
            //<output_plist2> ---- COMMA ID COLON <type> <output_plist2> 
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "COMMA"))
            {
                astNode * ID_node = createAST(parseNode->child->sibling, NULL, NULL);
                astNode * type_node = createAST(parseNode->child->sibling->sibling->sibling, NULL, NULL);
                astNode *outList22_inh = NULL, *outList22_syn = NULL;
                outList22_inh = concatenate(inh, ID_node);
                outList22_inh = concatenate(inh, type_node);
                createAST(parseNode->child->sibling->sibling->sibling->sibling, outList22_inh, &outList22_syn);
                *syn = outList22_syn;
            }    
            //Rule 16
            //<output_plist2> ---- ε
            else 
                *syn = inh;
                
            return NULL;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "dataType"))
        {
            //Rule 17, 18, 19
            //<dataType> ---- INTEGER 
            //<dataType> ---- REAL
            //<dataType> ---- BOOLEAN 
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "INTEGER") || !strcmp(parseNode->child->ele.leaf.tkn.token, "REAL") || !strcmp(parseNode->child->ele.leaf.tkn.token, "BOOLEAN"))
            {
                return createAST(parseNode->child, NULL, NULL);
            }
            //Rule 20
            //<dataType> ---- ARRAY SQBO <range1> SQBC OF <type>
            else
            {
                astNode *range1_node = NULL, *type_node = NULL;
                range1_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                type_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
                arrASTnodes[0] = range1_node;
                arrASTnodes[1] = type_node;
                astNode * dtNode = makeASTnode("ARRAY", arrASTnodes, 2);
                dtNode->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
                dtNode->node->ele.internalNode->lineNumEnd = dtNode->child->sibling->node->ele.leafNode->lineNum;
                return dtNode;
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "type"))
        {
            //Rule 21, 22, 23
            //<type> ---- INTEGER 
            //<type> ---- REAL
            //<type> ---- BOOLEAN 
            return createAST(parseNode->child, NULL, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "moduleDef"))
        {
            //Rule 24
            //<moduleDef> ---- START <statements> END
            astNode *stmts_inh = NULL, *stmts_syn = NULL;
            createAST(parseNode->child->sibling, stmts_inh, &stmts_syn);
            arrASTnodes[0] = stmts_syn;
            astNode *moduleDef = makeASTnode("MODULEDEF", arrASTnodes, 1);
            moduleDef->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
            moduleDef->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->ele.leaf.tkn.lineNum;
            return moduleDef;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "statements"))
        {
            //Rule 25
            //<statements> ---- <statement> <statements>
            if(!strcmp(parseNode->child->ele.nonleaf.nt.str, "statement"))
            {
                astNode *stmt_node = createAST(parseNode->child, NULL, NULL);
                astNode *stmts2_inh = NULL, *stmts2_syn = NULL;
                stmts2_inh = concatenate(inh, stmt_node);
                createAST(parseNode->child->sibling, stmts2_inh, &stmts2_syn);
                *syn = stmts2_syn;
            }
            //Rule 26
            //<statements> ---- ε
            else
            {
                *syn = inh;
            }
            return NULL;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "statement"))
        {
            //Rule 27, 28, 29, 30, 31
            //<statement> ---- <ioStmt>
            //<statement> ---- <simpleStmt>
            //<statement> ---- <declareStmt>
            //<statement> ---- <conditionalStmt>
            //<statement> ---- <iterativeStmt>
            return createAST(parseNode->child, NULL, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "ioStmt"))
        {
            // Rule 32
            // <ioStmt> ---- GET_VALUE BO ID BC SEMICOL 
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "GET_VALUE"))
            {
                astNode *ID_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                arrASTnodes[0] = ID_node;
                astNode *ioNode = makeASTnode("GET_VAL", arrASTnodes, 1);
                ioNode->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
                ioNode->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
                return ioNode;
            }
            // Rule 33
            // <ioStmt> ---- PRINT BO <varAndBool> BC SEMICOL
            else
            {
                astNode *varBool_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                arrASTnodes[0] = varBool_node;
                astNode *ioNode = makeASTnode("PRINT", arrASTnodes, 1);
                ioNode->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
                ioNode->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
                return ioNode;
            }
            
        }       
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "varAndBool"))
        {
            // Rule 34 and 35
            // <varAndBool> ---- <var>
            // <varAndBool> ---- <boolConst>
            return createAST(parseNode->child, NULL, NULL);
        }

        
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "var"))
        {
            // Rule 36
            // <var> ---- ID <whichId>
            // var.node = makenode(‘ID_ARR’, ID.node, whichId.node)
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "ID"))
            {
                astNode *IDNode = createAST(parseNode->child, NULL, NULL);
                astNode *whichIDNode = createAST(parseNode->child->sibling, NULL, NULL);
                arrASTnodes[0] = IDNode;
                arrASTnodes[1] = whichIDNode;
                astNode *idarrNode = makeASTnode("ID_ARR", arrASTnodes, 2);
                idarrNode->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
                
                if(whichIDNode!=NULL)
                    idarrNode->node->ele.internalNode->lineNumEnd = idarrNode->child->sibling->node->ele.leafNode->lineNum;
                else
                    idarrNode->node->ele.internalNode->lineNumEnd = idarrNode->node->ele.internalNode->lineNumStart;
                
                
                return idarrNode;
            
            }
            else 
            {
                // Rule 37 and 38
                // <var> ---- NUM 
                // <var> ---- RNUM
                return createAST(parseNode->child, NULL, NULL);
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "whichId"))
        {
            // Rule 39
            // <whichId> ---- SQBO <index> SQBC 
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "SQBO"))
            {
                return createAST(parseNode->child->sibling, NULL, NULL);
            }

            // Rule 40
            // <whichId> ---- ε
            else 
            {
                return NULL;
            }
        }

        
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "simpleStmt"))
        {
            // Rule 41 and 42
            // <simpleStmt> ---- <assignmentStmt> 
            // <simpleStmt> ---- <moduleReuseStmt>
            return createAST(parseNode->child, NULL, NULL);
        }

        
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "assignmentStmt"))
        {
            //Rule 43
            //<assignmentStmt> ---- ID <whichStmt>   
            astNode *whichStmt_inh = NULL, *ID_node = NULL;
            ID_node = createAST(parseNode->child, NULL, NULL);
            whichStmt_inh = ID_node;
            return createAST(parseNode->child->sibling, whichStmt_inh, NULL);
        }

        
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "whichStmt"))
        {
            //Rule 44 and 45
            // <whichStmt> ---- <lvalueIDStmt> 
            // <whichStmt> ---- <lvalueARRStmt>
            astNode * child_inh = inh;
            return createAST(parseNode->child, child_inh, NULL);
        }

        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "lvalueIDStmt"))
        {
            // Rule 46
            // <lvalueIDStmt> ---- ASSIGNOP <expression> SEMICOL
            // astNode *assignNode = createAST(parseNode->child, NULL, NULL);
            astNode *exprNode = createAST(parseNode->child->sibling, NULL, NULL);
            arrASTnodes[0] = inh;
            arrASTnodes[1] = exprNode;
            astNode * lvalNode = makeASTnode("ASSIGNOP", arrASTnodes, 2);
            lvalNode->node->ele.internalNode->lineNumStart = inh->node->ele.leafNode->lineNum;
            lvalNode->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->ele.leaf.tkn.lineNum;
            return lvalNode;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "lvalueARRStmt"))
        {
            // Rule 47
            // <lvalueARRStmt> ---- SQBO <index> SQBC ASSIGNOP <expression> SEMICOL
            astNode *ind_node = createAST(parseNode->child->sibling, NULL, NULL);
            astNode * expr_node = createAST(parseNode->child->sibling->sibling->sibling->sibling, NULL, NULL);

            arrASTnodes[0] = inh;
            arrASTnodes[1] = ind_node;
            arrASTnodes[2] = expr_node;
            astNode * lvalARRNode = makeASTnode("ASSIGNOPARR", arrASTnodes, 3);
            lvalARRNode->node->ele.internalNode->lineNumStart = inh->node->ele.leafNode->lineNum;
            lvalARRNode->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
            return lvalARRNode;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "index"))
        {
            // Rule 48, 49
            // <index> ----  NUM
            // <index> ----  ID
            return createAST(parseNode->child, NULL, NULL);   
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "moduleReuseStmt"))
        {
            //Rule 50
            //<moduleReuseStmt> ---- <optional> USE MODULE ID WITH PARAMETERS <idList> SEMICOL
            astNode *optional_node = createAST(parseNode->child, NULL, NULL);
            astNode *ID_node = createAST(parseNode->child->sibling->sibling->sibling, NULL, NULL);
            astNode *idList_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
            
            if(optional_node == NULL)
            {
                arrASTnodes[0] = optional_node;
                optional_node = makeASTnode("ID_LIST", arrASTnodes, 1);
            }
            arrASTnodes[0] = optional_node;
            arrASTnodes[1] = ID_node;
            arrASTnodes[2] = idList_node;
            astNode * tmp = makeASTnode("MODULECALL", arrASTnodes, 3);

            if(optional_node->child == NULL)
                tmp->node->ele.internalNode->lineNumStart = parseNode->child->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
            else
                tmp->node->ele.internalNode->lineNumStart = optional_node->node->ele.internalNode->lineNumStart;
            tmp->node->ele.internalNode->lineNumEnd = tmp->child->sibling->sibling->node->ele.internalNode->lineNumEnd;
            // if(optional_node == NULL)
            // {
            //     return tmp;
            // }
            // else
            // {
            //     arrASTnodes[0] = optional_node;
            //     arrASTnodes[1] = tmp;
            //     astNode * assgNode = makeASTnode("MODULEASSIGNOP", arrASTnodes, 2);
            //     assgNode->node->ele.internalNode->lineNumStart = assgNode->child->node->ele.internalNode->lineNumStart;
            //     assgNode->node->ele.internalNode->lineNumEnd = assgNode->child->sibling->node->ele.internalNode->lineNumEnd;
            //     return assgNode;
            // }
            return tmp;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "optional"))
        {
            // Rule 51
            // <optional> ---- SQBO <idList> SQBC ASSIGNOP
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "SQBO"))
                return createAST(parseNode->child->sibling, NULL, NULL);
            // Rule 52
            // <optional> ---- ε s
            else
                return NULL;            
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "idList"))
        {
            //Rule 53
            // <idList> ---- ID <idList2>
            astNode *idList2Inh,*idList2Syn = NULL;
            astNode *IDNode = createAST(parseNode->child, NULL, NULL);
            idList2Inh = concatenate(inh, IDNode);
            createAST(parseNode->child->sibling, idList2Inh, &idList2Syn);
            arrASTnodes[0] = idList2Syn;
            astNode *idList =  makeASTnode("ID_LIST", arrASTnodes, 1);
            getLineNums(idList2Syn, &(idList->node->ele.internalNode->lineNumStart), &(idList->node->ele.internalNode->lineNumEnd));
            return idList;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "idList2"))
        {
            //Rule 54
            //<idList2> ---- COMMA ID <idList2>
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "COMMA"))
            {
                astNode *ID_node = createAST(parseNode->child->sibling, NULL, NULL);
                astNode *idList22_inh = concatenate(inh, ID_node);
                astNode *idList22_syn = NULL;
                createAST(parseNode->child->sibling->sibling, idList22_inh, &idList22_syn);
                
                *syn = idList22_syn;
            }
            //Rule 55
            //<idList2> ---- ε
            else
            {
                *syn = inh;
            }
            return NULL;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "expression"))
        {
            // Rule 56 and 57 boh are automatically considered 
            // <expression> ---- <expression2>
            // <expression> ----  <unaryExprArithmetic> 
            return createAST(parseNode->child, NULL, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "expression2"))
        {
            // Rule 58 and 59  
            // <expression2> ---- <myChoice> <expression3>
            astNode *exp3Inh = NULL;
            astNode *logExpOrboolConst = createAST(parseNode->child, NULL, NULL);
            exp3Inh = logExpOrboolConst;
            return createAST(parseNode->child->sibling, exp3Inh, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "myChoice"))
        {
            return createAST(parseNode->child, NULL, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "expression3"))
        {
            //Rule 60
            //<expression3> ---- <logicalOp> <myChoice> <expression3>
            if(!strcmp(parseNode->child->ele.nonleaf.nt.str, "logicalOp"))
            {
                astNode *logOp_syn = createAST(parseNode->child, NULL, NULL);
                astNode *myChoice_node = createAST(parseNode->child->sibling, NULL, NULL);
                arrASTnodes[0] = inh;
                arrASTnodes[1] = myChoice_node;
                astNode *tmpNode = makeASTnode(logOp_syn->node->ele.leafNode->type, arrASTnodes, 2);
                

                if(inh->node->tag == Internal)
                    tmpNode->node->ele.internalNode->lineNumStart = inh->node->ele.internalNode->lineNumStart;
                else
                    tmpNode->node->ele.internalNode->lineNumStart = inh->node->ele.leafNode->lineNum;
                    
                if(tmpNode->child->sibling->node->tag == Internal)
                    tmpNode->node->ele.internalNode->lineNumEnd = tmpNode->child->sibling->node->ele.internalNode->lineNumEnd;
                else
                    tmpNode->node->ele.internalNode->lineNumEnd = tmpNode->child->sibling->node->ele.leafNode->lineNum;

                astNode *tmpInh = tmpNode;
                astNode * retNode = createAST(parseNode->child->sibling->sibling, tmpInh, NULL);

                // astNode *expr2_node = createAST(parseNode->child->sibling, NULL, NULL);
                
                // arrASTnodes[0] = inh;
                // arrASTnodes[1] = expr2_node;
                
                // astNode * expr3Node = makeASTnode(logOp_syn->node->ele.leafNode->type, arrASTnodes, 2);
                return retNode;
            }
            //Rule 61
            //<expression3> ---- ε
            else
            {
                return inh;
            }
        }

        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "logicalExpr"))
        {
            
            // Rule 62
            // <logicalExpr> ---- <arithmeticExpr> <logicalExpr2> 
            // logicalExpr2.inh = arithmeticExpr.node
            // logicalExpr.node = logicalExpr2.node
         
            astNode *arithNode = createAST(parseNode->child, NULL, NULL);  
            astNode *logical2inh = arithNode;
            return createAST(parseNode->child->sibling, logical2inh, NULL);
        }
        
        

        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "logicalExpr2"))
        {
            // Rule 63
            // <logicalExpr2> ---- <relationalOp> <arithmeticExpr>
            if((parseNode->child->tag == 1))
            {
                astNode *relOp = createAST(parseNode->child, NULL, NULL);
                astNode *arithExp = createAST(parseNode->child->sibling, NULL, NULL);
                arrASTnodes[0] = inh;
                arrASTnodes[1] = arithExp;
                astNode *logExpr2 = makeASTnode(relOp->node->ele.leafNode->type, arrASTnodes, 2);
                if(inh->node->tag == Internal)
                    logExpr2->node->ele.internalNode->lineNumStart = inh->node->ele.internalNode->lineNumStart;
                else
                    logExpr2->node->ele.internalNode->lineNumStart = inh->node->ele.leafNode->lineNum;
                if(arithExp->node->tag == Internal)
                    logExpr2->node->ele.internalNode->lineNumEnd = arithExp->node->ele.internalNode->lineNumEnd;
                else
                    logExpr2->node->ele.internalNode->lineNumEnd = arithExp->node->ele.leafNode->lineNum;
                return logExpr2;                        
            }
            // Rule  64
            // <logicalExpr2> ---- ε
            else 
            {
                return inh;       
            } 
        }

        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "arithmeticExpr"))
        {

            // Rule 65
            // <arithmeticExpr> ---- <term> <arithmeticExpr2>	
            
            astNode *arithExp2Inh = NULL;
            astNode *term = createAST(parseNode->child, NULL, NULL);
            arithExp2Inh = term;
            return createAST(parseNode->child->sibling, arithExp2Inh, NULL);
        }

        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "arithmeticExpr2"))
        {
            // Rule 66
            // <arithmeticExpr2> ---- <op1> <term> <arithmeticExpr>
            if(parseNode->child->tag == 1)
            {
                astNode *op1 = createAST(parseNode->child, NULL, NULL);
                astNode *termNode = createAST(parseNode->child->sibling, NULL, NULL);

                arrASTnodes[0] = inh;
                arrASTnodes[1] = termNode;

                astNode *tmpNode = makeASTnode(op1->node->ele.leafNode->type, arrASTnodes, 2);

                if(inh->node->tag == Internal)
                    tmpNode->node->ele.internalNode->lineNumStart = inh->node->ele.internalNode->lineNumStart;
                else
                    tmpNode->node->ele.internalNode->lineNumStart = inh->node->ele.leafNode->lineNum;
                
                if(tmpNode->child->sibling->node->tag == Internal)
                    tmpNode->node->ele.internalNode->lineNumEnd = tmpNode->child->sibling->node->ele.internalNode->lineNumStart;
                else
                    tmpNode->node->ele.internalNode->lineNumEnd = tmpNode->child->sibling->node->ele.leafNode->lineNum;
                
                astNode *tmpInh = tmpNode;
                astNode *arithExp2 = createAST(parseNode->child->sibling->sibling, tmpInh, NULL);
                
                // arrASTnodes[0] = inh;
                // arrASTnodes[1] = arithExp;
                // astNode *arithExpr2 = makeASTnode(op1->node->ele.leafNode->type, arrASTnodes, 2);
                // if(inh->node->tag == Internal)
                //     arithExpr2->node->ele.internalNode->lineNumStart = inh->node->ele.internalNode->lineNumStart;
                // else
                //     arithExpr2->node->ele.internalNode->lineNumStart = inh->node->ele.leafNode->lineNum;
                // if(arithExp->node->tag == Internal)
                //     arithExpr2->node->ele.internalNode->lineNumEnd = arithExp->node->ele.internalNode->lineNumEnd;
                // else
                //     arithExpr2->node->ele.internalNode->lineNumEnd = arithExp->node->ele.leafNode->lineNum;
                return arithExp2;       
            }
            // Rule 67
            // <arithmeticExpr2> ---- ε 
            else
            {
                return inh;
            }
        }

        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "term"))
        {
            //Rule 68
            // <term> ---- <factor> <term2>
            astNode *fact_node = createAST(parseNode->child, NULL, NULL);
            astNode *term2_inh = fact_node;
            return createAST(parseNode->child->sibling, term2_inh, NULL);
        }

        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "term2"))
        {
            // Rule 69
            // <term2> ---- <op2> <factor> <term2>
            if(parseNode->child->tag == 1)
            {
                astNode *op2_syn = createAST(parseNode->child, NULL, NULL);
                astNode *factor_node = createAST(parseNode->child->sibling, NULL, NULL);
                arrASTnodes[0] = inh;
                arrASTnodes[1] = factor_node;
                astNode *tmpNode = makeASTnode(op2_syn->node->ele.leafNode->type, arrASTnodes, 2);
                if(inh->node->tag == Internal)
                    tmpNode->node->ele.internalNode->lineNumStart = inh->node->ele.internalNode->lineNumStart;
                else
                    tmpNode->node->ele.internalNode->lineNumStart = inh->node->ele.leafNode->lineNum;
                if(tmpNode->child->sibling->node->tag == Internal)
                    tmpNode->node->ele.internalNode->lineNumEnd = tmpNode->child->sibling->node->ele.internalNode->lineNumEnd;
                else
                    tmpNode->node->ele.internalNode->lineNumEnd = tmpNode->child->sibling->node->ele.leafNode->lineNum;

                astNode *tmpInh = tmpNode;

                astNode *retNode = createAST(parseNode->child->sibling->sibling, tmpInh, NULL);
                return retNode;
            }
            // Rule 70 
            // <term2> ----  ε 
            else
            {
                return inh;
            }
        }

        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "factor"))
        {
            // Rule 71
            // <factor> ----  BO <expression2> BC 
            if(parseNode->child->tag == 2)
                return createAST(parseNode->child->sibling, NULL, NULL);
            
            //Rule 72
            // <factor> --- <var>
            else
                return createAST(parseNode->child, NULL, NULL);
        }
        
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "unaryExprArithmetic"))
        {
            //Rule 73
            //<unaryExprArithmetic> ---- <op1> <myOptions>
            // unaryExprArithmetic.node = makeNode(op1.syn, myOptions.node)
            astNode *op1_syn = createAST(parseNode->child, NULL, NULL);
            astNode *myOpt_node = createAST(parseNode->child->sibling, NULL, NULL);
            arrASTnodes[0] = myOpt_node;
            astNode *unaryExpArithNode = makeASTnode(op1_syn->node->ele.leafNode->type, arrASTnodes, 1);
                
            unaryExpArithNode->node->ele.internalNode->lineNumStart = op1_syn->node->ele.leafNode->lineNum;
            if(myOpt_node->node->tag == Internal)
                unaryExpArithNode->node->ele.internalNode->lineNumEnd = myOpt_node->node->ele.internalNode->lineNumEnd;
            else
                unaryExpArithNode->node->ele.internalNode->lineNumEnd = myOpt_node->node->ele.leafNode->lineNum;
                
            return unaryExpArithNode;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "myOptions"))
        {
            //Rule 74
            // <myOptions> ---- <var>
            if(parseNode->child->tag == 1)
            {
                return createAST(parseNode->child, NULL, NULL);
            }
            // Rule 75
            // <myOptions> ---- BO <arithmeticExprBoolnt> BC
            else
            {
                return createAST(parseNode->child->sibling, NULL, NULL);
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "arithmeticExprBoolnt"))
        {
            //Rule 76
            //<arithmeticExprBoolnt> ---- <termBoolnt> <arithmeticExpr2Boolnt>
            astNode *termBool_node = createAST(parseNode->child, NULL, NULL);
            astNode *expr2Bool_inh = termBool_node;
            return createAST(parseNode->child->sibling, expr2Bool_inh, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "arithmeticExpr2Boolnt"))
        {
            //Rule 77
            //<arithmeticExpr2Boolnt> ---- <op1> <termBoolnt> <arithmeticExpr2Boolnt>
            if(parseNode->child->tag == 1)
            {
                astNode *op1_syn = createAST(parseNode->child, NULL, NULL);
                astNode *termBoolnt_node = createAST(parseNode->child->sibling, NULL, NULL);
                arrASTnodes[0] = inh;
                arrASTnodes[1] = termBoolnt_node;
                astNode *tmpNode = makeASTnode(op1_syn->node->ele.leafNode->type, arrASTnodes, 2);
                if(inh->node->tag == Internal)
                    tmpNode->node->ele.internalNode->lineNumStart = inh->node->ele.internalNode->lineNumStart;
                else
                    tmpNode->node->ele.internalNode->lineNumStart = inh->node->ele.leafNode->lineNum;
                if(tmpNode->child->sibling->node->tag == Internal)
                    tmpNode->node->ele.internalNode->lineNumEnd = tmpNode->child->sibling->node->ele.internalNode->lineNumEnd;
                else
                    tmpNode->node->ele.internalNode->lineNumEnd = tmpNode->child->sibling->node->ele.leafNode->lineNum;
                
                astNode *tmpInh = tmpNode;

                astNode *retNode = createAST(parseNode->child->sibling->sibling, tmpInh, NULL);
                return retNode;
            }
            //Rule 78
            //<arithmeticExpr2Boolnt> ---- ε
            else
            {
                return inh;
            }
            
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "termBoolnt"))
        {
            //Rule 79
            //<termBoolnt> ---- <factorBoolnt> <term2Boolnt>
            astNode *factBoolnt_node = createAST(parseNode->child, NULL, NULL);
            astNode *term2Boolnt_inh = factBoolnt_node;
            return createAST(parseNode->child->sibling, term2Boolnt_inh, NULL);   
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "term2Boolnt"))
        {
            //Rule 80
            // <term2Boolnt> ---- <op2> <factorBoolnt> <term2Boolnt> 
            // term2Boolnt.node = makenode(‘op2.syn’, term2BooInt.inh, factorBoolnt.node)
            if(parseNode->child->tag == 1)
            {
                astNode *op2_syn = createAST(parseNode->child,NULL, NULL);
                astNode *factorBoolInt = createAST(parseNode->child->sibling, NULL, NULL);
                arrASTnodes[0] = inh;
                arrASTnodes[1] = factorBoolInt;
                astNode *tmpNode = makeASTnode(op2_syn->node->ele.leafNode->type, arrASTnodes, 2);
                if(inh->node->tag == Internal)
                    tmpNode->node->ele.internalNode->lineNumStart = inh->node->ele.internalNode->lineNumStart;
                else
                    tmpNode->node->ele.internalNode->lineNumStart = inh->node->ele.leafNode->lineNum;
                if(tmpNode->child->sibling->node->tag == Internal)
                    tmpNode->node->ele.internalNode->lineNumEnd = tmpNode->child->sibling->node->ele.internalNode->lineNumEnd;
                else
                    tmpNode->node->ele.internalNode->lineNumEnd = tmpNode->child->sibling->node->ele.leafNode->lineNum;
                
                astNode *tmpInh = tmpNode;
                astNode *retNode = createAST(parseNode->child->sibling->sibling, tmpInh, NULL);
                return retNode;
            }
            
            //Rule 81
            // <term2Boolnt> ----  ε
            else
            {
                return inh;
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "factorBoolnt"))
        {
            //Rule 82
            //<factorBoolnt> ---- BO <arithmeticExprBoolnt> BC
            if(parseNode->child->tag == 2)
            {
                return createAST(parseNode->child->sibling, NULL, NULL);
            }
            //Rule 83
            //<factorBoolnt> ---- <var>
            else
            {
                return createAST(parseNode->child, NULL, NULL);
            }
            
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "op1") || !strcmp(parseNode->ele.nonleaf.nt.str, "op2") || !strcmp(parseNode->ele.nonleaf.nt.str, "relationalOp") || !strcmp(parseNode->ele.nonleaf.nt.str, "logicalOp") || !strcmp(parseNode->ele.nonleaf.nt.str, "boolConst") || !strcmp(parseNode->ele.nonleaf.nt.str, "value"))
        {
            //Rule 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 103, 104, 105
            //<op1> ---- PLUS
            //<op1> ---- MINUS
            //<op2> ---- MUL
            //<op2> ---- DIV
            //<relationalOp>  ---- LT
            //<relationalOp>  ---- LE
            //<relationalOp>  ---- GT
            //<relationalOp>  ---- GE
            //<relationalOp>  ---- EQ
            //<relationalOp>  ---- NE
            //<logicalOp> ---- AND
            //<logicalOp> ---- OR
            //<boolConst> ---- TRUE
            //<boolConst> ---- FALSE
            return createAST(parseNode->child, NULL, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "declareStmt"))
        {
            // Rule 98
            // <declareStmt> ---- DECLARE <idList> COLON <dataType> SEMICOL

            astNode *idList = createAST(parseNode->child->sibling, NULL, NULL);
            astNode *dataType = createAST(parseNode->child->sibling->sibling->sibling, NULL, NULL);
            
            arrASTnodes[0] = idList;
            arrASTnodes[1] = dataType;
            astNode *declareNode = makeASTnode("DECLARE", arrASTnodes, 2);

            declareNode->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
            declareNode->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
            return declareNode;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "conditionalStmt"))
        {
            // Rule 99
            // <conditionalStmt> ---- SWITCH BO ID BC START <caseStmt><default> END
            astNode *ID_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
            astNode *caseStmt_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
            astNode *default_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
            arrASTnodes[0] = ID_node; 
            arrASTnodes[1] = caseStmt_node;    
            arrASTnodes[2] = default_node;   
            astNode * condNode = makeASTnode("SWITCH", arrASTnodes, 3);
            condNode->node->ele.internalNode->lineNumStart = parseNode->child->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
            condNode->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
            return condNode;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "caseStmt"))
        {
            // Rule 100
            // <caseStmt> ---- CASE <value> COLON <statements> BREAK SEMICOL <caseStmts>
            
            astNode *value = createAST(parseNode->child->sibling, NULL, NULL);
            astNode *stmts_syn = NULL;
            createAST(parseNode->child->sibling->sibling->sibling, NULL, &stmts_syn);
            astNode *caseStmtsInh=NULL,*caseStmtsSyn=NULL;
            arrASTnodes[0] = value;
            arrASTnodes[1] = stmts_syn;
            caseStmtsInh = makeASTnode("CASE", arrASTnodes, 2);
            caseStmtsInh->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
            caseStmtsInh->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
            createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling, caseStmtsInh, &caseStmtsSyn);
            return caseStmtsSyn;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "caseStmts"))
        {
            if(!strcmp(parseNode->child->ele.leaf.tkn.token,"CASE"))
            {
                
                //Rule 101
                // <caseStmts> ---- CASE <value> COLON <statements> BREAK SEMICOL <caseStmts> 

                astNode *value_node = createAST(parseNode->child->sibling, NULL, NULL);
                astNode *stmts_syn = NULL;
                createAST(parseNode->child->sibling->sibling->sibling, NULL, &stmts_syn);
                arrASTnodes[0] = value_node;
                arrASTnodes[1] = stmts_syn;
                astNode *tmp = makeASTnode("CASE",arrASTnodes,2);
                tmp->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
                tmp->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
                astNode *caseStmts_inh = concatenate(inh, tmp);
                astNode *caseStmts_syn = NULL;
                createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling, caseStmts_inh, &caseStmts_syn);
                *syn = caseStmts_syn;
                return NULL;
            }
            else
            {
                // Rule 102
                // <caseStmts> ----  ε
                *syn = inh;
                return NULL;
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "default"))
        {
            //Rule 106
            //<default> ---- DEFAULT COLON <statements> BREAK SEMICOL
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "DEFAULT"))
            {
                astNode *stmts_syn = NULL;
                createAST(parseNode->child->sibling->sibling, NULL, &stmts_syn);
                arrASTnodes[0] = stmts_syn;
                astNode *defNode = makeASTnode("DEFAULT", arrASTnodes, 1);
                defNode->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
                defNode->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
                return defNode;
            }
            //Rule 107
            //<default> ----  ε
            else
            {
                return NULL;
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "iterativeStmt"))
        {
            //Rule 108
            //<iterativeStmt> ---- FOR BO ID IN <range2> BC START <statements> END
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "FOR"))
            {
                astNode *ID_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                astNode *range2_node = createAST(parseNode->child->sibling->sibling->sibling->sibling, NULL, NULL);
                astNode *stmts_syn = NULL;
                createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->sibling, NULL, &stmts_syn);

                arrASTnodes[0] = ID_node;
                arrASTnodes[1] = range2_node;
                arrASTnodes[2] = stmts_syn;

                astNode *itrNode = makeASTnode("FOR", arrASTnodes, 3);
                itrNode->node->ele.internalNode->lineNumStart = parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
                itrNode->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
                return itrNode;
            }
            
            // Rule 109
            // <iterativeStmt> ---- WHILE BO <expression2> BC START <statements> END
            else
            {
                astNode *expr2_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                astNode *stmts_syn = NULL;
                createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling, NULL, &stmts_syn);
                
                arrASTnodes[0] = expr2_node;
                arrASTnodes[1] = stmts_syn;

                astNode *whileNode = makeASTnode("WHILE", arrASTnodes, 2);
                whileNode->node->ele.internalNode->lineNumStart = parseNode->child->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
                whileNode->node->ele.internalNode->lineNumEnd = parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->ele.leaf.tkn.lineNum;
                return whileNode;
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "range1"))
        {
            // Rule 110
            // <range1> ---- NUM RANGEOP <index>
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "NUM"))
            {
                astNode *NUM_node = createAST(parseNode->child, NULL, NULL);
                astNode *index_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                arrASTnodes[0] = NUM_node;
                arrASTnodes[1] = index_node;
                astNode *range1 = makeASTnode("RANGEOP", arrASTnodes, 2);
                range1->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
                range1->node->ele.internalNode->lineNumEnd = range1->child->sibling->node->ele.leafNode->lineNum;
                return range1;
            }
            // Rule 111
            // <range1> ---- ID RANGEOP <index>
            else
            {
                astNode *ID_node = createAST(parseNode->child, NULL, NULL);
                astNode *index_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                arrASTnodes[0] = ID_node;
                arrASTnodes[1] = index_node;
                astNode *range1 = makeASTnode("RANGEOP", arrASTnodes, 2);
                range1->node->ele.internalNode->lineNumStart = parseNode->child->ele.leaf.tkn.lineNum;
                range1->node->ele.internalNode->lineNumEnd = index_node->node->ele.leafNode->lineNum;
                return range1;
            }            
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "range2"))
        {
            // Rule 112
            // <range2> ---- NUM RANGEOP NUM
            astNode *NUM0_node = createAST(parseNode->child, NULL, NULL);
            astNode *NUM1_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
            arrASTnodes[0]= NUM0_node;
            arrASTnodes[1]= NUM1_node;
            astNode * range2 = makeASTnode("RANGEOP", arrASTnodes,2);
            range2->node->ele.internalNode->lineNumStart = NUM0_node->node->ele.leafNode->lineNum;
            range2->node->ele.internalNode->lineNumEnd = NUM1_node->node->ele.leafNode->lineNum;
            return range2;
        }
    }    
    //Terminal
    else
    {
        // printf("Terminal - %s\n",parseNode->ele.leaf.tkn.lexeme);
        return makeLeafNode(parseNode);
    }
}

//Function to print the AST
void printAST(astNode * ast)
{
    if(ast == NULL)
        return;
    
    char label[30];
    char node[30];
    char linenumber[30];
    char type[30];
    char value[30];

    if(ast->node->tag == Leaf)
    {
        strcpy(label,ast->node->ele.leafNode->lexeme);
        strcpy(node,"LEAF\0 ");
        sprintf(linenumber,"%d ",ast->node->ele.leafNode->lineNum);
        strcpy(type,ast->node->ele.leafNode->type);
        if(!strcmp(ast->node->ele.leafNode->type,"NUM") || !strcmp(ast->node->ele.leafNode->type,"RNUM"))
            strcpy(value,ast->node->ele.leafNode->lexeme);
        else 
            strcpy(value," -------  ");
    }    
    else
    {
        strcpy(label,ast->node->ele.internalNode->label);
        strcpy(node,"INTERNAL\0 ");
        sprintf(linenumber,"%d to %d ",ast->node->ele.internalNode->lineNumStart, ast->node->ele.internalNode->lineNumEnd);
        strcpy(type," ------- ");
        strcpy(value," ------- ");
    }    
    
    printf("|| %20s || %10s || %10s || %10s || %10s || \n", label, node, linenumber, type, value);

    astNode * tmp = ast->child;
    while(tmp != NULL)
    {
        printAST(tmp);
        tmp = tmp->sibling;
    }
}