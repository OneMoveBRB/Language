#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../../clibs/Stack/include/stack.h"
#include "../../clibs/HashTable/include/hash_table.h"

typedef enum SymbolType {
    SYM_TYPE_VARIABLE,
    SYM_TYPE_CONSTANT,
    SYM_TYPE_FUNCTION,
    SYM_TYPE_PARAMETER
} SymbolType;

typedef enum DataType {
    DATA_TYPE_SHORT,
    DATA_TYPE_INT,
    DATA_TYPE_LONG,
    DATA_TYPE_DOUBLE,
    DATA_TYPE_CHAR,
    DATA_TYPE_VOID
} DataType;

typedef struct SymbolData {
    SymbolType   symbol_type;
    DataType     data_type;
    unsigned int scope_level;
    void*        ast_node;
} SymbolData;

typedef struct Scope {
    unsigned int  level;
    HashTable_t*  symbols;
    struct Scope* prev;
} Scope;

typedef struct SymbolTable {
    Scope* global_scope;
    Scope* current_scope;
    Stack_t* end_scopes;
} SymbolTable;

typedef enum SymbolTableErr_t {
    SYM_TAB_OK,
    SYM_TAB_CALLOC_SCOPE_FAILED,
    SYM_TAB_STACK_FAILED,
    SYM_TAB_HASH_TABLE_FAILED
} SymbolTableErr_t;

SymbolTable*     SymbolTableInit();
SymbolTableErr_t SymbolTableDestroy(SymbolTable** table_ptr);

SymbolTableErr_t SymbolTableEnterScope(SymbolTable* table);
SymbolTableErr_t SymbolTableExitScope(SymbolTable* table);

SymbolTableErr_t SymbolTableNewBranch(SymbolTable* table);
SymbolTableErr_t SymbolTableDelBranch(SymbolTable* table);

SymbolData* SymbolTableLookUpCurrentScope(SymbolTable* table, const char* symbol_name);
SymbolData* SymbolTableLookUp(SymbolTable* table, const char* symbol_name);

SymbolTableErr_t SymbolTableInsert(SymbolTable* table, const char* symbol_name,
                                   SymbolType symbol_type, DataType data_type, void* ast_node);

#endif /* SYMBOL_TABLE_H */