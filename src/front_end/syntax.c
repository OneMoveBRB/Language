#include "../../include/front_end/syntax.h"

#include <stdio.h>
#include <assert.h>

#include "../../List/list.h"

#include "../../include/ast.h"
#include "../../include/front_end/front_end.h"

#define c(x, y) \
    AST_NodeInit(NULL, NULL, NULL, AST_ELEM_TYPE_CONST, x, y)

#define v(x) \
    AST_NodeInit(NULL, NULL, NULL, AST_ELEM_TYPE_VARIABLE, x)

#define ADD_(left, right) \
    AST_NodeInit(NULL, left, right, AST_ELEM_TYPE_OPERATION, AST_ELEM_OPERATION_ADD)

#define SUB_(left, right) \
    AST_NodeInit(NULL, left, right, AST_ELEM_TYPE_OPERATION, AST_ELEM_OPERATION_SUB)

#define MUL_(left, right) \
    AST_NodeInit(NULL, left, right, AST_ELEM_TYPE_OPERATION, AST_ELEM_OPERATION_MUL)

#define DIV_(left, right) \
    AST_NodeInit(NULL, left, right, AST_ELEM_TYPE_OPERATION, AST_ELEM_OPERATION_DIV)

#define OP_(left, right, type) \
    AST_NodeInit(NULL, left, right, AST_ELEM_TYPE_OPERATION, type)

static AST_Node* GetG(List_t* tokens, size_t *idx);
static AST_Node* GetAssignment(List_t* tokens, size_t *idx);
static AST_Node* GetExpression(List_t* tokens, size_t *idx);
static AST_Node* GetLogicalOr(List_t* tokens, size_t *idx);
static AST_Node* GetLogicalAnd(List_t* tokens, size_t *idx);
static AST_Node* GetEquality(List_t* tokens, size_t *idx);
static AST_Node* GetComparison(List_t* tokens, size_t *idx);
static AST_Node* GetTerm(List_t* tokens, size_t *idx);
static AST_Node* GetFactor(List_t* tokens, size_t *idx);
static AST_Node* GetPrimary(List_t* tokens, size_t *idx);
static AST_Node* GetIdentifier(List_t* tokens, size_t *idx);
static AST_Node* GetNumber(List_t* tokens, size_t* idx);

AST* SyntaxAnalysis(List_t* tokens) {
    assert( tokens != NULL );

    AST* ast = AST_Init();

    size_t idx = ListFront(tokens);
    ast->root = GetG(tokens, &idx);
    fprintf(stderr, "%p\n", ast->root);
    ast->root->parent = NULL;

    return ast;
}

static AST_Node* GetG(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* node = GetAssignment(tokens, idx);

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type == TOKEN_TYPE_END) {
        return node;
    }

    return NULL;
}

static AST_Node* GetAssignment(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    if ((((Token*)ListGet(tokens, *idx))->type == TOKEN_TYPE_VARIABLE  &&
        ((Token*)ListGet(tokens, ListNext(tokens, *idx)))->type == TOKEN_TYPE_ASSIGNMENT) == 0) {
        return NULL;
    };

    AST_Node* node1 = GetIdentifier(tokens, idx);
    AST_Node* node2 = NULL;
    Token*    token = NULL;

    while (1) {
        token = (Token*)ListGet(tokens, *idx);
        // fprintf(stderr, "Code: %zu\n", token->type);
        if (token->type != TOKEN_TYPE_ASSIGNMENT) {
            assert(0); //FIXME - error handler
        }
        *idx = ListNext(tokens, *idx);

        if ((((Token*)ListGet(tokens, *idx))->type == TOKEN_TYPE_VARIABLE  &&
            ((Token*)ListGet(tokens, ListNext(tokens, *idx)))->type == TOKEN_TYPE_ASSIGNMENT) == 1) {

            node2 = GetIdentifier(tokens, idx);
            node1 = OP_(node1, node2, AST_ELEM_OPERATION_ASSIGNMENT);

        } else {
            node2 = GetExpression(tokens, idx);
            node1 = OP_(node1, node2, AST_ELEM_OPERATION_ASSIGNMENT);
            break;
        }
    }

    if (((Token*)ListGet(tokens, *idx))->type == TOKEN_TYPE_SEMICOLON) {
        *idx = ListNext(tokens, *idx);
        return node1;
    }

    assert(0); //FIXME - error handler
    return NULL;
}

static AST_Node* GetExpression(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    return GetLogicalOr(tokens, idx);
}

static AST_Node* GetLogicalOr(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* node1 = GetLogicalAnd(tokens, idx);

    Token* token = (Token*)ListGet(tokens, *idx);
    while (token->type == TOKEN_TYPE_LOR) {
        *idx = ListNext(tokens, *idx);

        AST_Node* node2 = GetLogicalAnd(tokens, idx);

        node1 = OP_(node1, node2, AST_ELEM_OPERATION_LOR);

        token = (Token*)ListGet(tokens, *idx);
    }

    assert(node1 != NULL);
    return node1;
}

