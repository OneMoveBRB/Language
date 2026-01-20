#include "../include/symbol_table_dump.h"

#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/io.h"

#define STACK_IDX(idx) \
    ((size_t)table->end_scopes->data + idx * table->end_scopes->meta.element_size)

void DotVizualizeSymbolTable(const SymbolTable* table, const char* file_name) {
    assert( table != NULL );
    assert( file_name != NULL );

    FILE* fp = fopen(file_name, "w");
    if (fp == NULL) {
        fprintf(stderr, "DotVizualizeSymbolTable: fp == NULL\n");
        return; 
    }

    fprintf(fp, "digraph SymbolTable {\n\t");
    fprintf(fp, "rankdir=HR;\n\t");
    fprintf(fp, "node [shape=record, style=filled, fillcolor=lightblue];\n\t");
    fprintf(fp, "edge [fontsize=10,  color=green];\n\n\t");
    fprintf(fp, "node [shape=record, style=\"filled\", fillcolor=\"lightpink\"];\n\t");

    /* Here your code */
    fprintf(fp, "stack [label=\"<0> 0 ");
    for (size_t i = 1; i < table->end_scopes->meta.capacity; i++) {
        fprintf(fp, "| <%zu> %zu ", i, i);
    }   fprintf(fp, "\", width=5.0];\n\n\t");

    fprintf(fp, "node [shape=record, style=\"filled\", fillcolor=\"lightblue\"];\n\n\t");

    fprintf(fp, "current_scope [label=\"CURRENT\", shape=plaintext, fillcolor=\"orange\"];\n\n\t");

    fprintf(fp, "scope0 [label=\"{<f0>%p | <level>level: %u|{<prev_port>prev: %p}}\"];\n\t", 
                table->global_scope, table->global_scope->level, table->global_scope->prev);


    size_t scopes_cnt = 1;

    for (size_t i = 0; i < table->end_scopes->meta.size; i++) {
        size_t prev_global_scope_num = 0;

        Scope** cur_scope_ptr = (Scope**)STACK_IDX(i);
        Scope* cur_scope = *cur_scope_ptr;
        size_t end_scope_num = scopes_cnt;

        while (cur_scope != table->global_scope) {
            fprintf(fp, "scope%zu [label=\"{<f%u> %p | <f1>level: %u|{<prev_port>prev: %p}}\"];\n\t",
                        scopes_cnt, cur_scope->level, cur_scope, cur_scope->level, cur_scope->prev);

            prev_global_scope_num = scopes_cnt;

            cur_scope = cur_scope->prev;
            ++scopes_cnt;
        }

        fprintf(fp, "stack:%zu -> scope%zu [color=red, arrowhead=normal];\n\t", i, end_scope_num);

        for (size_t j = end_scope_num; j != prev_global_scope_num; j++) {
            fprintf(fp, "scope%zu:prev_port -> scope%zu [color=green, arrowhead=normal];\n\t",
                        j, j+1);
        }

        fprintf(fp, "scope%zu:prev_port -> scope%d [color=green, arrowhead=normal];\n\t",
                    prev_global_scope_num, 0);
    }

    Scope* cur_scope = table->current_scope;
    size_t prev_global_scope_num = 0;
    size_t end_scope_num = scopes_cnt;

    while (cur_scope != table->global_scope) {
        fprintf(fp, "scope%zu [label=\"{<f%u> %p | <f1>level: %u|{<prev_port>prev: %p}}\"];\n\t",
                    scopes_cnt, cur_scope->level, cur_scope, cur_scope->level, cur_scope->prev);

        prev_global_scope_num = scopes_cnt;

        cur_scope = cur_scope->prev;
        ++scopes_cnt;
    }

    for (size_t j = end_scope_num; j != prev_global_scope_num; j++) {
        fprintf(fp, "scope%zu:prev_port -> scope%zu [color=green, arrowhead=normal];\n\t",
                    j, j+1);
    }

    fprintf(fp, "scope%zu:prev_port -> scope%d [color=green, arrowhead=normal];\n\t",
                prev_global_scope_num, 0);

    fprintf(fp, "current_scope -> scope%zu[color=brown, arrowhead=normal];\n\t", end_scope_num);

    fprintf(fp, "\n}");

    fclose(fp);

    char* command = MultiStrCat(3, "dot -Tsvg ", file_name, " > img_sym_tab.svg");
    if (command == NULL) {
        fprintf(stderr, "CAN'T GET COMMAND\n");
        return;
    }

    system(command);

    free(command);
    command = NULL;

    return;
}