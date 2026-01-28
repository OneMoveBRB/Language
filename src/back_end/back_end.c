#include "../../include/back_end/back_end.h"

#include <stdio.h>
#include <assert.h>

#include "../../include/ast/ast.h"
#include "../../include/symbol_table/symbol_table.h"
#include "../../clibs/Buffer/include/buffer.h"

typedef struct BackEnd {
    AST* ast;
    SymbolTable* symbol_table;
    unsigned int scope_level;
    Buffer_t* assembly_code;
    size_t if_cnt;
    size_t while_cnt;
    size_t cur_ram_offset;
} BackEnd;

const int MAX_LEN = 128;

static BackEndErr_t AssemblyCodeGeneration(BackEnd* backend);

static BackEndErr_t SentinelHandler(AST_Node* node, BackEnd* backend);

static BackEndErr_t ExpressionHandler(AST_Node* node, BackEnd* backend);

static BackEndErr_t AddHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t SubHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t MulHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t DivHandler(AST_Node* node, BackEnd* backend);

static BackEndErr_t LTHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t LEHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t GTHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t GEHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t EEHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t NEHandler(AST_Node* node, BackEnd* backend);

static BackEndErr_t LandHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t LorHandler(AST_Node* node, BackEnd* backend);

static BackEndErr_t ConstHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t VariableHandler(AST_Node* node, BackEnd* backend);
//FIXME - do different boolean checking
BackEndErr_t CodeGeneration(AST* ast) {
    assert( ast != NULL );

    SymbolTable* symbol_table = SymbolTableInit();
    if (symbol_table == NULL) {
        return BACK_END_SYMBOL_TABLE_FAILED;
    }

    Buffer_t* assembly_code = BufferInit(0, sizeof(char));
    if (assembly_code == NULL) {
        SymbolTableDestroy(&symbol_table);
        return BACK_END_BUFFER_FAILED;
    }

    BackEnd back_end = {
        .ast = ast,
        .symbol_table = symbol_table,
        .scope_level = (unsigned int)-1,
        .assembly_code = assembly_code,
        .if_cnt = 0,
        .while_cnt = 0,
        .cur_ram_offset = 0
    };

    AssemblyCodeGeneration(&back_end);

    printf("%s", (char*)back_end.assembly_code->data);
    // for (size_t i = 0; i < back_end.assembly_code->capacity; i++) {
    //     printf("%c", *((char*)back_end.assembly_code->data + i));
    // }

    SymbolTableDestroy(&back_end.symbol_table);
    BufferDestroy(&back_end.assembly_code);

    return BACK_END_OK;
}

static BackEndErr_t AssemblyCodeGeneration(BackEnd* backend) {
    assert( backend != NULL );

    backend->symbol_table->global_scope->scope_ram_offset = 0;
    // backend->cur_ram_offset = 0;
    // backend->scope_level = (unsigned int)-1;

    Buffer_t* assembly_code = backend->assembly_code;
    const char* regs_init = "PUSH 0\n"
                            "POPR RAX\n\n"
                            "PUSH 0\n"
                            "POPR RBX\n\n"
                            "PUSH 0\n"
                            "POPR RCX\n\n"
                            "JMP main\n\n";

    BufferPush(assembly_code, regs_init, strlen(regs_init));

    // test
    fprintf(stderr, "Code: %d\n", ExpressionHandler(backend->ast->root->left, backend));

    return BACK_END_OK;
}

BackEndErr_t AST_NodeHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->type == AST_ELEM_TYPE_OPERATION && node->data.operation == AST_ELEM_OPERATION_SENTINEL) {
        SentinelHandler(node, backend);

    } else if (node->type == AST_ELEM_TYPE_VARIABLE) {
        VariableHandler(node, backend);
    }
}

static BackEndErr_t SentinelHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->right == NULL) {
        SymbolTableExitScope(backend->symbol_table);
        backend->cur_ram_offset = backend->symbol_table->current_scope->scope_ram_offset;

        return BACK_END_OK;
    }

    ++backend->scope_level;

    while (backend->scope_level > backend->symbol_table->current_scope->level) {
        SymbolTableEnterScope(backend->symbol_table);
        backend->symbol_table->current_scope->scope_ram_offset = backend->cur_ram_offset;
    }
    
    AST_NodeHandler(node->left, backend);

    --backend->scope_level;

    AST_NodeHandler(node->right, backend);

    return BACK_END_OK;
}

static BackEndErr_t DeclarationHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    AST_Node* right_node = node->right; assert( right_node != NULL );
    AST_Node* left_node  = node->left;  assert( left_node  != NULL );

    if (    right_node->type == AST_ELEM_TYPE_OPERATION 
        &&  right_node->data.operation == AST_ELEM_OPERATION_ASSIGNMENT ) {
        
    }

}

