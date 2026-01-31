#include "../../include/back_end/back_end.h"

#include <stdio.h>
#include <assert.h>

#include "../../include/ast/ast.h"
#include "../../include/symbol_table/symbol_table.h"
#include "../../include/symbol_table/symbol_table_dump.h"
#include "../../include/back_end/asm_instructions.h"
#include "../../clibs/Buffer/include/buffer.h"

typedef struct BackEnd {
    AST* ast;
    SymbolTable* symbol_table;
    unsigned int scope_level;
    Buffer_t* assembly_code;
    size_t if_cnt;
    size_t while_cnt;
    size_t bool_cnt;
} BackEnd;

static BackEndErr_t AssemblyCodeGeneration(BackEnd* backend);

static BackEndErr_t AST_NodeHandler(AST_Node* node, BackEnd* backend);

static BackEndErr_t SentinelHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t ExpressionHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t DeclarationHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t FuncCallHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t ReturnHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t PrintHandler(AST_Node* node, BackEnd* backend);

static BackEndErr_t FuncDecHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t ParamDecHandler(AST_Node* node, BackEnd* backend);
static BackEndErr_t VarDecHandler(AST_Node* node, BackEnd* backend);

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

char* CntLabel(const char* s, size_t cnt);

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
        .scope_level = 0,
        .assembly_code = assembly_code,
        .if_cnt = 0,
        .while_cnt = 0,
        .bool_cnt = 0,
    };

    // will be ast -> asm && asm -> bytecode
    AssemblyCodeGeneration(&back_end);

    printf("%s", (char*)back_end.assembly_code->data);

    SymbolTableDestroy(&back_end.symbol_table);
    BufferDestroy(&back_end.assembly_code);

    return BACK_END_OK;
}

static BackEndErr_t AssemblyCodeGeneration(BackEnd* backend) {
    assert( backend != NULL );

    backend->symbol_table->global_scope->scope_ram_offset = 0;

    Buffer_t* assembly_code = backend->assembly_code;

    BufferPush(assembly_code, asm_base,          strlen(asm_base));
    BufferPush(assembly_code, move_rax_by_one,   strlen(move_rax_by_one));
    BufferPush(assembly_code, enter_scope,       strlen(enter_scope));
    BufferPush(assembly_code, exit_scope,        strlen(exit_scope));
    BufferPush(assembly_code, set_rcx_offset,    strlen(set_rcx_offset));
    BufferPush(assembly_code, get_rax_by_offset, strlen(get_rax_by_offset));

    // test
    // fprintf(stderr, "Code: %d\n", ExpressionHandler(backend->ast->root->left, backend));
    AST_NodeHandler(backend->ast->root, backend);

    return BACK_END_OK;
}

static BackEndErr_t AST_NodeHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->type == AST_ELEM_TYPE_OPERATION && node->data.operation == AST_ELEM_OPERATION_SENTINEL) {
        SentinelHandler(node, backend);

    } else if (node->type == AST_ELEM_TYPE_DECLARATION) {
        DeclarationHandler(node, backend);

    } else if (node->type == AST_ELEM_TYPE_VARIABLE) {
        VariableHandler(node, backend);

    } else if (node->type == AST_ELEM_TYPE_OPERATION && node->data.operation == AST_ELEM_OPERATION_RETURN) {
        ReturnHandler(node, backend);
        
    } else if (node->type == AST_ELEM_TYPE_OPERATION && node->data.operation == AST_ELEM_OPERATION_PRINT) {
        PrintHandler(node, backend);

    }
    // else if (node->type == AST_ELEM_TYPE_OPERATION && node->data.operation == AST_ELEM_OPERATION_CALL) {
    //     FuncCallHandler(node, backend);

    // }

    return BACK_END_OK;
}

static BackEndErr_t SentinelHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );
    // fprintf(stderr, "Sc_l: %u, Sy_l: %u, %p\n", backend->scope_level, backend->symbol_table->current_scope->level, node);

    if (backend->scope_level > backend->symbol_table->current_scope->level) {
        if (backend->scope_level == backend->symbol_table->current_scope->level + 1) {
            SymbolTableEnterScope(backend->symbol_table);
            BufferPush(backend->assembly_code, enter_scope_call, strlen(enter_scope_call));
        } else {
            assert(0); //FIXME - error handler
        }
    }

    ++backend->scope_level;

    AST_NodeHandler(node->left, backend);

    --backend->scope_level;

    if (node->right == NULL) {
        if (backend->symbol_table->current_scope != backend->symbol_table->global_scope) {
            BufferPush(backend->assembly_code, exit_scope_call, strlen(exit_scope_call));
            // fprintf(stderr, "%p\n", backend->symbol_table->current_scope);
        }
        SymbolTableExitScope(backend->symbol_table);

        return BACK_END_OK;
    }

    AST_NodeHandler(node->right, backend);

    return BACK_END_OK;
}

