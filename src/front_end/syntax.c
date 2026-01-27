#include "../../include/front_end/syntax.h"

#include <stdio.h>
#include <assert.h>

#include "../../clibs/List/list.h"

#include "../../include/ast/ast.h"
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

#define DECL_(x) \
    AST_NodeInit(NULL, NULL, NULL, AST_ELEM_TYPE_DECLARATION, x)

#define SENTINEL \
    AST_NodeInit(NULL, NULL, NULL, AST_ELEM_TYPE_OPERATION, AST_ELEM_OPERATION_SENTINEL)

#define OP_(left, right, type) \
    AST_NodeInit(NULL, left, right, AST_ELEM_TYPE_OPERATION, type)

typedef AST_Node* (*SyntaxFunc)(List_t*, size_t*);

static AST_Node* GetG(List_t* tokens, size_t *idx);
static AST_Node* GetGlobal(List_t* tokens, size_t* idx);
static AST_Node* GetFuncDec(List_t* tokens, size_t* idx);
static AST_Node* GetParameters(List_t* tokens, size_t* idx);
static AST_Node* GetStatement(List_t* tokens, size_t* idx);
static AST_Node* GetIfStatement(List_t* tokens, size_t *idx);
static AST_Node* GetWhileStatement(List_t* tokens, size_t *idx);
static AST_Node* GetReturnStatement(List_t* tokens, size_t *idx);
static AST_Node* GetBlock(List_t* tokens, size_t* idx);
static AST_Node* GetExprStatement(List_t* tokens, size_t *idx);
static AST_Node* GetVarDec(List_t* tokens, size_t *idx);
static AST_Node* GetAssignment(List_t* tokens, size_t *idx);
static AST_Node* GetPrint(List_t* tokens, size_t* idx);
static AST_Node* GetExpression(List_t* tokens, size_t *idx);
static AST_Node* GetLogicalOr(List_t* tokens, size_t *idx);
static AST_Node* GetLogicalAnd(List_t* tokens, size_t *idx);
static AST_Node* GetEquality(List_t* tokens, size_t *idx);
static AST_Node* GetComparison(List_t* tokens, size_t *idx);
static AST_Node* GetTerm(List_t* tokens, size_t *idx);
static AST_Node* GetFactor(List_t* tokens, size_t *idx);
static AST_Node* GetPrimary(List_t* tokens, size_t *idx);
static AST_Node* GetInput(List_t* tokens, size_t* idx);
static AST_Node* GetFuncCall(List_t* tokens, size_t *idx);
static AST_Node* GetArguments(List_t* tokens, size_t *idx);
static AST_Node* GetDataType(List_t* tokens, size_t *idx);
static AST_Node* GetIdentifier(List_t* tokens, size_t *idx);
static AST_Node* GetNumber(List_t* tokens, size_t* idx);

AST* SyntaxAnalysis(List_t* tokens) {
    assert( tokens != NULL );

    AST* ast = AST_Init();

    size_t idx = ListFront(tokens);
    ast->root = GetG(tokens, &idx);
    assert(ast->root != NULL); //FIXME - error handler

    ast->root->parent = NULL;

    return ast;
}

static AST_Node* GetG(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* node = SENTINEL;
    AST_Node* statement = node;

    while (( statement->left = GetGlobal(tokens, idx) ) != NULL) {
        statement->right = SENTINEL;

        statement->left->parent = statement;
        statement->right->parent = statement;

        statement = statement->right;
    }

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type == TOKEN_TYPE_END) {
        return node;
    }

    return NULL;
}

static AST_Node* GetGlobal(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* node = GetFuncDec(tokens, idx);
    if (node != NULL) {
        return node;
    }

    node = GetStatement(tokens, idx);
    if (node != NULL) {
        return node;
    }

    return NULL;
}

static AST_Node* GetFuncDec(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    size_t check_bracket_idx = ListNext(tokens, ListNext(tokens, *idx));
    if (((Token*)ListGet(tokens, check_bracket_idx))->type != TOKEN_TYPE_ROUND_BRACKET_OPEN) {
        return NULL;
    }

    AST_Node* data_type = GetDataType(tokens, idx);
    if (data_type == NULL) {
        return NULL;
    }

    AST_Node* identifier = GetIdentifier(tokens, idx);
    if (identifier == NULL) {
        assert(0); //FIXME - error handler
    }

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_ROUND_BRACKET_OPEN) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    AST_Node* parameters = GetParameters(tokens, idx);

    token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_ROUND_BRACKET_CLOSE) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    token = (Token*)ListGet(tokens, *idx);
    if (token->type == TOKEN_TYPE_SEMICOLON) {
        assert(0); //TODO - to symbol_table
    }

    AST_Node* block = GetBlock(tokens, idx);
    if (block == NULL) {
        assert(0); //FIXME - error handler
    }

    data_type->right = identifier;
    identifier->parent = data_type;

    identifier->left = parameters;
    identifier->right = block;

    if (parameters != NULL) {
        parameters->parent = identifier;
    }
    block->parent = identifier;
    
    return data_type;
}