static BackEndErr_t ExpressionHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    switch (node->type) {
    case AST_ELEM_TYPE_VARIABLE:
        return VariableHandler(node, backend);
    
    case AST_ELEM_TYPE_CONST:
        return ConstHandler(node, backend);

    case AST_ELEM_TYPE_OPERATION:
        break;

    case AST_ELEM_TYPE_DECLARATION:
    case AST_ELEM_TYPE_UNDEFINED:
        assert(0);

    default:
        assert(0);
    }

    switch (node->data.operation) {
    case AST_ELEM_OPERATION_ADD:
        return AddHandler(node, backend);

    case AST_ELEM_OPERATION_SUB:
        return SubHandler(node, backend);

    case AST_ELEM_OPERATION_MUL:
        return MulHandler(node, backend);

    case AST_ELEM_OPERATION_DIV:
        return DivHandler(node, backend);

    case AST_ELEM_OPERATION_LT:
        return LTHandler(node, backend);

    case AST_ELEM_OPERATION_LE:
        return LEHandler(node, backend);

    case AST_ELEM_OPERATION_GT:
        return GTHandler(node, backend);

    case AST_ELEM_OPERATION_GE:
        return GEHandler(node, backend);

    case AST_ELEM_OPERATION_EE:
        return EEHandler(node, backend);

    case AST_ELEM_OPERATION_NE:
        return NEHandler(node, backend);

    case AST_ELEM_OPERATION_LAND:
        return LandHandler(node, backend);

    case AST_ELEM_OPERATION_LOR:
        return LorHandler(node, backend);

    case AST_ELEM_OPERATION_CALL:
        assert(0);
    
    default:
        assert(0);
    }

    return BACK_END_ERROR;
}

static BackEndErr_t AddHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->left == NULL || node->right == NULL) {
        assert(0); //FIXME - error handler
    }

    ExpressionHandler(node->left, backend);
    ExpressionHandler(node->right, backend);

    const char* add = "ADD\n";

    BufferPush(backend->assembly_code, add, strlen(add));

    return BACK_END_OK;
}

static BackEndErr_t SubHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->left == NULL || node->right == NULL) {
        assert(0); //FIXME - error handler
    }

    ExpressionHandler(node->left, backend);
    ExpressionHandler(node->right, backend);

    const char* sub = "SUB\n";

    BufferPush(backend->assembly_code, sub, strlen(sub));

    return BACK_END_OK;
}

static BackEndErr_t MulHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->left == NULL || node->right == NULL) {
        assert(0); //FIXME - error handler
    }

    ExpressionHandler(node->left, backend);
    ExpressionHandler(node->right, backend);

    const char* mul = "MUL\n";

    BufferPush(backend->assembly_code, mul, strlen(mul));

    return BACK_END_OK;
}

static BackEndErr_t DivHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->left == NULL || node->right == NULL) {
        assert(0); //FIXME - error handler
    }

    ExpressionHandler(node->left, backend);
    ExpressionHandler(node->right, backend);

    const char* div = "DIV\n";

    BufferPush(backend->assembly_code, div, strlen(div));

    return BACK_END_OK;
}

// ================================ JUMP HANDLER ================================

static BackEndErr_t LTHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->left == NULL || node->right == NULL) {
        assert(0); //FIXME - error handler
    }

    ExpressionHandler(node->left, backend);
    ExpressionHandler(node->right, backend);

    const char* jb_cmp = "JBE false_comparison_result\n"
                         "PUSH 1\n"
                         "JMP truth_comparison_result\n"
                         ":   false_comparison_result\n"
                         "PUSH 0\n"
                         ":   truth_comparison_result\n\n";

    BufferPush(backend->assembly_code, jb_cmp, strlen(jb_cmp));

    return BACK_END_OK;
}

static BackEndErr_t LEHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->left == NULL || node->right == NULL) {
        assert(0); //FIXME - error handler
    }

    ExpressionHandler(node->left, backend);
    ExpressionHandler(node->right, backend);

    const char* jbe_cmp = "JB false_comparison_result\n"
                         "PUSH 1\n"
                         "JMP truth_comparison_result\n"
                         ":   false_comparison_result\n"
                         "PUSH 0\n"
                         ":   truth_comparison_result\n\n";

    BufferPush(backend->assembly_code, jbe_cmp, strlen(jbe_cmp));

    return BACK_END_OK;
}

