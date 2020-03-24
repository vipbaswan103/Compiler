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

// simply same as astSyblingInsert function
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

astNode * createAST(TreeNode *parseNode, astNode *inh, astNode **syn)
{
    //Non-Terminal
    if(parseNode->tag == 1)
    {
        printf("Non Terminal - %s\n",parseNode->ele.nonleaf.nt.str);

        astNode* arrASTnodes[4];

        // Rule 1
        // <program> -> <moduleDeclarations> <otherModules> <driverModule> <otherModules>
        if(!strcmp(parseNode->ele.nonleaf.nt.str,"program"))
        {
            
            // printf("The node is %s", parseNode->ele.nonleaf.nt.str);
            // printf("The child is %s", parseNode->child->ele.nonleaf.nt.str);
            // printf("The node is %s", parseNode->child->sibling->ele.nonleaf.nt.str);
            
            astNode *programNode;
            astNode *modDecSyn = NULL;
            
            //recurcively call the function on the children
            //pass the inhereted and synthesised attributes as needed
            createAST(parseNode->child, NULL, &modDecSyn);
            arrASTnodes[0] = modDecSyn; 
            astNode * modDec = makeASTnode("MODULEDEC",arrASTnodes ,1);
            
            astNode *othMod1_syn = NULL, *othMod2_syn = NULL;
            astNode *othMod1_node = NULL, *othMod2_node = NULL;
            createAST(parseNode->child->sibling, NULL, &othMod1_syn);
            astNode *driverMod_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
            createAST(parseNode->child->sibling->sibling->sibling, NULL, &othMod2_syn);
            
            arrASTnodes[0] = othMod1_syn;
            othMod1_node = makeASTnode("MODULES", arrASTnodes, 1);
            arrASTnodes[0] = othMod2_syn;
            othMod2_node = makeASTnode("MODULES", arrASTnodes, 1);
            
            // we have all 4 children to create a program AST node
            arrASTnodes[0] = modDec;
            arrASTnodes[1] = othMod1_node;
            arrASTnodes[2] = driverMod_node;
            arrASTnodes[3] = othMod2_node;
            programNode = makeASTnode("PROGRAM", arrASTnodes, 4);
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
            astNode *modDef_node = createAST(parseNode->child->sibling->sibling->sibling->sibling, NULL, NULL);
            arrASTnodes[0] = modDef_node;
            return makeASTnode("DRIVER", arrASTnodes, 1);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "module"))
        {
            astNode *ID_node = NULL, *inpList_node = NULL, *ret_node = NULL, *modDef_node = NULL, *mod_node = NULL;
            ID_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
            inpList_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
            ret_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
            modDef_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling->sibling, NULL, NULL);

            arrASTnodes[0] = ID_node;
            arrASTnodes[1] = inpList_node;
            arrASTnodes[2] = ret_node;
            arrASTnodes[3] = modDef_node;
            return makeASTnode("MODULE", arrASTnodes, 4);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "ret"))
        {
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "RETURNS"))
                return createAST(parseNode->child->sibling->sibling, NULL, NULL);
            else 
                return NULL;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "input_plist"))
        {
            astNode *input2Inh = NULL, *input2Syn = NULL;
            astNode *IDNode = createAST(parseNode->child, NULL, NULL);
            astNode *datatypeNode = createAST(parseNode->child->sibling->sibling, NULL, NULL);
            input2Inh = concatenate(IDNode, datatypeNode);
            createAST(parseNode->child->sibling->sibling->sibling, input2Inh, &input2Syn);
            arrASTnodes[0] = input2Syn;
            astNode *inputNode = makeASTnode("INPUT_LIST", arrASTnodes, 1);
            return inputNode;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "input_plist2"))
        {
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
            else
            {
                *syn = inh;
            }
            return NULL;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "output_plist"))
        {
            astNode * ID_node = createAST(parseNode->child, NULL, NULL);
            astNode * dataType_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
            astNode *outList2_inh = NULL, *outList2_syn = NULL;
            outList2_inh = concatenate(ID_node, dataType_node);
            createAST(parseNode->child->sibling->sibling->sibling, outList2_inh, &outList2_syn);
            arrASTnodes[0] = outList2_syn;
            astNode *output_node = makeASTnode("OUTPUT_LIST", arrASTnodes, 1);
            
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "output_plist2"))
        {
            
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
            else 
                *syn = inh;
                
            return NULL;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "dataType"))
        {
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "INTEGER") || !strcmp(parseNode->child->ele.leaf.tkn.token, "REAL") || !strcmp(parseNode->child->ele.leaf.tkn.token, "BOOLEAN"))
            {
                return createAST(parseNode->child, NULL, NULL);
            }
            else
            {
                astNode *range1_node = NULL, *type_node = NULL;
                range1_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                type_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
                arrASTnodes[0] = range1_node;
                arrASTnodes[1] = type_node;
                return makeASTnode("ARRAY", arrASTnodes, 2);
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "type"))
        {
            return createAST(parseNode->child, NULL, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "moduleDef"))
        {
            astNode *stmts_inh = NULL, *stmts_syn = NULL, *stmts_node = NULL;
            createAST(parseNode->child->sibling, stmts_inh, &stmts_syn);
            arrASTnodes[0] = stmts_syn;
            return makeASTnode("MODULEDEF", arrASTnodes, 1);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "statements"))
        {
            if(!strcmp(parseNode->child->ele.nonleaf.nt.str, "statement"))
            {
                astNode *stmt_node = createAST(parseNode->child, NULL, NULL);
                astNode *stmts2_inh = NULL, *stmts2_syn = NULL;
                stmts2_inh = concatenate(inh, stmt_node);
                createAST(parseNode->child->sibling, stmts2_inh, &stmts2_syn);
                *syn = stmts2_syn;
            }
            else
            {
                *syn = inh;
            }
            return NULL;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "statement"))
        {
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
                return makeASTnode("GET_VAL", arrASTnodes, 1);
            }
            // Rule 33
            // <ioStmt> ---- PRINT BO <varAndBool> BC SEMICOL
            else
            {
                astNode *varBool_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                arrASTnodes[0] = varBool_node;
                return makeASTnode("PRINT", arrASTnodes, 1);
            }
            
        }

        // Rule 34 and 35
        // <varAndBool> ---- <var>
        // <varAndBool> ---- <boolConst>
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "varAndBool"))
        {
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
                return makeASTnode("ID_ARR", arrASTnodes, 2);
            }
            // Rule 37 and 38
            // <var> ---- NUM 
            // <var> ---- RNUM
            else 
            {
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
            return createAST(parseNode->child, NULL, NULL);
        }

        //Rule 43
        //<assignmentStmt> ---- ID <whichStmt>
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "assignmentStmt"))
        {
            astNode *whichStmt_inh = NULL, *ID_node = NULL;
            ID_node = createAST(parseNode->child, NULL, NULL);
            whichStmt_inh = ID_node;
            return createAST(parseNode->child->sibling, whichStmt_inh, NULL);
        }

        //Rule 44 and 45
        // <whichStmt> ---- <lvalueIDStmt> 
        // <whichStmt> ---- <lvalueARRStmt>
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "whichStmt"))
        {
            astNode * child_inh = inh;
            return createAST(parseNode->child, child_inh, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "lvalueIDStmt"))
        {
            astNode *assignNode = createAST(parseNode->child, NULL, NULL);
            astNode *exprNode = createAST(parseNode->child->sibling, NULL, NULL);
            arrASTnodes[0] = inh;
            arrASTnodes[1] = exprNode;
            return makeASTnode("ASSIGNOP", arrASTnodes, 2);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "lvalueARRStmt"))
        {
            astNode *ind_node = createAST(parseNode->child->sibling, NULL, NULL);
            astNode * expr_node = createAST(parseNode->child->sibling->sibling->sibling->sibling, NULL, NULL);

            arrASTnodes[0] = inh;
            arrASTnodes[1] = ind_node;
            arrASTnodes[2] = expr_node;
            return makeASTnode("ASSIGNOPARR", arrASTnodes, 3);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "index"))
        {
            return createAST(parseNode->child, NULL, NULL);   
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "moduleReuseStmt"))
        {
            astNode *optional_node = createAST(parseNode->child, NULL, NULL);
            astNode *ID_node = createAST(parseNode->child->sibling->sibling->sibling, NULL, NULL);
            astNode *idList_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
            
            arrASTnodes[0] = ID_node;
            arrASTnodes[1] = idList_node;
            astNode * tmp = makeASTnode("MODULECALL", arrASTnodes, 2);
            if(optional_node == NULL)
            {
                return tmp;
            }
            else
            {
                arrASTnodes[0] = optional_node;
                arrASTnodes[1] = tmp;
                return makeASTnode("MODULEASSIGNOP", arrASTnodes, 2);
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "optional"))
        {
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "SQBO"))
                return createAST(parseNode->child->sibling, NULL, NULL);
            else
                return NULL;            
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "idList"))
        {
            astNode *idList2Inh,*idList2Syn = NULL;
            astNode *IDNode = createAST(parseNode->child, NULL, NULL);
            idList2Inh = concatenate(inh, IDNode);
            createAST(parseNode->child->sibling, idList2Inh, &idList2Syn);
            arrASTnodes[0] = idList2Syn;
            return makeASTnode("ID_LIST", arrASTnodes, 1);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "idList2"))
        {
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "COMMA"))
            {
                astNode *ID_node = createAST(parseNode->child->sibling, NULL, NULL);
                astNode *idList22_inh = concatenate(inh, ID_node);
                astNode *idList22_syn = NULL;
                createAST(parseNode->child->sibling->sibling, idList22_inh, &idList22_syn);
                *syn = idList22_syn;
            }
            else
            {
                *syn = inh;
            }
            return NULL;
        }
        
        // Rule 56 and 57 boh are automatically considered 
        // <expression> ---- <expression2>
        // <expression> ----  <unaryExprArithmetic> 
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "expression"))
        {
            return createAST(parseNode->child, NULL, NULL);
        }
        
        // Rule 58
        // <expression2> ---- <logicalExpr> <expression3>
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "expression2"))
        {
            astNode *exp3Inh = NULL;
            astNode *logExpOrboolConst = createAST(parseNode->child, NULL, NULL);
            exp3Inh = logExpOrboolConst;
            return createAST(parseNode->child->sibling, exp3Inh, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "expression3"))
        {
            if(!strcmp(parseNode->child->ele.nonleaf.nt.str, "logicalOp"))
            {
                astNode *logOp_syn = createAST(parseNode->child, NULL, NULL); 
                astNode *expr2_node = createAST(parseNode->child->sibling, NULL, NULL);
                
                arrASTnodes[0] = inh;
                arrASTnodes[1] = expr2_node;
                
                return makeASTnode(logOp_syn->node->ele.leafNode->type, arrASTnodes, 2);
            }
            else
            {
                return inh;
            }
        }
        //Rule 62
        // <logicalExpr> ---- <arithmeticExpr> <logicalExpr2> 
        // logicalExpr2.inh = arithmeticExpr.node
        // logicalExpr.node = logicalExpr2.node

        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "logicalExpr"))
        {
         
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
                return makeASTnode(relOp->node->ele.leafNode->type, arrASTnodes, 2);
            }
            // Rule  64
            // <logicalExpr2> ---- ε
            else 
            {
                return inh;       
            } 
        }

        // Rule 65
        // <arithmeticExpr> ---- <term> <arithmeticExpr2>	
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "arithmeticExpr"))
        {
            astNode *arithExp2Inh = NULL;
            astNode *term = createAST(parseNode->child, NULL, NULL);
            arithExp2Inh = term;
            return createAST(parseNode->child->sibling, arithExp2Inh, NULL);
        }

        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "arithmeticExpr2"))
        {
            // Rule 66
            // <arithmeticExpr2> ---- <op1> <arithmeticExpr>
            if(parseNode->child->tag == 1)
            {
                astNode *op1 = createAST(parseNode->child, NULL, NULL);
                astNode *arithExp = createAST(parseNode->child->sibling, NULL, NULL);
                
                arrASTnodes[0] = inh;
                arrASTnodes[1] = arithExp;
                return makeASTnode(op1->node->ele.leafNode->type, arrASTnodes, 2);
            }
            // Rule 67
            // <arithmeticExpr2> ---- ε 
            else
            {
                return inh;
            }
        }

        //Rule 68
        // <term> ---- <factor> <term2>
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "term"))
        {
            astNode *fact_node = createAST(parseNode->child, NULL, NULL);
            astNode *term2_inh = fact_node;
            return createAST(parseNode->child->sibling, term2_inh, NULL);
        }

        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "term2"))
        {
            // Rule 69
            // <term2> ---- <op2> <term>
            if(parseNode->child->tag == 1)
            {
                astNode *op2_syn = createAST(parseNode->child, NULL, NULL);
                astNode *term_node = createAST(parseNode->child->sibling, NULL, NULL);
                arrASTnodes[0] = inh;
                arrASTnodes[1] = term_node;
                return makeASTnode(op2_syn->node->ele.leafNode->type, arrASTnodes, 2);
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
        
        //Rule 73
        //<unaryExprArithmetic> ---- <op1> <myOptions>
        // unaryExprArithmetic.node = makeNode(op1.syn, myOptions.node)
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "unaryExprArithmetic"))
        {
            astNode *op1_syn = createAST(parseNode->child, NULL, NULL);
            astNode *myOpt_node = createAST(parseNode->child->sibling, NULL, NULL);
            arrASTnodes[0] = myOpt_node;
            return makeASTnode(op1_syn->node->ele.leafNode->type, arrASTnodes, 1);
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
            astNode *termBool_node = createAST(parseNode->child, NULL, NULL);
            astNode *expr2Bool_inh = termBool_node;
            return createAST(parseNode->child->sibling, expr2Bool_inh, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "arithmeticExpr2Boolnt"))
        {
            if(parseNode->child->tag == 1)
            {
                astNode *op1_syn = createAST(parseNode->child, NULL, NULL);
                astNode *exprBool_node = createAST(parseNode->child->sibling, NULL, NULL);
                arrASTnodes[0] = inh;
                arrASTnodes[1] = exprBool_node;
                return makeASTnode(op1_syn->node->ele.leafNode->type, arrASTnodes, 2);
            }
            else
            {
                return inh;
            }
            
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "termBoolnt"))
        {
            astNode *factBoolnt_node = createAST(parseNode->child, NULL, NULL);
            astNode *term2Boolnt_inh = factBoolnt_node;
            return createAST(parseNode->child->sibling, term2Boolnt_inh, NULL);   
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "term2Boolnt"))
        {
            //Rule 80
            // <term2Boolnt> ---- <op2> <factorBoolnt> 
            // term2Boolnt.node=  makenode(‘op2.syn’, term2BooInt.inh, factorBoolnt.node)
            if(parseNode->child->tag == 1)
            {
                astNode *op2_syn = createAST(parseNode->child,NULL, NULL);
                astNode *factorBoolInt = createAST(parseNode->child->sibling, NULL, NULL);
                arrASTnodes[0] = inh;
                arrASTnodes[1] = factorBoolInt;
                return makeASTnode(op2_syn->node->ele.leafNode->type, arrASTnodes, 2);
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
            if(parseNode->child->tag == 2)
            {
                astNode *op2_syn = createAST(parseNode->child, NULL, NULL);
                astNode *facBool_node = createAST(parseNode->child->sibling, NULL, NULL);
                arrASTnodes[0] = inh;
                arrASTnodes[1] = facBool_node;
                return makeASTnode(op2_syn->node->ele.leafNode->type, arrASTnodes, 2);
            }
            else
            {
                return createAST(parseNode->child, NULL, NULL);
            }
            
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "op1") || !strcmp(parseNode->ele.nonleaf.nt.str, "op2") || !strcmp(parseNode->ele.nonleaf.nt.str, "relationalOp") || !strcmp(parseNode->ele.nonleaf.nt.str, "logicalOp") || !strcmp(parseNode->ele.nonleaf.nt.str, "boolConst") || !strcmp(parseNode->ele.nonleaf.nt.str, "value"))
        {
            return createAST(parseNode->child, NULL, NULL);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "declareStmt"))
        {
            astNode *idList = createAST(parseNode->child->sibling, NULL, NULL);
            astNode *dataType = createAST(parseNode->child->sibling->sibling->sibling, NULL, NULL);
            
            arrASTnodes[0] = idList;
            arrASTnodes[1] = dataType;
            return makeASTnode("DECLARE", arrASTnodes, 2);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "conditionalStmt"))
        {
            astNode *ID_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
            astNode *caseStmt_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
            astNode *default_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
            arrASTnodes[0] = ID_node;
            arrASTnodes[1] = caseStmt_node;
            arrASTnodes[2] = default_node;
            return makeASTnode("SWITCH", arrASTnodes, 3);
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "caseStmt"))
        {
            astNode *value = createAST(parseNode->child->sibling, NULL, NULL);
            astNode *statements = createAST(parseNode->child->sibling->sibling->sibling, NULL, NULL);
            astNode *caseStmtsInh,*caseStmtsSyn;
            arrASTnodes[0] = value;
            arrASTnodes[1] = statements;
            caseStmtsInh = makeASTnode("CASE", arrASTnodes, 2);
            createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling, caseStmtsInh, &caseStmtsSyn);
            return caseStmtsSyn;
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "caseStmts"))
        {
            if(!strcmp(parseNode->child->ele.leaf.tkn.token,"CASE"))
            {
                astNode *value_node = createAST(parseNode->child->sibling, NULL, NULL);
                astNode *stmts_node = createAST(parseNode->child->sibling->sibling->sibling, NULL, NULL);
                arrASTnodes[0] = value_node;
                arrASTnodes[1] = stmts_node;
                astNode *tmp = makeASTnode("CASE",arrASTnodes,2);
                
                astNode *caseStmts_inh = concatenate(inh, tmp);
                astNode *caseStmts_syn = NULL;
                createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling, caseStmts_inh, &caseStmts_syn);
                *syn = caseStmts_syn;
                return NULL;
            }
            else
            {
                *syn = inh;
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "default"))
        {
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "DEFAULT"))
            {
                astNode *stmts_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                arrASTnodes[0] = stmts_node;
                return makeASTnode("DEFAULT", arrASTnodes, 1);
            }
            else
            {
                return NULL;
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "iterativeStmt"))
        {
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "FOR"))
            {
                astNode *ID_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                astNode *range2_node = createAST(parseNode->child->sibling->sibling->sibling->sibling, NULL, NULL);
                astNode *stmts_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling->sibling->sibling, NULL, NULL);

                arrASTnodes[0] = ID_node;
                arrASTnodes[1] = range2_node;
                arrASTnodes[2] = stmts_node;

                return makeASTnode("FOR", arrASTnodes, 3);
            }
            else
            {
                astNode *expr2_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                astNode *stmts_node = createAST(parseNode->child->sibling->sibling->sibling->sibling->sibling, NULL, NULL);
                
                arrASTnodes[0] = expr2_node;
                arrASTnodes[1] = stmts_node;

                return makeASTnode("WHILE", arrASTnodes, 2);
            }
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "range1"))
        {
            if(!strcmp(parseNode->child->ele.leaf.tkn.token, "NUM"))
            {
                astNode *NUM_node = createAST(parseNode->child, NULL, NULL);
                astNode *ID_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                arrASTnodes[0] = NUM_node;
                arrASTnodes[1] = ID_node;
                return makeASTnode("RANGEOP", arrASTnodes, 2);
            }
            else
            {
                astNode *ID_node = createAST(parseNode->child, NULL, NULL);
                astNode *index_node = createAST(parseNode->child->sibling->sibling, NULL, NULL);
                arrASTnodes[0] = ID_node;
                arrASTnodes[1] = index_node;
                return makeASTnode("RANGEOP", arrASTnodes, 2);
            }            
        }
        else if(!strcmp(parseNode->ele.nonleaf.nt.str, "range2"))
        {
            astNode *NUM0_node = createAST(parseNode->child, NULL, NULL);
            astNode *NUM1_node = createAST(parseNode->child->sibling, NULL, NULL);
            arrASTnodes[0]= NUM0_node;
            arrASTnodes[1]= NUM1_node;
            return makeASTnode("RANGE", arrASTnodes,2);
        }
    }    
    //Terminal
    else
    {
        printf("Terminal - %s\n",parseNode->ele.leaf.tkn.lexeme);
        return makeLeafNode(parseNode);
    }
}

//Function to print the AST

void printAST(astNode * ast, FILE * fp)
{
    if(ast == NULL)
        return;
    if(ast->node->tag == Leaf)
    {
        printf("%s\n", ast->node->ele.leafNode->type);
        fprintf(fp, "%s\n", ast->node->ele.leafNode->type);
    }
    else
    {
        printf("%s\n", ast->node->ele.internalNode->label);
        fprintf(fp, "%s\n", ast->node->ele.internalNode->label);
    }
    astNode * tmp = ast->child;
    while(tmp != NULL)
    {
        printAST(tmp, fp);
        tmp = tmp->sibling;
    }
}