static AST_Node* GetLogicalAnd(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* node1 = GetEquality(tokens, idx);

    Token* token = (Token*)ListGet(tokens, *idx);
    while (token->type == TOKEN_TYPE_LAND) {
        *idx = ListNext(tokens, *idx);

        AST_Node* node2 = GetEquality(tokens, idx);

        node1 = OP_(node1, node2, AST_ELEM_OPERATION_LAND);

        token = (Token*)ListGet(tokens, *idx);
    }

    assert(node1 != NULL);
    return node1;
}

static AST_Node* GetEquality(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* node1 = GetComparison(tokens, idx);

    Token* token = (Token*)ListGet(tokens, *idx);
    while (token->type == TOKEN_TYPE_BIN_EE || token->type == TOKEN_TYPE_BIN_NE) {
        *idx = ListNext(tokens, *idx);

        AST_Node* node2 = GetComparison(tokens, idx);

        if (token->type == TOKEN_TYPE_BIN_EE) {
            node1 = OP_(node1, node2, AST_ELEM_OPERATION_EE);
        } else {
            node1 = OP_(node1, node2, AST_ELEM_OPERATION_NE);
        }

        token = (Token*)ListGet(tokens, *idx);
    }

    assert(node1 != NULL);
    return node1;
}

static AST_Node* GetComparison(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* node1 = GetTerm(tokens, idx);

    Token* token = (Token*)ListGet(tokens, *idx);
    while (   token->type == TOKEN_TYPE_BIN_LT || token->type == TOKEN_TYPE_BIN_GT 
           || token->type == TOKEN_TYPE_BIN_LE || token->type == TOKEN_TYPE_BIN_GE ) {

        *idx = ListNext(tokens, *idx);

        AST_Node* node2 = GetTerm(tokens, idx);

        if (token->type == TOKEN_TYPE_BIN_LT) {
            node1 = OP_(node1, node2, AST_ELEM_OPERATION_LT);
        } else if (token->type == TOKEN_TYPE_BIN_GT) {
            node1 = OP_(node1, node2, AST_ELEM_OPERATION_GT);
        } else if (token->type == TOKEN_TYPE_BIN_LE) {
            node1 = OP_(node1, node2, AST_ELEM_OPERATION_LE);
        } else {
            node1 = OP_(node1, node2, AST_ELEM_OPERATION_GE);
        }

        token = (Token*)ListGet(tokens, *idx);
    }

    assert(node1 != NULL);
    return node1;
}

static AST_Node* GetTerm(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* node1 = GetFactor(tokens, idx);

    Token* token = (Token*)ListGet(tokens, *idx);
    while (token->type == TOKEN_TYPE_BIN_ADD || token->type == TOKEN_TYPE_BIN_SUB) {
        *idx = ListNext(tokens, *idx);

        AST_Node* node2 = GetFactor(tokens, idx);

        if (token->type == TOKEN_TYPE_BIN_ADD) {
            node1 = ADD_(node1, node2);
        } else {
            node1 = SUB_(node1, node2);
        }

        token = (Token*)ListGet(tokens, *idx);
    }

    assert(node1 != NULL);
    return node1;
}

static AST_Node* GetFactor(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* node1 = GetPrimary(tokens, idx);

    Token* token = (Token*)ListGet(tokens, *idx);
    while (token->type == TOKEN_TYPE_BIN_MUL || token->type == TOKEN_TYPE_BIN_DIV) {
        *idx = ListNext(tokens, *idx);

        AST_Node* node2 = GetPrimary(tokens, idx);

        if (token->type == TOKEN_TYPE_BIN_MUL) {
            node1 = MUL_(node1, node2);
        } else {
            node1 = DIV_(node1, node2);
        }

        token = (Token*)ListGet(tokens, *idx);
    }

    assert(node1 != NULL);
    return node1;
}

static AST_Node* GetPrimary(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* node = NULL;
    // AST_Node* node = GetFuncCall(tokens, idx);
    // if (node != NULL) {
    //     return node;
    // }

    if ((node = GetIdentifier(tokens, idx)) != NULL) {
        return node;
    }

    if ((node = GetNumber(tokens, idx)) != NULL) {
        return node;
    }

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type == TOKEN_TYPE_BOOL_TRUE) {
        *idx = ListNext(tokens, *idx);
        return c(CONST_TYPE_INT, 1);
    }
    
    if (token->type == TOKEN_TYPE_BOOL_FALSE) {
        *idx = ListNext(tokens, *idx);
        return c(CONST_TYPE_INT, 0);
    }

    if (token->type == TOKEN_TYPE_ROUND_BRACKET_OPEN) {
        *idx = ListNext(tokens, *idx);
        node = GetExpression(tokens, idx);
    }

    token = (Token*)ListGet(tokens, *idx);
    if (token->type == TOKEN_TYPE_ROUND_BRACKET_CLOSE) {
        *idx = ListNext(tokens, *idx);
    }

    fprintf(stderr, "Code: %d\n", token->type);
    assert(node != NULL);
    return node;
}