static BackEndErr_t GTHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->left == NULL || node->right == NULL) {
        assert(0); //FIXME - error handler
    }

    ExpressionHandler(node->left, backend);
    ExpressionHandler(node->right, backend);

    const char* ja_cmp = "JAE false_comparison_result\n"
                         "PUSH 1\n"
                         "JMP truth_comparison_result\n"
                         ":   false_comparison_result\n"
                         "PUSH 0\n"
                         ":   truth_comparison_result\n\n";

    BufferPush(backend->assembly_code, ja_cmp, strlen(ja_cmp));

    return BACK_END_OK;
}

static BackEndErr_t GEHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->left == NULL || node->right == NULL) {
        assert(0); //FIXME - error handler
    }

    ExpressionHandler(node->left, backend);
    ExpressionHandler(node->right, backend);

    const char* jae_cmp = "JA false_comparison_result\n"
                         "PUSH 1\n"
                         "JMP truth_comparison_result\n"
                         ":   false_comparison_result\n"
                         "PUSH 0\n"
                         ":   truth_comparison_result\n\n";

    BufferPush(backend->assembly_code, jae_cmp, strlen(jae_cmp));

    return BACK_END_OK;
}

static BackEndErr_t EEHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->left == NULL || node->right == NULL) {
        assert(0); //FIXME - error handler
    }

    ExpressionHandler(node->left, backend);
    ExpressionHandler(node->right, backend);

    const char* je_cmp = "JNE false_comparison_result\n"
                         "PUSH 1\n"
                         "JMP truth_comparison_result\n"
                         ":   false_comparison_result\n"
                         "PUSH 0\n"
                         ":   truth_comparison_result\n\n";

    BufferPush(backend->assembly_code, je_cmp, strlen(je_cmp));

    return BACK_END_OK;
}

static BackEndErr_t NEHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->left == NULL || node->right == NULL) {
        assert(0); //FIXME - error handler
    }

    ExpressionHandler(node->left, backend);
    ExpressionHandler(node->right, backend);

    const char* jne_cmp = "JE false_comparison_result\n"
                         "PUSH 1\n"
                         "JMP truth_comparison_result\n"
                         ":   false_comparison_result\n"
                         "PUSH 0\n"
                         ":   truth_comparison_result\n\n";

    BufferPush(backend->assembly_code, jne_cmp, strlen(jne_cmp));

    return BACK_END_OK;
}

static BackEndErr_t LandHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    MulHandler(node, backend);

    const char* bool_cmp = "PUSH 1\n"
                           "JA false_comparison_result\n"
                           "PUSH 1\n"
                           "JMP truth_comparison_result\n"
                           ":   false_comparison_result\n"
                           "PUSH 0\n"
                           ":   truth_comparison_result\n\n";

    BufferPush(backend->assembly_code, bool_cmp, strlen(bool_cmp));

    return BACK_END_OK;
}

static BackEndErr_t LorHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    AddHandler(node, backend);
    MulHandler(node, backend);
    SubHandler(node, backend);

    const char* bool_cmp = "PUSH 1\n"
                           "JA false_comparison_result\n"
                           "PUSH 1\n"
                           "JMP truth_comparison_result\n"
                           ":   false_comparison_result\n"
                           "PUSH 0\n"
                           ":   truth_comparison_result\n\n";

    BufferPush(backend->assembly_code, bool_cmp, strlen(bool_cmp));

    return BACK_END_OK;
}

//

static BackEndErr_t ConstHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    // just int
    if (node->data.constant.type != CONST_TYPE_INT) {
        assert(0); //TODO - other const types
    }

    char temp_buffer[MAX_LEN] = "";

    snprintf(temp_buffer, MAX_LEN, "PUSH %d\n", node->data.constant.data.int_const);

    BufferPush(backend->assembly_code, temp_buffer, strlen(temp_buffer));

    return BACK_END_OK;
}

static BackEndErr_t VariableHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    SymbolData* symbol_data = SymbolTableLookUp(backend->symbol_table, node->data.variable);
    if (symbol_data == NULL) {
        fprintf(stderr, "VariableHandler: SymbolTableLookUp == NULL\n");
        return BACK_END_SYMBOL_TABLE_FAILED;
    }   // need error handler in middle_end

    char temp_buffer[MAX_LEN] = "";

    snprintf(temp_buffer, MAX_LEN,  "PUSHR RAX\n"
                                    "PUSH %d\n"
                                    "ADD\n"
                                    "POPR RCX\n"
                                    "PUSHM [RCX]\n\n", (int)symbol_data->symbol_ram_offset);

    BufferPush(backend->assembly_code, temp_buffer, strlen(temp_buffer));

    return BACK_END_OK;
}