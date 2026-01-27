#include "../include/hash_table_dump.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "../../../include/io.h"

void DotVizualizeHashTable(const HashTable_t* table, const char* filename) {
    assert( table != NULL );
    assert( filename != NULL );

    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "DotVizualizeHashTable: fp == NULL\n");
        return; 
    }

    fprintf(fp, "digraph HashTable {\n\t");
    fprintf(fp, "rankdir=LR;\n\t");
    fprintf(fp, "node [shape=record, style=filled, fillcolor=lightblue];\n\t");
    fprintf(fp, "edge [fontsize=10,  color=green];\n\n\t");
    fprintf(fp, "node [shape=record, style=\"filled\", fillcolor=\"lightpink\"];\n\t");

    /* Here your code */
    fprintf(fp, "hashTable [label=\"<0> 0 ");
    for (size_t i = 1; i < table->buckets_capacity; i++) {
        fprintf(fp, "| <%zu> %zu ", i, i);
    }   fprintf(fp, "\", height=2.5];\n\n\t");

    fprintf(fp, "node [shape=record, style=\"filled\", fillcolor=\"lightblue\"];\n\t");
    fprintf(fp, "null_node [label=\"NULL\", shape=plaintext];\n\t");

    for (size_t i = 0; i < table->buckets_capacity; i++) {
        HashNode* cur_node = table->buckets[i];
        size_t cnt = 0;

        if (cur_node) {
            fprintf(
                fp, 
                "node_%zu_%zu [label=\"key = %p | key_size = %zu "
                "| data = %p | data_size = %zu "
                "| { <prev_port> prev: %p | <next_port> next: %p } \"];\n\t", 
                i, cnt, cur_node->key, cur_node->key_size,
                        cur_node->data, cur_node->data_size,
                        cur_node->prev, cur_node->next
            );

            fprintf(fp, "hashTable:<%zu> -> node_%zu_%zu;\n\t", i, i, cnt);

            ++cnt;
            cur_node = cur_node->next;
        } else {
            fprintf(fp, "hashTable:<%zu> -> null_node;\n\t", i);
            continue;
        }

        while (cur_node) {
            HashNode* next_node = cur_node->next;

            fprintf(
                fp, 
                "node_%zu_%zu [label=\"key = %p | key_size = %zu "
                "| data = %p | data_size = %zu "
                "| { <prev_port> prev: %p | <next_port> next: %p } \"];\n\t", 
                i, cnt, cur_node->key, cur_node->key_size,
                        cur_node->data, cur_node->data_size,
                        cur_node->prev, cur_node->next
            );

            if (cur_node->prev->next == cur_node) {
                fprintf(fp, "node_%zu_%zu:next_port -> node_%zu_%zu:prev_port [color=green, dir=both, arrowhead=normal];\n\t", i, cnt - 1, i, cnt);
            } else {
                fprintf(fp, "node_%zu_%zu:next_port -> null_node [color=red, arrowhead=normal];\n\t", i, cnt - 1);
                fprintf(fp, "node_%zu_%zu:prev_port -> null_node [color=red, arrowhead=normal];\n\t", i, cnt);
            }

            ++cnt;
            cur_node = next_node;
        }

        if (cur_node == NULL) {
            fprintf(fp, "node_%zu_%zu:next_port -> null_node;\n\t", i, cnt - 1);
        }
    }

    fprintf(fp, "\n}");

    fclose(fp);

    char* command = MultiStrCat(3, "dot -Tsvg ", filename, " > img.svg");
    if (command == NULL) {
        fprintf(stderr, "CAN'T GET COMMAND\n");
        return;
    }

    system(command);

    free(command);
    command = NULL;

    return;
}