static AST_Node* GetParameters(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* data_type = GetDataType(tokens, idx);
    if (data_type == NULL) {
        return NULL;
    }

    AST_Node* identifier = GetIdentifier(tokens, idx);
    if (identifier == NULL) {
        assert(0); //FIXME - error handler
    }

    data_type->right = identifier;
    identifier->parent = data_type;

    AST_Node* cur_node = data_type;
    Token* token = (Token*)ListGet(tokens, *idx);
    while (token->type == TOKEN_TYPE_COMMA) {
        *idx = ListNext(tokens, *idx);

        AST_Node* data_type2 = GetDataType(tokens, idx);
        if (data_type2 == NULL) {
            assert(0); //FIXME - error handler
        }

        AST_Node* identifier2 = GetIdentifier(tokens, idx);
        if (identifier2 == NULL) {
            assert(0); //FIXME - error handler
        }

        data_type2->right = identifier2;
        identifier2->parent = data_type2;

        cur_node->left = data_type2;
        data_type2->parent = cur_node;
        
        cur_node = cur_node->left;

        token = (Token*)ListGet(tokens, *idx);
    }

    return data_type;
}

static AST_Node* GetStatement(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* statement = NULL;
    SyntaxFunc statements[] = {
        GetVarDec,
        GetAssignment,
        GetIfStatement,
        GetWhileStatement,
        GetReturnStatement,
        GetBlock,
        GetExprStatement,
        GetPrint
    };
    size_t statements_size = sizeof(statements)/sizeof(SyntaxFunc);

    for (size_t i = 0; i < statements_size; i++) {
        statement = statements[i](tokens, idx);
        if (statement != NULL) {
            break;
        }
    }

    return statement;
}

static AST_Node* GetIfStatement(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_STATEMENT_IF) {
        return NULL;
    }

    *idx = ListNext(tokens, *idx);

    AST_Node* if_statement = OP_(NULL, NULL, AST_ELEM_OPERATION_IF);
    if (if_statement == NULL) {
        assert(0); //FIXME - error handler
    }

    token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_ROUND_BRACKET_OPEN) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    AST_Node* expression = GetExpression(tokens, idx);
    if (expression == NULL) {
        assert(0); //FIXME - error handler
    }

    token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_ROUND_BRACKET_CLOSE) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    AST_Node* if_block = GetBlock(tokens, idx);
    if (if_block == NULL) {
        assert(0); //FIXME - error handler
    }

    if_statement->left = expression;
    if_statement->right = if_block;

    expression->parent = if_statement;
    if_block->parent = if_statement;

    AST_Node* end_block = if_block;
    while (end_block->right != NULL) {
        end_block = end_block->right;
    }
    
    token = (Token*)ListGet(tokens, *idx);
    if (token->type == TOKEN_TYPE_STATEMENT_ELSE) {
        *idx = ListNext(tokens, *idx);

        if (((Token*)ListGet(tokens, *idx))->type == TOKEN_TYPE_STATEMENT_IF) {
            AST_Node* next_if = GetIfStatement(tokens, idx);

            end_block->right = next_if;
            next_if->parent = end_block;
        } else {
            AST_Node* else_statement = OP_(NULL, NULL, AST_ELEM_OPERATION_ELSE);
            if (else_statement == NULL) {
                assert(0); //FIXME - error handler
            }

            AST_Node* else_block = GetBlock(tokens, idx);
            if (else_block == NULL) {
                assert(0); //FIXME - error handler
            }

            else_statement->right = else_block;
            else_block->parent = else_statement;

            end_block->right = else_statement;
            else_statement->parent = end_block;
        }
    }

    return if_statement;
}

