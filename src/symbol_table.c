#include "../include/symbol_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/utils.h"

const int INITIAL_CAPACITY = 8;

SymbolTable* SymbolTableInit() {
    SymbolTable* table = (SymbolTable*)calloc(1, sizeof(SymbolTable));
    if (table == NULL) {
        return NULL;
    }

    table->current_scope = NULL;

    table->end_scopes = StackInit(INITIAL_CAPACITY, sizeof(Scope*), "end_scopes");
    if (table->end_scopes == NULL) {
        FREE(table);
        return NULL;
    }

    if (SymbolTableEnterScope(table) != SYM_TAB_OK) {
        FREE(table);
        StackDestroy(&table->end_scopes);
        return NULL;
    }

    table->global_scope = table->current_scope;

    return table;
}

SymbolTableErr_t SymbolTableEnterScope(SymbolTable* table) {
    assert( table != NULL );

    Scope* new_scope = (Scope*)calloc(1, sizeof(Scope));
    if (new_scope == NULL) {
        return SYM_TAB_CALLOC_SCOPE_FAILED;
    }

    new_scope->prev = table->current_scope;
    table->current_scope = new_scope;

    new_scope->symbols = HashTableInit();
    if (new_scope->symbols == NULL) {
        FREE(new_scope);
        return SYM_TAB_HASH_TABLE_FAILED;
    }

    if (new_scope->prev != NULL) {
        new_scope->level = new_scope->prev->level + 1;
    } else {
        new_scope->level = 0;
    }

    return SYM_TAB_OK;
}

SymbolTableErr_t SymbolTableExitScope(SymbolTable* table) {
    assert( table != NULL );

    Scope* cur_scope = table->current_scope;

    cur_scope->level = 0;
    HashTableDestroy(&cur_scope->symbols);

    if (cur_scope->prev == table->global_scope && !StackEmpty(table->end_scopes)) {
        Scope** scope_ptr = (Scope**)StackPop(table->end_scopes);
        if (scope_ptr == NULL) {
            return SYM_TAB_STACK_FAILED;
        }

        table->current_scope = *scope_ptr;
        FREE(scope_ptr);
    } else {
        table->current_scope = cur_scope->prev;
    }

    cur_scope->prev = NULL;
    FREE(cur_scope);

    return SYM_TAB_OK;
}

SymbolTableErr_t SymbolTableDestroy(SymbolTable** table_ptr) {
    assert(  table_ptr != NULL );
    assert( *table_ptr != NULL );

    SymbolTable* table = *table_ptr;

    while (table->current_scope != NULL) {
        SymbolTableExitScope(table);
    }
    
    table->global_scope = NULL;

    StackDestroy(&table->end_scopes);

    FREE(*table_ptr);

    return SYM_TAB_OK;
}

SymbolTableErr_t SymbolTableNewBranch(SymbolTable* table) {
    assert( table != NULL );

    if (StackPush(table->end_scopes, &table->current_scope) != STACK_OK) {
        return SYM_TAB_STACK_FAILED;
    }

    table->current_scope = table->global_scope;

    return SYM_TAB_OK;
}

SymbolTableErr_t SymbolTableDelBranch(SymbolTable* table) {
    assert( table != NULL );

    while (table->current_scope->prev != table->global_scope) {
        SymbolTableExitScope(table);
    }
    
    SymbolTableExitScope(table);

    return SYM_TAB_OK;
}

SymbolData* SymbolTableLookUpCurrentScope(SymbolTable* table, const char* symbol_name) {
    assert( table != NULL );
    assert( symbol_name != NULL );

    return (SymbolData*)HashTableFind(table->current_scope->symbols, 
                                      symbol_name, strlen(symbol_name) + 1);
}

SymbolData* SymbolTableLookUp(SymbolTable* table, const char* symbol_name) {
    assert( table != NULL );
    assert( symbol_name != NULL );

    Scope* cur_scope = table->current_scope;
    while (cur_scope != NULL) {
        SymbolData* symbol_data = (SymbolData*)HashTableFind(cur_scope->symbols, 
                                                             symbol_name, strlen(symbol_name) + 1);
        if (symbol_data != NULL) {
            return symbol_data;
        }

        cur_scope = cur_scope->prev;
    }
    
    return NULL;
}

SymbolTableErr_t SymbolTableInsert(SymbolTable* table, const char* symbol_name,
                                   SymbolType symbol_type, DataType data_type, void* ast_node) {
    assert( table != NULL );
    assert( symbol_name != NULL );

    if (SymbolTableLookUpCurrentScope(table, symbol_name) != NULL) {
        fprintf(stderr, "Symbol %s already declared in this scope\n", symbol_name); // DUMP
        return SYM_TAB_OK;
    }

    SymbolData symbol_data = {
        .symbol_type = symbol_type,
        .data_type = data_type,
        .scope_level = table->current_scope->level,
        .ast_node = ast_node
    };

    if (HashTableInsert(table->current_scope->symbols, symbol_name, strlen(symbol_name) + 1, 
                        &symbol_data, sizeof(SymbolData)) != HASH_TABLE_OK) {
        return SYM_TAB_HASH_TABLE_FAILED;
    }

    return SYM_TAB_OK;
}
