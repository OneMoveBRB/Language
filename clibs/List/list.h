#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct List_t List_t;

typedef enum ListErr_t {
    LIST_OK,
    LIST_OVERFLOW,
    LIST_WRONG_CAPACITY,
    LIST_ALLOCATION_FAILED,
    LIST_GROUP_FAILED,
    LIST_LINEARIZATION_FAILED,
    LIST_INSERT_BEFORE_UNINITIALIZED,
    LIST_INSERT_AFTER_UNINITIALIZED,
    LIST_INDEX_OUT_OF_RANGE,
    LIST_ERASE_UNINITIALIZED,
    LIST_OUTPUT_FAILED
} ListErr_t;

List_t* ListInit(const size_t element_size);
ListErr_t ListReallocUp(List_t* list);
ListErr_t ListReallocDown(List_t* list, size_t capacity);
ListErr_t ListDestroy(List_t** list, void (*FreeValue)(void*));

void* ListGet(List_t* list, size_t position);

size_t ListNext(const List_t* list, size_t position);
size_t ListPrev(const List_t* list, size_t position);
size_t ListFront(const List_t* list);
size_t ListEnd(const List_t* list);
size_t ListSize(const List_t* list);

ListErr_t ListSet(List_t* list, size_t position, void* value);

ListErr_t ListPushBack(List_t* list, void* value);
ListErr_t ListInsert(List_t* list, size_t position, void* value);
ListErr_t ListPop(List_t* list);
ListErr_t ListErase(List_t* list, size_t position);

ListErr_t ListLinearization(List_t* list);
ListErr_t ListGroupNodes(List_t* list);

#endif /* LIST_H */