static AST_Node* GetWhileStatement(List_t* tokens, size_t *idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_STATEMENT_WHILE) {
        return NULL;
    }

    *idx = ListNext(tokens, *idx);

    AST_Node* while_statement = OP_(NULL, NULL, AST_ELEM_OPERATION_WHILE);
    if (while_statement == NULL) {
        assert(0); //FIXME - error handler
    }

    token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_ROUND_BRACKET_OPEN) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    AST_Node* expression = GetExpression(tokens, idx);
    if (expression == NULL) {
        assert(0); //FIXME - error handler
    }

    token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_ROUND_BRACKET_CLOSE) {
        assert(0); //FIXME - error handler
    }
    
    *idx = ListNext(tokens, *idx);

    AST_Node* block = GetBlock(tokens, idx);
    if (block == NULL) {
        assert(0); //FIXME - error handler
    }

    while_statement->left = expression;
    while_statement->right = block;

    expression->parent = while_statement;
    block->parent = while_statement;

    return while_statement;
}

static AST_Node* GetReturnStatement(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_STATEMENT_RETURN) {
        return NULL;
    }

    *idx = ListNext(tokens, *idx);

    AST_Node* return_node = OP_(NULL, NULL, AST_ELEM_OPERATION_RETURN);
    if (return_node == NULL) {
        assert(0); //FIXME - error handler
    }

    AST_Node* expr_statement = GetExprStatement(tokens, idx);
    if (expr_statement == NULL) {
        assert(0); //FIXME - error handler
    }

    return_node->right = expr_statement;
    expr_statement->parent = return_node;

    return return_node;
}

static AST_Node* GetBlock(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_CURLY_BRACKET_OPEN) {
        return NULL;
    }

    *idx = ListNext(tokens, *idx);

    AST_Node* block = SENTINEL;
    AST_Node* statement = block;

    while (( statement->left = GetStatement(tokens, idx) ) != NULL) {
        statement->right = SENTINEL;

        statement->left->parent = statement;
        statement->right->parent = statement;

        statement = statement->right;
    }

    token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_CURLY_BRACKET_CLOSE) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    return block;
}

static AST_Node* GetExprStatement(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* expression = GetExpression(tokens, idx);
    if (expression == NULL) {
        return NULL;
    }

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_SEMICOLON) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    return expression;
}

static AST_Node* GetVarDec(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* data_type = GetDataType(tokens, idx);
    if (data_type == NULL) {
        return NULL;
    }

    AST_Node* identifier = GetIdentifier(tokens, idx);
    if (identifier == NULL) {
        assert(0); //FIXME - error handler
    }

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type == TOKEN_TYPE_ASSIGNMENT) {
        AST_Node* assignment = OP_(NULL, NULL, AST_ELEM_OPERATION_ASSIGNMENT);
        if (assignment == NULL) {
            assert(0); //FIXME - error handler
        }

        *idx = ListNext(tokens, *idx);

        AST_Node* expression = GetExpression(tokens, idx);
        if (expression == NULL) {
            assert(0); //FIXME - error handler
        }

        token = (Token*)ListGet(tokens, *idx);
        if (token->type != TOKEN_TYPE_SEMICOLON) {
            assert(0); //FIXME - error handler
        }

        *idx = ListNext(tokens, *idx);

        data_type->right = assignment;
        assignment->parent = data_type;
        assignment->left = identifier;
        assignment->right = expression;
        identifier->parent = assignment;
        expression->parent = assignment;

        return data_type;

    } else if (token->type == TOKEN_TYPE_SEMICOLON) {
        *idx = ListNext(tokens, *idx);

        data_type->right = identifier;
        identifier->parent = data_type;

        return data_type;
    } else {
        assert(0); //FIXME - error handler
    }

    assert(0);
    return NULL;
}

static AST_Node* GetAssignment(List_t* tokens, size_t* idx) {
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

static AST_Node* GetPrint(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_PRINT) {
        return NULL;
    }

    *idx = ListNext(tokens, *idx);

    token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_ROUND_BRACKET_OPEN) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    AST_Node* expression = GetExpression(tokens, idx);
    if (expression == NULL) {
        assert(0); //FIXME - error handler
    }

    token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_ROUND_BRACKET_CLOSE) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_SEMICOLON) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    return OP_(NULL, expression, AST_ELEM_OPERATION_PRINT);
}

static AST_Node* GetExpression(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    return GetLogicalOr(tokens, idx);
}

static AST_Node* GetLogicalOr(List_t* tokens, size_t* idx) {
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

    // assert(node1 != NULL);
    return node1;
}

static AST_Node* GetLogicalAnd(List_t* tokens, size_t* idx) {
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

    // assert(node1 != NULL);
    return node1;
}

static AST_Node* GetEquality(List_t* tokens, size_t* idx) {
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

    // assert(node1 != NULL);
    return node1;
}

static AST_Node* GetComparison(List_t* tokens, size_t* idx) {
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

    // assert(node1 != NULL);
    return node1;
}

static AST_Node* GetTerm(List_t* tokens, size_t* idx) {
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

    // assert(node1 != NULL);
    return node1;
}

