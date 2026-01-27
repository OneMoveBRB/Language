#include "list_private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

ListErr_t RealInsertAfter(List_t* list, size_t position, void* value) {
    assert( list != NULL );

    size_t prev_position = position;
    Node*  prev_node     = &list->arr.data[prev_position];

    size_t next_position = prev_node->next;
    Node*  next_node     = &list->arr.data[next_position];

    size_t free_position = list->free;
    Node*  free_node     = &list->arr.data[free_position];

    list->free = free_node->next;

    // free_node->value = value;
    free_node->value = calloc(1, list->arr.element_size);
    memcpy(free_node->value, value, list->arr.element_size);

    prev_node->next = free_position;
    free_node->prev = prev_position;

    next_node->prev = free_position;
    free_node->next = next_position;

    ++list->arr.size;

    if (list->arr.size + 1 == list->arr.capacity) {
        return ListReallocUp(list);
    }

    return LIST_OK;
}

ListErr_t RealDeleteAt(List_t* list, size_t position) {
    assert( list != NULL );

    size_t del_position = position;
    Node*  del_node     = &list->arr.data[del_position];
    
    size_t prev_position = del_node->prev;
    Node*  prev_node     = &list->arr.data[prev_position];
    
    size_t next_position = del_node->next;
    Node*  next_node     = &list->arr.data[next_position];

    // del_node->value = 0;
    FREE(del_node->value);
    del_node->prev = UNINITIALIZED;

    del_node->next = list->free;
    list->free = del_position;
    
    prev_node->next = next_position;
    next_node->prev = prev_position;

    --list->arr.size;

    ListReallocDown(list, NONE_CAPACITY);

    return LIST_OK;
}