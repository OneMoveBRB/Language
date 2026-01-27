#ifndef LIST_PRIVATE_H
#define LIST_PRIVATE_H

#include <stddef.h>

#include "list.h"

#define FREE(ptr) free(ptr); ptr = NULL;

const int FIRST_SIZE    = 8;
const int UNINITIALIZED = 0xBAD;
const int NONE_CAPACITY = 0;
const int FakeElemIdx   = 0;

typedef struct Node {
    void* value;
    size_t next;
    size_t prev;
} Node;

typedef struct Nodes {
    Node* data;
    size_t size;
    size_t capacity;
    size_t element_size;
} Nodes;

typedef struct List_t {
    Nodes  arr;
    size_t free;
} List_t;

ListErr_t RealInsertAfter(List_t* list, size_t position, void* value);
ListErr_t RealDeleteAt(List_t* list, size_t position);

#endif /* LIST_PRIVATE_H */