static BackEndErr_t DeclarationHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    AST_Node* right_node = node->right; assert( right_node != NULL );
    AST_Node* left_node  = node->left;  assert( left_node  == NULL ); //FIXME - error handler

    if (right_node->type == AST_ELEM_TYPE_VARIABLE && right_node->right != NULL) {
        return FuncDecHandler(node, backend);

    } else {
        return VarDecHandler(node, backend);
    }

    return BACK_END_OK;
}

static BackEndErr_t FuncCallHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    AST_Node* sentinel = node->left;
    while (sentinel != NULL) {
        ExpressionHandler(sentinel->right, backend);
        sentinel = sentinel->left;
    }
    
    char func_name[MAX_LEN] = "";

    int func_name_len = snprintf(func_name, MAX_LEN, "CALL %s\n", node->right->data.variable);

    BufferPush(backend->assembly_code, func_name, (size_t)func_name_len);

    return BACK_END_OK;
}

static BackEndErr_t ReturnHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );
    // fprintf(stderr, "[RET]Sc_l: %u, Sy_l: %u, %p\n", backend->scope_level, backend->symbol_table->current_scope->level, node);

    ExpressionHandler(node->right, backend);

    // --backend->scope_level;
    SymbolTableExitScope(backend->symbol_table);
    BufferPush(backend->assembly_code, exit_scope_call, strlen(exit_scope_call));

    BufferPush(backend->assembly_code, ret, strlen(ret));

    return BACK_END_OK;
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
        return FuncCallHandler(node, backend);
    
    default:
        assert(0);
    }

    return BACK_END_ERROR;
}

static BackEndErr_t PrintHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    VariableHandler(node->right, backend);

    BufferPush(backend->assembly_code, out, strlen(out));

    return BACK_END_OK;
}

// ============================= DECLARATION HANDLER =============================

static BackEndErr_t FuncDecHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );
    // fprintf(stderr, "[FUNC]Sc_l: %u, Sy_l: %u, %p\n", backend->scope_level, backend->symbol_table->current_scope->level, node);

    AST_Node* right_node = node->right;

    char func_label[MAX_LEN] = "";

    int func_label_len = snprintf(func_label, MAX_LEN, ": %s\n", right_node->data.variable);

    BufferPush(backend->assembly_code, func_label, (size_t)func_label_len);

    // ++backend->scope_level;
    SymbolTableEnterScope(backend->symbol_table);
    BufferPush(backend->assembly_code, enter_scope_call, strlen(enter_scope_call));

    AST_Node* var_dec = right_node->left;
    while (var_dec != NULL) {
        // fprintf(stderr, "addr: %p\n", var_dec);
        ParamDecHandler(var_dec, backend);
        var_dec = var_dec->left;
    }

    AST_NodeHandler(right_node->right, backend);

    return BACK_END_OK;
}

static BackEndErr_t ParamDecHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    AST_Node* right_node = node->right;

    BufferPush(backend->assembly_code, ram_push, strlen(ram_push));

    backend->symbol_table->current_scope->scope_ram_offset++;

    SymbolTableInsert(backend->symbol_table, right_node->data.variable, 
                        SYM_TYPE_VARIABLE, DATA_TYPE_INT, right_node, 
                        backend->symbol_table->current_scope->scope_ram_offset);

    return BACK_END_OK;
}

static BackEndErr_t VarDecHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    AST_Node* right_node = node->right;

    if (    right_node->type == AST_ELEM_TYPE_OPERATION 
        &&  right_node->data.operation == AST_ELEM_OPERATION_ASSIGNMENT ) {

        ExpressionHandler(right_node->right, backend);
        BufferPush(backend->assembly_code, ram_push, strlen(ram_push));

    } else {
        BufferPush(backend->assembly_code, move_rax_by_one_call, strlen(move_rax_by_one_call));
    }

    backend->symbol_table->current_scope->scope_ram_offset++;

    SymbolTableInsert(backend->symbol_table, right_node->left->data.variable, 
                        SYM_TYPE_VARIABLE, DATA_TYPE_INT, right_node->left, 
                        backend->symbol_table->current_scope->scope_ram_offset);

    return BACK_END_OK;
}

