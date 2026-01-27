#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list_private.h"
// #include "../stack/stack.h"

static int IndexCompare(const void* a, const void* b);

List_t* ListInit(const size_t element_size) {
    List_t* list = (List_t*)calloc(1, sizeof(List_t));
    if (list == NULL) {
        return NULL;
    }

    list->arr.size = 1;
    list->arr.capacity = FIRST_SIZE;
    list->arr.element_size = element_size;
    list->arr.data = (Node*)calloc(FIRST_SIZE, sizeof(Node));
    if (list->arr.data == NULL) {
        FREE(list);
        return NULL;
    }

    list->free = 1;

    for (size_t i = list->free; i < FIRST_SIZE - 1; i++) {
        list->arr.data[i].next = i + 1;
        list->arr.data[i].prev = UNINITIALIZED;
    }   
    
    list->arr.data[FakeElemIdx].next = FakeElemIdx;
    list->arr.data[FakeElemIdx].prev = FakeElemIdx;
    list->arr.data[FIRST_SIZE - 1].next = FakeElemIdx;
    list->arr.data[FIRST_SIZE - 1].prev = UNINITIALIZED;

    return list;
}

ListErr_t ListReallocUp(List_t* list) {
    assert( list != NULL );

    const size_t exp_multiplier = 2;
    
    Node* new_data = NULL;
    new_data = (Node*)realloc(list->arr.data, list->arr.capacity * exp_multiplier * sizeof(Node));
    if (new_data == NULL) {
        return LIST_OVERFLOW;
    }

    list->arr.capacity *= exp_multiplier;
    list->arr.data = new_data;
    new_data = NULL;

    if (list->free == FakeElemIdx) {
        list->free = list->arr.size + 1;
    } else {
        list->arr.data[list->free].next = list->arr.size + 1;
    }

    for (size_t i = list->arr.size + 1; i < list->arr.capacity - 1; i++) {
        list->arr.data[i].value = NULL;
        list->arr.data[i].next = i + 1;
        list->arr.data[i].prev = UNINITIALIZED;
    }

    list->arr.data[list->arr.capacity - 1].value = NULL;
    list->arr.data[list->arr.capacity - 1].next = FakeElemIdx;
    list->arr.data[list->arr.capacity - 1].prev = UNINITIALIZED;

    return LIST_OK;
}

ListErr_t ListReallocDown(List_t* list, size_t capacity) {
    assert( list != NULL );

    if (capacity >= list->arr.capacity) {
        return LIST_WRONG_CAPACITY;

    } else if (capacity == NONE_CAPACITY) {
        list->arr.capacity = list->arr.size + 2;
    } else {
        list->arr.capacity = capacity;
    }

    ListLinearization(list);

    list->arr.data = (Node*)realloc(list->arr.data, list->arr.capacity * sizeof(Node));

    list->arr.data[list->arr.capacity - 1].next = FakeElemIdx;

    return LIST_OK;
}

ListErr_t ListDestroy(List_t** list_ptr, void (*FreeValue)(void*)) {
    assert(  list_ptr != NULL );
    assert( *list_ptr != NULL );

    List_t* list = *list_ptr;

    for (size_t i = ListFront(list); i != FakeElemIdx; i = ListNext(list, i)) {
        if (FreeValue) {
            FreeValue(list->arr.data[i].value);
        }
        FREE(list->arr.data[i].value);
    }

    list->free = 0;

    list->arr.size = 0;
    list->arr.capacity = 0;
    list->arr.element_size = 0;
    FREE(list->arr.data);

    FREE(*list_ptr);

    return LIST_OK;
}

void* ListGet(List_t* list, size_t position) {
    assert( list != NULL );
    assert( position < list->arr.capacity );

    return list->arr.data[position].value;
}

size_t ListNext(const List_t* list, size_t position) {
    assert( list != NULL );
    assert( position < list->arr.capacity );

    return list->arr.data[position].next;
}

size_t ListPrev(const List_t* list, size_t position) {
    assert( list != NULL );
    assert( position < list->arr.capacity );

    return list->arr.data[position].prev;
}

size_t ListFront(const List_t* list) {
    assert( list != NULL );

    return list->arr.data[FakeElemIdx].next;
}

size_t ListEnd(const List_t* list) {
    assert( list != NULL );

    return list->arr.data[FakeElemIdx].prev;
}

size_t ListSize(const List_t* list) {
    assert( list != NULL );

    return list->arr.size - 1;
}

ListErr_t ListSet(List_t* list, size_t position, void* value) {
    assert( list != NULL );

    if (position + 1 >= list->arr.capacity) {
        return LIST_INDEX_OUT_OF_RANGE;
    }
    
    const size_t element_size = list->arr.element_size; 

    list->arr.data[position + 1].value = calloc(1, element_size);
    memcpy(list->arr.data[position + 1].value, value, element_size);

    return LIST_OK;
}

ListErr_t ListPushBack(List_t* list, void* value) {
    assert( list != NULL );

    return RealInsertAfter(list, ListEnd(list), value);
}