static AST_Node* GetFactor(List_t* tokens, size_t* idx) {
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

    // assert(node1 != NULL);
    return node1;
}

static AST_Node* GetPrimary(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* node = GetFuncCall(tokens, idx);
    if (node != NULL) {
        return node;
    }

    if ((node = GetIdentifier(tokens, idx)) != NULL) {
        return node;
    }

    if ((node = GetNumber(tokens, idx)) != NULL) {
        return node;
    }

    if ((node = GetInput(tokens, idx)) != NULL) {
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

    // fprintf(stderr, "Code: %d\n", token->type);
    // assert(node != NULL);
    return node;
}

static AST_Node* GetInput(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_INPUT) {
        return NULL;
    }

    *idx = ListNext(tokens, *idx);

    token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_ROUND_BRACKET_OPEN) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_ROUND_BRACKET_CLOSE) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx);

    return OP_(NULL, NULL, AST_ELEM_OPERATION_INPUT);
}

static AST_Node* GetFuncCall(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    if ( !(((Token*)ListGet(tokens, *idx))->type == TOKEN_TYPE_VARIABLE) ||
         !(((Token*)ListGet(tokens, ListNext(tokens, *idx)))->type == TOKEN_TYPE_ROUND_BRACKET_OPEN) ) {
        return NULL;
    }

    AST_Node* identifier = GetIdentifier(tokens, idx);
    if (identifier == NULL) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx); // skip '('

    AST_Node* arguments = GetArguments(tokens, idx);

    if (((Token*)ListGet(tokens, *idx))->type != TOKEN_TYPE_ROUND_BRACKET_CLOSE) {
        assert(0); //FIXME - error handler
    }

    *idx = ListNext(tokens, *idx); // skip ')'

    return OP_(arguments, identifier, AST_ELEM_OPERATION_CALL);
}

static AST_Node* GetArguments(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    AST_Node* expression = GetExpression(tokens, idx);
    if (expression == NULL) {
        return NULL;
    }

    AST_Node* arguments = OP_(NULL, expression, AST_ELEM_OPERATION_SENTINEL);
    AST_Node* cur_node = arguments;
    Token* token = (Token*)ListGet(tokens, *idx);
    while (token->type == TOKEN_TYPE_COMMA) {
        *idx = ListNext(tokens, *idx);

        AST_Node* expression2 = GetExpression(tokens, idx);
        if (expression2 == NULL) {
            assert(0); //FIXME - error handler
        }

        AST_Node* arguments2 = OP_(NULL, expression2, AST_ELEM_OPERATION_SENTINEL);

        cur_node->left = arguments2;
        arguments2->parent = cur_node;

        cur_node = cur_node->left;

        token = (Token*)ListGet(tokens, *idx);
    }

    return arguments;
}

static AST_Node* GetDataType(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type == TOKEN_TYPE_SHORT) {
        *idx = ListNext(tokens, *idx);

        return DECL_(CONST_TYPE_SHORT);

    } else if (token->type == TOKEN_TYPE_INT) {
        *idx = ListNext(tokens, *idx);

        return DECL_(CONST_TYPE_INT);

    } else if (token->type == TOKEN_TYPE_LONG) {
        *idx = ListNext(tokens, *idx);

        return DECL_(CONST_TYPE_LONG);

    } else if (token->type == TOKEN_TYPE_DOUBLE) {
        *idx = ListNext(tokens, *idx);

        return DECL_(CONST_TYPE_DOUBLE);

    } else if (token->type == TOKEN_TYPE_CHAR) {
        *idx = ListNext(tokens, *idx);

        return DECL_(CONST_TYPE_CHAR);

    } else if (token->type == TOKEN_TYPE_VOID) {
        *idx = ListNext(tokens, *idx);

        return DECL_(CONST_TYPE_VOID);
    }

    return NULL;
}

static AST_Node* GetIdentifier(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_VARIABLE) {
        return NULL;
    }

    *idx = ListNext(tokens, *idx);

    return v(token->data.variable);
}

static AST_Node* GetNumber(List_t* tokens, size_t* idx) {
    assert( tokens != NULL );
    assert( idx != NULL );

    Token* token = (Token*)ListGet(tokens, *idx);
    if (token->type != TOKEN_TYPE_CONST) {
        return NULL;
    }

    *idx = ListNext(tokens, *idx);

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

    case CONST_TYPE_VOID:
        assert(0);

    case CONST_TYPE_UNDEFINED:
        assert(0);

    default:
        assert(0);
    }

    assert(0);
    return NULL;
}
