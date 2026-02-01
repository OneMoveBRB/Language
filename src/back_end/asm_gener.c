#include "../../include/back_end/asm_gener.h"

#include <stdio.h>
#include <assert.h>

#include "../../include/ast/ast.h"
#include "../../include/symbol_table/symbol_table.h"
#include "../../include/symbol_table/symbol_table_dump.h"
#include "../../include/back_end/asm_instructions.h"
#include "../../clibs/Buffer/include/buffer.h"

typedef struct ASM_GenerSetup {
    AST* ast;
    SymbolTable* symbol_table;
    unsigned int scope_level;
    Buffer_t* assembly_code;
    size_t if_cnt;
    size_t while_cnt;
    size_t bool_cnt;
} ASM_GenerSetup;

static BackEndErr_t AST_NodeHandler(AST_Node* node, ASM_GenerSetup* backend);

static BackEndErr_t SentinelHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t ExpressionHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t DeclarationHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t FuncCallHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t ReturnHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t PrintHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t AssignmentHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t IfStatementHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t WhileStatementHandler(AST_Node* node, ASM_GenerSetup* backend);

static BackEndErr_t FuncDecHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t ParamDecHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t VarDecHandler(AST_Node* node, ASM_GenerSetup* backend);

static BackEndErr_t AddHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t SubHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t MulHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t DivHandler(AST_Node* node, ASM_GenerSetup* backend);

static BackEndErr_t LTHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t LEHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t GTHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t GEHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t EEHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t NEHandler(AST_Node* node, ASM_GenerSetup* backend);

static BackEndErr_t LandHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t LorHandler(AST_Node* node, ASM_GenerSetup* backend);

static BackEndErr_t ConstHandler(AST_Node* node, ASM_GenerSetup* backend);

static BackEndErr_t SetVariableHandler(AST_Node* node, ASM_GenerSetup* backend);
static BackEndErr_t GetVariableHandler(AST_Node* node, ASM_GenerSetup* backend);

char* CntLabel(const char* s, size_t cnt);

BackEndErr_t AssemblyCodeGeneration(AST* ast, Buffer_t* assembly_code) {
    assert( ast != NULL );
    assert( assembly_code != NULL );

    SymbolTable* symbol_table = SymbolTableInit();
    if (symbol_table == NULL) {
        return BACK_END_SYMBOL_TABLE_FAILED;
    }
    
    if (assembly_code == NULL) {
        SymbolTableDestroy(&symbol_table);
        return BACK_END_BUFFER_FAILED;
    }

    ASM_GenerSetup backend = {
        .ast = ast,
        .symbol_table = symbol_table,
        .scope_level = 0,
        .assembly_code = assembly_code,
        .if_cnt = 0,
        .while_cnt = 0,
        .bool_cnt = 0,
    };

    backend.symbol_table->global_scope->scope_ram_offset = 0;

    // Base
    BufferPush(assembly_code, asm_base,          strlen(asm_base));
    BufferPush(assembly_code, move_rax_by_one,   strlen(move_rax_by_one));
    BufferPush(assembly_code, enter_scope,       strlen(enter_scope));
    BufferPush(assembly_code, exit_scope,        strlen(exit_scope));
    BufferPush(assembly_code, set_rcx_offset,    strlen(set_rcx_offset));
    BufferPush(assembly_code, get_rcx_by_offset, strlen(get_rcx_by_offset));
    BufferPush(assembly_code, set_rcx_by_offset, strlen(set_rcx_by_offset));

    AST_NodeHandler(backend.ast->root, &backend);

    // test
    // printf("%s", (char*)backend.assembly_code->data);

    SymbolTableDestroy(&backend.symbol_table);

    return BACK_END_OK;
}