ListErr_t ListInsert(List_t* list, size_t position, void* value) {
    assert( list != NULL );

    size_t real_position = position + 1;

    if (real_position >= list->arr.capacity) {
        return LIST_INDEX_OUT_OF_RANGE;
    }

    if (list->arr.data[real_position].prev == UNINITIALIZED) {
        return LIST_INSERT_BEFORE_UNINITIALIZED;
    }

    return RealInsertAfter(list, list->arr.data[real_position].prev, value);
}

ListErr_t ListPop(List_t* list) {
    assert( list != NULL );

    return RealDeleteAt(list, ListEnd(list));
}

ListErr_t ListErase(List_t* list, size_t position) {
    assert( list != NULL );

    size_t real_position = position + 1;
    
    if (real_position >= list->arr.capacity) {
        return LIST_INDEX_OUT_OF_RANGE;
    }

    if (list->arr.data[real_position].prev == UNINITIALIZED) {
        return LIST_ERASE_UNINITIALIZED;
    }

    return RealDeleteAt(list, real_position);
}

ListErr_t ListLinearization(List_t* list) {
    assert( list != NULL );

    if (ListSize(list) == 0) {
        return LIST_OK;
    }

    const size_t element_size = list->arr.element_size;

    void* buffer = calloc(list->arr.size, element_size);
    if (buffer == NULL) {
        return LIST_LINEARIZATION_FAILED;
    }

    size_t buffer_pointer = 0;
    for (size_t i = ListFront(list); i != FakeElemIdx; i = ListNext(list, i)) {
        // buffer[buffer_pointer++] = ListGet(list, i);
        memcpy((char*)buffer + buffer_pointer*element_size, ListGet(list, i), element_size);
        buffer_pointer++;
        FREE(list->arr.data[i].value);
    }

    list->arr.data[FakeElemIdx].next = 1;
    list->arr.data[FakeElemIdx].prev = list->arr.size - 1;

    for (buffer_pointer = 0; buffer_pointer < list->arr.size - 1; buffer_pointer++) {
        // list->arr.data[buffer_pointer + 1].value = buffer[buffer_pointer];
        ListSet(list, buffer_pointer + 1, (char*)buffer + buffer_pointer*element_size);
        list->arr.data[buffer_pointer + 1].next = buffer_pointer + 2;
        list->arr.data[buffer_pointer + 1].prev = buffer_pointer;
    }

    list->arr.data[list->arr.size - 1].next = FakeElemIdx;

    list->free = list->arr.size;
    for (size_t i = list->free; i < list->arr.capacity; i++) {
        list->arr.data[i].value = NULL;
        list->arr.data[i].next = i + 1;
        list->arr.data[i].prev = UNINITIALIZED;
    }

    list->arr.data[list->arr.capacity - 1].next = FakeElemIdx;

    FREE(buffer);

    return LIST_OK;
}

#ifdef STACK_H
ListErr_t ListGroupNodes(List_t* list) {
    assert( list != NULL );

    size_t last_node_index = list->arr.capacity - 1;
    for (; list->arr.data[last_node_index].prev == UNINITIALIZED; last_node_index--);

    size_t free_size = list->arr.capacity - list->arr.size;

    size_t* buffer = (size_t*)calloc(free_size, sizeof(size_t));
    if (buffer == NULL) {
        return LIST_GROUP_FAILED;
    }

    size_t buffer_pointer = 0;
    for (size_t free_index = list->free; free_index != FakeElemIdx; free_index = ListNext(list, free_index)) {
        buffer[buffer_pointer++] = free_index;
    }

    qsort(buffer, free_size, sizeof(size_t), IndexCompare);


    list->free = buffer[0];
    for (buffer_pointer = 0; buffer_pointer < free_size - 1; buffer_pointer++) {
        size_t cur_position = buffer[buffer_pointer];
        size_t next_position = buffer[buffer_pointer + 1];

        list->arr.data[cur_position].next = next_position;        
    }
    list->arr.data[buffer[buffer_pointer]].next = FakeElemIdx;

    Stack_t* del_nodes = NULL;
    StackInit(&del_nodes, sizeof(size_t), FIRST_SIZE, "del_nodes");

    for (buffer_pointer = 0; buffer[buffer_pointer] < last_node_index; buffer_pointer++) {
        ListInsert(list, last_node_index - 1, ListGet(list, last_node_index - 1));
        StackPush(del_nodes, &last_node_index);
        
        for (last_node_index--; list->arr.data[last_node_index].prev == UNINITIALIZED; last_node_index--);
    }

    while (del_nodes->meta.size != 0) {
        StackPop(del_nodes, &last_node_index);
        ListErase(list, last_node_index - 1);
    }

    StackDestroy(del_nodes);
    FREE(buffer);

    return LIST_OK;
}

static int IndexCompare(const void* a, const void* b) {
    size_t idx1 = *(const size_t*)a;
    size_t idx2 = *(const size_t*)b;

    if (idx1 < idx2)
        return -1;

    else if (idx1 == idx2)
        return 0;

    return 1;
}
#endif // STACK_H