// static AST_Node* GetArguments(List_t* tokens, size_t *idx) {}

static AST_Node* GetIdentifier(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    size_t i = *idx;

    Token* token = (Token*)ListGet(tokens, i);
    if (token->type != TOKEN_TYPE_VARIABLE) {
        return NULL;
    }

    *idx = ListNext(tokens, i);

    return v(token->data.variable);
}

static AST_Node* GetNumber(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    size_t i = *idx;

    Token* token = (Token*)ListGet(tokens, i);
    if (token->type != TOKEN_TYPE_CONST) {
        return NULL;
    }

    *idx = ListNext(tokens, i);

    switch (token->data.constant.type) {
    case CONST_TYPE_SHORT:
        return c(CONST_TYPE_SHORT, token->data.constant.data.short_const);
    
    case CONST_TYPE_INT:
        return c(CONST_TYPE_INT, token->data.constant.data.int_const);

    case CONST_TYPE_LONG:
        return c(CONST_TYPE_INT, token->data.constant.data.long_const);

    case CONST_TYPE_DOUBLE:
        return c(CONST_TYPE_DOUBLE, token->data.constant.data.double_const);

    case CONST_TYPE_CHAR:
        return c(CONST_TYPE_CHAR, token->data.constant.data.char_const);

    case CONST_TYPE_UNDEFINED:
        assert(0);

    default:
        assert(0);
    }

    assert(0);
    return NULL;
}


// [] - выполняется точно 1 раз
// () - выполняется 0 или 1 раз
// []* - выполняется >= 1 раз
// ()* - выполняется >= 0 раз

/*
Program         := (FuncDec | StatementList) "!END_TOKEN!"

FuncDec         := DataTypes IDENTIFIER "(" (Parameters) ")" [Block | ";"]
Parameters      := DataTypes IDENTIFIER ("," DataTypes IDENTIFIER)*

StatementList   := [Statement]*
Statement       := VarDec
                 | Assignment
                 | IfStatement
                 | WhileStatement
                 | ReturnStatement
                 | Block
                 | ExprStatement


IfStatement     := "if" "(" Expression ")" Block
                    ("else if" "(" Expression ")" Block)*
                    ("else" Block)
WhileStatement  := "while" "(" Expression ")" Block
ReturnStatement := "return" Expression ";"

Block           := "{" StatementList "}"
ExprStatement   := Expression ";"

VarDec          := DataTypes IDENTIFIER ("=" Expression) ";"
Assignment      := [IDENTIFIER "="]* Expression ";"

Expression      := LogicalOr

LogicalOr       := LogicalAnd ( "||" LogicalAnd )*
LogicalAnd      := Equality ( "&&" Equality )*

Equality        := Comparison ( ["==" | "!="] Comparison )*
Comparison      := Term ( ["<" | ">" | "<=" | ">="] Term )*

Term            := Factor ( ['+''-'] Factor )*
Factor          := Primary ( ['*''/'] Primary )*
Primary         := FuncCall | IDENTIFIER | NUM | '('E')' | "true" | "false"

FuncCall        := IDENTIFIER ( "(" (Arguments) ")" )

Arguments       := Expression ("," Expression)*
DataTypes       := "short" | "int" | "long" | "char" | "void" | "float"

IDENTIFIER      := TYPE_VARIABLE
NUM             := TYPE_NUMBER
*/

/*!SECTION
        if (((Token*)ListGet(tokens, *idx))->type == TOKEN_TYPE_VARIABLE) {
            if (((Token*)ListGet(tokens, ListNext(tokens, *idx)))->type == TOKEN_TYPE_ASSIGNMENT) {
                node2 = GetIdentifier(tokens, idx);
                node1 = OP_(node1, node2, AST_ELEM_OPERATION_ASSIGNMENT);

            } else {
                // fprintf(stderr, "yes Code: %zu\n", token->type);
                node2 = GetExpression(tokens, idx);
                node1 = OP_(node1, node2, AST_ELEM_OPERATION_ASSIGNMENT);

                break;
            }
        } else {
            node2 = GetExpression(tokens, idx);
            node1 = OP_(node1, node2, AST_ELEM_OPERATION_ASSIGNMENT);

            break;
            // assert(0); //FIXME - error handler
        }
*/