static BackEndErr_t AST_NodeHandler(AST_Node* node, ASM_GenerSetup* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    if (node->type == AST_ELEM_TYPE_OPERATION && node->data.operation == AST_ELEM_OPERATION_SENTINEL) {
        SentinelHandler(node, backend);

    } else if (node->type == AST_ELEM_TYPE_DECLARATION) {
        DeclarationHandler(node, backend);

    } else if (node->type == AST_ELEM_TYPE_OPERATION && node->data.operation == AST_ELEM_OPERATION_RETURN) {
        ReturnHandler(node, backend);
        
    } else if (node->type == AST_ELEM_TYPE_OPERATION && node->data.operation == AST_ELEM_OPERATION_PRINT) {
        PrintHandler(node, backend);

    } else if (node->type == AST_ELEM_TYPE_OPERATION && node->data.operation == AST_ELEM_OPERATION_ASSIGNMENT) {
        AssignmentHandler(node, backend);

    } else if (node->type == AST_ELEM_TYPE_OPERATION && node->data.operation == AST_ELEM_OPERATION_IF) {
        IfStatementHandler(node, backend);

    } else if (node->type == AST_ELEM_TYPE_OPERATION && node->data.operation == AST_ELEM_OPERATION_WHILE) {
        WhileStatementHandler(node, backend);

    } else {
        ExpressionHandler(node, backend);

    }

    return BACK_END_OK;
}

static BackEndErr_t SentinelHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t DeclarationHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t FuncCallHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t ReturnHandler(AST_Node* node, ASM_GenerSetup* backend) {
    assert( node    != NULL );
    assert( backend != NULL );
    // fprintf(stderr, "[RET]Sc_l: %u, Sy_l: %u, %p\n", backend->scope_level, backend->symbol_table->current_scope->level, node);

    ExpressionHandler(node->right, backend);

    for (size_t i = 0; i < backend->symbol_table->current_scope->level; i++) {
        BufferPush(backend->assembly_code, exit_scope_call, strlen(exit_scope_call));
    }

    SymbolTableExitScope(backend->symbol_table);
    BufferPush(backend->assembly_code, ret, strlen(ret));

    return BACK_END_OK;
}

