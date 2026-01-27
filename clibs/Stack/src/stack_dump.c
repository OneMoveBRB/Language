#include "../include/stack_dump.h"

#include <stdio.h>
#include <string.h>

typedef struct ErrorMapping {
    StackErr_t err;
    const char* str;
} ErrorMapping;

static size_t fnv1aHash(const void* data, size_t len, size_t seed);

static ErrorMapping error_table[] = {
    { STACK_OK,                 "STACK_OK"                 },
    { STACK_UNDERFLOW,          "STACK_UNDERFLOW"          },
    { STACK_OVERFLOW,           "STACK_OVERFLOW"           },
    { STACK_CANARY_CORRUPTED,   "STACK_CANARY_CORRUPTED"   },
    { STACK_DATA_CORRUPTED,     "STACK_DATA_CORRUPTED"     },
    { STACK_NULL_PTR,           "STACK_NULL_PTR"           },
    { STACK_INVALID_SIZE,       "STACK_INVALID_SIZE"       },
    { STACK_INVALID_CAPACITY,   "STACK_INVALID_CAPACITY"   },
    { STACK_MEMORY_ERROR,       "STACK_MEMORY_ERROR"       },
    { STACK_USE_AFTER_FREE,     "STACK_USE_AFTER_FREE"     },
    { STACK_DEST_CALLOC_FAILED, "STACK_DEST_CALLOC_FAILED" }
};

static size_t error_table_size = sizeof(error_table)/sizeof(ErrorMapping);

StackErr_t StackVerify(Stack_t* stack) {
    if (stack == NULL) return STACK_NULL_PTR;

    if (stack->meta.magic_number != MAGIC_VALUE) 
        return STACK_USE_AFTER_FREE;

    if ((stack->LeftBorder != CANARY_VALUE) || (stack->RightBorder != CANARY_VALUE)) 
        return STACK_CANARY_CORRUPTED;

    if (stack->meta.size > stack->meta.capacity) 
        return STACK_OVERFLOW;

    if (stack->meta.element_size == 0) 
        return STACK_INVALID_SIZE;

    size_t expected_data_hash = calculateDataHash(stack);
    if (stack->meta.data_hash != expected_data_hash) 
        return STACK_DATA_CORRUPTED;

    if (stack->data == NULL && stack->meta.capacity > 0) 
        return STACK_MEMORY_ERROR;

    return STACK_OK;
}

StackErr_t StackDump(Stack_t* stack, StackErr_t error) {
    const char* error_message = StackErrorMessage(error);

    FILE* debug_file = fopen(stack->meta.stack_name, "w");
    if (debug_file == NULL) {
        fprintf(stderr, "STACK Error: %s\nStackDump can't write the file[%s] for more details\n", 
                        error_message, stack->meta.stack_name);
        return STACK_NULL_PTR;
    }

    fprintf(debug_file, "=========================== STACK DUMP ==========================\n");

    VarInfo debugInfo = stack->meta.debugMemory;
    fprintf(debug_file, "Verify failed!\nReason: %s from %s at %s:%d\n\n", error_message, 
                             debugInfo.func, debugInfo.file, debugInfo.line);

    fprintf(debug_file, "Stack addres: %p\n",   stack);
    fprintf(debug_file, "Name: %s\n",           stack->meta.stack_name);
    fprintf(debug_file, "Capacity: %zu\n",      stack->meta.capacity);
    fprintf(debug_file, "Size: %zu\n",          stack->meta.size);
    fprintf(debug_file, "Element size: %zu\n\n",  stack->meta.element_size);
    fprintf(debug_file, "Created: %s",          ctime(&stack->meta.created_time));
    fprintf(debug_file, "Last modified: %s\n",  ctime(&stack->meta.last_modified));

    fprintf(debug_file, "Canary front: 0x%016u (expected: 0x%016u)\n", 
            stack->LeftBorder, CANARY_VALUE);
    fprintf(debug_file, "Canary back:  0x%016u (expected: 0x%016u)\n", 
            stack->RightBorder, CANARY_VALUE);
    fprintf(debug_file, "Magic number: 0x%016zu (expected: 0x%016u)\n", 
            stack->meta.magic_number, MAGIC_VALUE);

    fprintf(debug_file, "Data pointer: %p\n",   stack->data);
    fprintf(debug_file, "Data hash:    0x%08zu\n\n", stack->meta.data_hash);

    return error;
}

size_t calculateDataHash(const Stack_t* stack) {
    if (stack->data == NULL || stack->meta.size == 0)  return 0;
    return fnv1aHash(stack->data, 
                      stack->meta.size * stack->meta.element_size, 
                      HASH_SEED);
}

static size_t fnv1aHash(const void* data, size_t len, size_t seed) {
    const unsigned char* bytes = (const unsigned char*)data;
    size_t hash = seed;
    
    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 0x01000193;
    }
    
    return hash;
}

void fillPoison(void* data, size_t size) {
    memset(data, 0xBAD, size);
}

const char* StackErrorMessage(StackErr_t error) {
    for (size_t i = 0; i < error_table_size; ++i) {
        if (error_table[i].err == error) 
            return error_table[i].str;
    }
    return "NAN";
}
