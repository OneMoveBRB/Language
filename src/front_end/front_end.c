#include "../../include/front_end/front_end.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../../clibs/List/list.h"
#include "../../clibs/Buffer/include/buffer.h"

#include "../../include/ast/ast.h"
#include "../../include/ast/ast_dump.h"

#include "../../include/front_end/lexer.h"
#include "../../include/front_end/syntax.h"
#include "../../include/io.h"
#include "../../include/utils.h"

static void FreeString(void* node_value);

const char* tree_dump_text = "syntax_tree.txt";

FrontEndErr_t FrontEnd(AST** ast, const char* file_name) {
    assert( file_name != NULL );

    Buffer_t* buffer = BufferInit(0, sizeof(char));
    if (buffer == NULL) {
        return FRONT_END_BUFFER_FAILED;
    }

    if (BufferGet(buffer, file_name) != IO_OK) {
        fprintf(stderr, "BufferGet(buffer, file_name) != IO_OK\n");
        BufferDestroy(&buffer);
        return FRONT_END_IO_FAILED;
    }

    List_t* tokens = ListInit(sizeof(Token));
    if (tokens == NULL) {
        fprintf(stderr, "tokens == NULL\n");
        BufferDestroy(&buffer);
        return FRONT_END_LIST_FAILED;
    }

    size_t read_elems = LexicalAnalysis((const char*)buffer->data, tokens); //FIXME - error handler

    BufferDestroy(&buffer);

    *ast = SyntaxAnalysis(tokens); //FIXME - error handler

    DotVizualizeTree(*ast, tree_dump_text);
    
    ListDestroy(&tokens, FreeString);

    return FRONT_END_OK;
}

static void FreeString(void* node_value) {
    Token* token = (Token*)node_value;
    if (token->type == TOKEN_TYPE_VARIABLE) {
        FREE(token->data.variable);
    }
}