static BackEndErr_t ExpressionHandler(AST_Node* node, ASM_GenerSetup* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    switch (node->type) {
    case AST_ELEM_TYPE_VARIABLE:
        return GetVariableHandler(node, backend);
    
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

static BackEndErr_t PrintHandler(AST_Node* node, ASM_GenerSetup* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    ExpressionHandler(node->right, backend);

    BufferPush(backend->assembly_code, out, strlen(out));

    return BACK_END_OK;
}

static BackEndErr_t AssignmentHandler(AST_Node* node, ASM_GenerSetup* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    AST_Node* assignment_node = node->left;
    while (assignment_node->right != NULL) {
        ExpressionHandler(node->right, backend);
        SetVariableHandler(assignment_node->right, backend);

        assignment_node = assignment_node->left;
    }

    ExpressionHandler(node->right, backend);
    SetVariableHandler(assignment_node, backend);
    
    return BACK_END_OK;
}

static BackEndErr_t IfStatementHandler(AST_Node* node, ASM_GenerSetup* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    ExpressionHandler(node->left, backend);

    char* if_st  = CntLabel(if_statement, backend->if_cnt);
    char* if_end = CntLabel(endif, backend->if_cnt);

    ++backend->if_cnt;

    BufferPush(backend->assembly_code, if_st, strlen(if_st));

    SymbolTableEnterScope(backend->symbol_table);
    BufferPush(backend->assembly_code, enter_scope_call, strlen(enter_scope_call));

    AST_NodeHandler(node->right, backend);

    BufferPush(backend->assembly_code, if_end, strlen(if_end));

    FREE(if_st);
    FREE(if_end);

    return BACK_END_OK;
}

static BackEndErr_t WhileStatementHandler(AST_Node* node, ASM_GenerSetup* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    char* if_st  = CntLabel(if_statement, backend->if_cnt);
    char* if_beg = CntLabel(begif, backend->if_cnt);
    char* if_beg_jump = CntLabel(begif_jmp, backend->if_cnt);
    char* if_end = CntLabel(endif, backend->if_cnt);

    ++backend->if_cnt;

    BufferPush(backend->assembly_code, if_beg, strlen(if_beg));

    ExpressionHandler(node->left, backend);

    BufferPush(backend->assembly_code, if_st, strlen(if_st));

    SymbolTableEnterScope(backend->symbol_table);
    BufferPush(backend->assembly_code, enter_scope_call, strlen(enter_scope_call));

    AST_NodeHandler(node->right, backend);

    BufferPush(backend->assembly_code, if_beg_jump, strlen(if_beg_jump));
    BufferPush(backend->assembly_code, if_end, strlen(if_end));

    FREE(if_st);
    FREE(if_beg);
    FREE(if_beg_jump);
    FREE(if_end);

    return BACK_END_OK;
}

// ============================= DECLARATION HANDLER =============================

static BackEndErr_t FuncDecHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t ParamDecHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t VarDecHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t AddHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t SubHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t MulHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t DivHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t LTHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t LEHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t GTHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t GEHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t EEHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t NEHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t LandHandler(AST_Node* node, ASM_GenerSetup* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    MulHandler(node, backend);

    char* cnt_bool_cmp = CntLabel(unary_bool_cmp, backend->bool_cnt);
    backend->bool_cnt++;

    BufferPush(backend->assembly_code, cnt_bool_cmp, strlen(cnt_bool_cmp));

    FREE(cnt_bool_cmp);

    return BACK_END_OK;
}

static BackEndErr_t LorHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

static BackEndErr_t ConstHandler(AST_Node* node, ASM_GenerSetup* backend) {
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

// ============================== VARIABLE HANDLER ==============================

static BackEndErr_t GetVariableHandler(AST_Node* node, ASM_GenerSetup* backend) {
    assert( node    != NULL );
    assert( backend != NULL );
    // fprintf(stderr, "%p\n", node); //FIXME - dynamic temp_buffer or clever to do

    SymbolData* symbol_data = SymbolTableLookUp(backend->symbol_table, node->data.variable);
    if (symbol_data == NULL) {
        fprintf(stderr, "GetVariableHandler: SymbolTableLookUp == NULL\n");
        return BACK_END_SYMBOL_TABLE_FAILED;
    }   // need error handler in middle_end

    char temp_buffer[MAX_LEN] = "";

    snprintf(temp_buffer, MAX_LEN, "; get variable \"%s\"\n", node->data.variable);

    strcat(temp_buffer + strlen(temp_buffer),   "PUSHR  RBX\n"
                                                "POPR   RCX\n");

    for (size_t i = symbol_data->scope_level; i < backend->scope_level - 1; i++) {
        strcat(temp_buffer + strlen(temp_buffer), "PUSHM [RCX]\n"
                                                  "POPR   RCX\n");
    }

    snprintf(temp_buffer + strlen(temp_buffer), MAX_LEN,  "PUSH %lu\n"
                                    "CALL get_rcx_by_offset\n\n", symbol_data->symbol_ram_offset);

    BufferPush(backend->assembly_code, temp_buffer, strlen(temp_buffer));

    return BACK_END_OK;
}

static BackEndErr_t SetVariableHandler(AST_Node* node, ASM_GenerSetup* backend) {
    assert( node    != NULL );
    assert( backend != NULL );

    SymbolData* symbol_data = SymbolTableLookUp(backend->symbol_table, node->data.variable);
    if (symbol_data == NULL) {
        fprintf(stderr, "SetVariableHandler: SymbolTableLookUp == NULL\n");
        return BACK_END_SYMBOL_TABLE_FAILED;
    }   // need error handler in middle_end

    char temp_buffer[MAX_LEN] = "";

    snprintf(temp_buffer, MAX_LEN, "; set variable \"%s\"\n", node->data.variable);

    strcat(temp_buffer + strlen(temp_buffer),   "PUSHR  RBX\n"
                                                "POPR   RCX\n");

    for (size_t i = symbol_data->scope_level; i < backend->scope_level - 1; i++) {
        strcat(temp_buffer + strlen(temp_buffer), "PUSHM [RCX]\n"
                                                  "POPR   RCX\n");
    }

    snprintf(temp_buffer + strlen(temp_buffer), MAX_LEN,  "PUSH %lu\n"
                                    "CALL set_rcx_by_offset\n\n", symbol_data->symbol_ram_offset);

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