// ================================ MATH HANDLER ================================

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

    const char* jbe = "JBE";
    
    char* cnt_bool_cmp = CntLabel(bool_cmp, backend->bool_cnt);
    backend->bool_cnt++;

    BufferPush(backend->assembly_code, jbe, strlen(jbe));
    BufferPush(backend->assembly_code, cnt_bool_cmp, strlen(cnt_bool_cmp));

    FREE(cnt_bool_cmp);

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

    const char* jb = "JB";
    
    char* cnt_bool_cmp = CntLabel(bool_cmp, backend->bool_cnt);
    backend->bool_cnt++;

    BufferPush(backend->assembly_code, jb, strlen(jb));
    BufferPush(backend->assembly_code, cnt_bool_cmp, strlen(cnt_bool_cmp));

    FREE(cnt_bool_cmp);

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

    const char* jae = "JAE";
    
    char* cnt_bool_cmp = CntLabel(bool_cmp, backend->bool_cnt);
    backend->bool_cnt++;

    BufferPush(backend->assembly_code, jae, strlen(jae));
    BufferPush(backend->assembly_code, cnt_bool_cmp, strlen(cnt_bool_cmp));

    FREE(cnt_bool_cmp);

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
    
    const char* ja = "JA";
    
    char* cnt_bool_cmp = CntLabel(bool_cmp, backend->bool_cnt);
    backend->bool_cnt++;

    BufferPush(backend->assembly_code, ja, strlen(ja));
    BufferPush(backend->assembly_code, cnt_bool_cmp, strlen(cnt_bool_cmp));

    FREE(cnt_bool_cmp);

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

    const char* jne = "JNE";
    
    char* cnt_bool_cmp = CntLabel(bool_cmp, backend->bool_cnt);
    backend->bool_cnt++;

    BufferPush(backend->assembly_code, jne, strlen(jne));
    BufferPush(backend->assembly_code, cnt_bool_cmp, strlen(cnt_bool_cmp));

    FREE(cnt_bool_cmp);

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

    const char* je = "JE";
    
    char* cnt_bool_cmp = CntLabel(bool_cmp, backend->bool_cnt);
    backend->bool_cnt++;

    BufferPush(backend->assembly_code, je, strlen(je));
    BufferPush(backend->assembly_code, cnt_bool_cmp, strlen(cnt_bool_cmp));

    FREE(cnt_bool_cmp);

    return BACK_END_OK;
}

static BackEndErr_t LandHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    MulHandler(node, backend);

    char* cnt_bool_cmp = CntLabel(unary_bool_cmp, backend->bool_cnt);
    backend->bool_cnt++;

    BufferPush(backend->assembly_code, cnt_bool_cmp, strlen(cnt_bool_cmp));

    FREE(cnt_bool_cmp);

    return BACK_END_OK;
}

static BackEndErr_t LorHandler(AST_Node* node, BackEnd* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    AddHandler(node, backend);
    MulHandler(node, backend);
    
    const char* sub = "SUB\n";

    char* cnt_bool_cmp = CntLabel(unary_bool_cmp, backend->bool_cnt);
    backend->bool_cnt++;

    BufferPush(backend->assembly_code, sub, strlen(sub));
    BufferPush(backend->assembly_code, cnt_bool_cmp, strlen(cnt_bool_cmp));

    FREE(cnt_bool_cmp);

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

    snprintf(temp_buffer, MAX_LEN,  "PUSH %lu\n"
                                    "CALL get_rax_by_offset\n\n", symbol_data->symbol_ram_offset);

    BufferPush(backend->assembly_code, temp_buffer, strlen(temp_buffer));

    return BACK_END_OK;
}

char* CntLabel(const char* s, size_t cnt) {
    assert(s != NULL);
    
    char num_buffer[32];
    int num_len = snprintf(num_buffer, sizeof(num_buffer), "_%zu", cnt);
    
    char* str_buffer = (char*)calloc(MAX_LEN, sizeof(char));
    if (str_buffer == NULL) { 
        return NULL;
    }
    
    char* dest = str_buffer;
    while (*s) {
        if (*s == '#') {
            memcpy(dest, num_buffer, (size_t)num_len);
            dest += num_len;
        } else {
            *dest++ = *s;
        }
        s++;
    }
    *dest = '\0';
    
    return str_buffer;
}