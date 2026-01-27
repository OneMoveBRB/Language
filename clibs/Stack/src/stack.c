#include "../include/stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef STACK_DEBUG
#include <time.h>

#include "../include/stack_dump.h"
#endif /* STACK_DEBUG */

#define STACK_IDX(idx) MovePtr(stack->data, idx, stack->meta.element_size)

const int CAPACITY_UPPER_LIMIT = 1000;

static StackErr_t StackRealloc(Stack_t* stack);
static void* MovePtr(void* base, size_t i, size_t element_size);

Stack_t* StackInit(size_t capacity, size_t element_size, const char* name) {
    if (capacity > CAPACITY_UPPER_LIMIT || element_size == 0) {
        return NULL;
    }

    Stack_t* stack = (Stack_t*)calloc(1, sizeof(Stack_t));

    stack->meta.capacity = capacity;
    stack->meta.size = 0;
    stack->meta.element_size = element_size;

    stack->data = calloc(stack->meta.capacity, element_size);
    if (stack->data == NULL) {
        FREE(stack);
        return NULL;
    }

#ifdef STACK_DEBUG
    stack->meta.magic_number = MAGIC_VALUE;
    stack->meta.stack_name = (name != NULL) ? name : "unnamed";
    stack->meta.created_time = time(NULL);
    stack->meta.last_modified = time(NULL);
    stack->meta.debugMemory = INIT;
    
    stack->LeftBorder = CANARY_VALUE;
    stack->RightBorder = CANARY_VALUE;

    fillPoison(stack->data, stack->meta.capacity * stack->meta.element_size);

    stack->meta.data_hash = calculateDataHash(stack);
#endif /* STACK_DEBUG */

    return stack;
}

static StackErr_t StackRealloc(Stack_t* stack) {
    HARD_STACK_VERIFY(stack, STACK_NULL_PTR);
    
    const size_t exp_multiplier = 2;

    size_t new_capacity = stack->meta.capacity * exp_multiplier;
    void* new_data = realloc(stack->data, new_capacity * stack->meta.element_size);
    if (new_data == NULL) {
        return STACK_OVERFLOW;
    }

    stack->meta.capacity = new_capacity;
    stack->data = new_data;

#ifdef STACK_DEBUG
    stack->meta.last_modified = time(NULL);
#endif /* STACK_DEBUG */

    HARD_STACK_VERIFY(stack, STACK_NULL_PTR);
    return STACK_OK;
}

StackErr_t StackDestroy(Stack_t** stack_ptr) {
    CHECK_STACK_PTR(stack_ptr, STACK_NULL_PTR);

    Stack_t* stack = *stack_ptr;
    SOFT_STACK_VERIFY(stack, STACK_NULL_PTR);

    if (stack->data != NULL) {
#ifdef STACK_DEBUG
        size_t total_size = stack->meta.capacity * stack->meta.element_size;
        fillPoison(stack->data, total_size);

        stack->meta.stack_name = "unnamed";
        stack->meta.last_modified = time(NULL);        
#endif /* STACK_DEBUG */
        stack->meta.capacity = 0;
        stack->meta.size = 0;
        stack->meta.element_size = 0;

        FREE(stack->data);
    }

    FREE(*stack_ptr);

    return STACK_OK;
}

StackErr_t StackPush(Stack_t* stack, const void* element) {
    HARD_STACK_VERIFY(stack, STACK_NULL_PTR);
    
    if (stack->meta.size + 1 >= stack->meta.capacity) {
        if (StackRealloc(stack) == STACK_OVERFLOW) {
            return STACK_DUMP(stack, STACK_OVERFLOW);
        }
    }

    memcpy(STACK_IDX(stack->meta.size), element, stack->meta.element_size);

    stack->meta.size++;

#ifdef STACK_DEBUG
    stack->meta.last_modified = time(NULL);

    stack->meta.data_hash = calculateDataHash(stack);
#endif /* STACK_DEBUG */

    HARD_STACK_VERIFY(stack, STACK_NULL_PTR);
    return STACK_OK;
}

void* StackPop(Stack_t* stack) {
    SOFT_STACK_VERIFY(stack, NULL);

    if (stack->meta.size == 0) {
        UNUSED(STACK_DUMP(stack, STACK_UNDERFLOW));
        return NULL;
    }
    
    void* dest = calloc(1, stack->meta.element_size);
    if (dest == NULL) {
        UNUSED(STACK_DUMP(stack, STACK_DEST_CALLOC_FAILED));
    } else {
        memcpy(dest, STACK_IDX(stack->meta.size - 1), stack->meta.element_size);
    }

    stack->meta.size--;

#ifdef STACK_DEBUG
    fillPoison(STACK_IDX(stack->meta.size), stack->meta.element_size);
    
    stack->meta.last_modified = time(NULL);
    
    stack->meta.data_hash = calculateDataHash(stack);
#endif /* STACK_DEBUG */

    SOFT_STACK_VERIFY(stack, NULL);
    return dest;
}

void* StackTop(Stack_t* stack) {
    SOFT_STACK_VERIFY(stack, NULL);

    if (stack->meta.size == 0) {
        UNUSED(STACK_DUMP(stack, STACK_UNDERFLOW));
        return NULL;
    }

    void* dest = calloc(1, stack->meta.element_size);
    if (dest == NULL) {
        UNUSED(STACK_DUMP(stack, STACK_DEST_CALLOC_FAILED));
    } else {
        memcpy(dest, STACK_IDX(stack->meta.size - 1), stack->meta.element_size);
    }

    SOFT_STACK_VERIFY(stack, NULL);
    return dest;
}

size_t StackSize(Stack_t* stack) {
    SOFT_STACK_VERIFY(stack, (size_t)-1);

    return stack->meta.size;
}

int StackEmpty(Stack_t* stack) {
    SOFT_STACK_VERIFY(stack, -1);

    return stack->meta.size == 0;
}

static void* MovePtr(void* base, size_t i, size_t element_size) {
    return (void*)((size_t)base + i * element_size);
}
