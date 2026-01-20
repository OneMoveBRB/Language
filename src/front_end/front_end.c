#include "../../include/front_end/front_end.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../../List/list.h"
#include "../../../Buffer/include/buffer.h"

#include "../../include/ast.h"
#include "../../include/ast_dump.h"

// #include "../../include/symbol_table.h"
// #include "../../include/symbol_table_dump.h"

#include "../../include/front_end/lexer.h"
#include "../../include/front_end/syntax.h"
#include "../../include/io.h"
#include "../../include/utils.h"

static void FreeString(void* node_value);

FrontEndErr_t FrontEnd(AST** ast, const char* file_name) {
    assert( file_name != NULL );

    Buffer_t* buffer = BufferInit(0, sizeof(char));
    if (buffer == NULL) {
        return FRONT_END_BUFFER_FAILED;
    }

    if (BufferGet(buffer, file_name) != IO_OK) {
        fprintf(stderr, "BufferGet(buffer, file_name) != IO_OK\n");
    }

    List_t* tokens = ListInit(sizeof(Token));
    if (tokens == NULL) {
        fprintf(stderr, "tokens == NULL\n");
    }

    size_t read_elems = LexicalAnalysis((const char*)buffer->data, tokens);

    /*
    printf("Read elems: %zu\n", read_elems);

    for (size_t i = ListFront(tokens); i != ListEnd(tokens); i++) {
        Token* temp = (Token*)ListGet(tokens, i);
        printf("code: %d\t", temp->type);
        if (temp->type == TOKEN_TYPE_VARIABLE) {
            printf("%s\n", temp->data.variable);
        } else if (temp->type == TOKEN_TYPE_CONST) {
            if (temp->data.constant.type == CONST_TYPE_DOUBLE) {
                printf("%lg\n", temp->data.constant.data.double_const);
            } else if (temp->data.constant.type == CONST_TYPE_INT) {
                printf("%d\n", temp->data.constant.data.int_const);
            }
        } else {
            printf("\n");
        }
    }
    */

    *ast = SyntaxAnalysis(tokens);

    const char* tree_dump_text = "syntax_tree.txt";

    DotVizualizeTree(*ast, tree_dump_text);

    /*
        SymbolTable* table = SymbolTableInit();
    if (table == NULL) {
        fprintf(stderr, "table == NULL\n");
    }
        
    SymbolTableInsert(table, "message1.1", SYM_TYPE_VARIABLE, DATA_TYPE_INT, NULL);
    SymbolTableInsert(table, "message1.2", SYM_TYPE_VARIABLE, DATA_TYPE_INT, NULL);
    SymbolTableInsert(table, "message1.3", SYM_TYPE_VARIABLE, DATA_TYPE_INT, NULL);

    SymbolTableEnterScope(table);

    SymbolTableInsert(table, "message2.1", SYM_TYPE_VARIABLE, DATA_TYPE_INT, NULL);
    SymbolTableInsert(table, "message2.2", SYM_TYPE_VARIABLE, DATA_TYPE_INT, NULL);

    SymbolTableEnterScope(table);

    SymbolTableInsert(table, "message3.1", SYM_TYPE_VARIABLE, DATA_TYPE_INT, NULL);
    SymbolTableInsert(table, "message3.2", SYM_TYPE_VARIABLE, DATA_TYPE_INT, NULL);

    SymbolTableNewBranch(table);
    SymbolTableEnterScope(table);

    SymbolTableInsert(table, "text1.1", SYM_TYPE_VARIABLE, DATA_TYPE_INT, NULL);
    SymbolTableInsert(table, "text1.2", SYM_TYPE_VARIABLE, DATA_TYPE_INT, NULL);

    SymbolTableEnterScope(table);

    SymbolTableInsert(table, "text2.1", SYM_TYPE_VARIABLE, DATA_TYPE_INT, NULL);
    SymbolTableInsert(table, "text2.2", SYM_TYPE_VARIABLE, DATA_TYPE_INT, NULL);

    DotVizualizeSymbolTable(table, "dot_sym_tab.txt");

    SymbolTableDestroy(&table);
    */

    BufferDestroy(&buffer);
    ListDestroy(&tokens, FreeString);

    return FRONT_END_OK;
}

static void FreeString(void* node_value) {
    Token* token = (Token*)node_value;
    if (token->type == TOKEN_TYPE_VARIABLE) {
        FREE(token->data.variable);
    }
}
