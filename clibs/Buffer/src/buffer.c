#include "../include/buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define FREE(ptr) free(ptr); ptr = NULL;
#define BUFFER_IDX(idx) MovePtr(buffer->data, idx, buffer->element_size)

const int EXP_MUL = 2;

static void* MovePtr(void* base, size_t i, size_t element_size);

/* capacity = 0 - valid */
Buffer_t* BufferInit(size_t capacity, size_t element_size) {
    Buffer_t* buffer = (Buffer_t*)calloc(1, sizeof(Buffer_t));
    if (buffer == NULL) {
        fprintf(stderr, "BufferInit: buffer == NULL\n");
        return NULL;
    }

    buffer->size = 0;
    buffer->capacity = capacity;
    buffer->element_size = element_size;

    if (capacity != 0) {
        buffer->data = calloc(buffer->capacity, element_size);
        if (buffer->data == NULL) {
            fprintf(stderr, "BufferInit: buffer->data == NULL\n");
            FREE(buffer);
            return NULL;
        }
    } else {
        buffer->data = NULL;
    }

    return buffer;
}

BufferErr_t BufferRealloc(Buffer_t* buffer, size_t new_capacity) {
    assert( buffer != NULL );

    if (new_capacity == 0) {
        new_capacity = buffer->capacity * EXP_MUL;
    }
    void* new_data = realloc(buffer->data, new_capacity * buffer->element_size);
    if (new_data == NULL) {
        return BUFFER_OVERFLOW;
    }

    buffer->capacity = new_capacity;
    buffer->data = new_data;

    memset(BUFFER_IDX(buffer->capacity - 1), 0, buffer->element_size);

    return BUFFER_OK;
}

BufferErr_t BufferDestroy(Buffer_t** buffer_ptr) {
    assert( buffer_ptr != NULL );
    assert( *buffer_ptr != NULL );

    Buffer_t* buffer = *buffer_ptr;

    buffer->size = 0;
    buffer->capacity = 0;
    buffer->element_size = 0;
    FREE(buffer->data);
    FREE(*buffer_ptr);

    return BUFFER_OK;
}

/* saves the actual data size of the buffer + zero element */
BufferErr_t BufferRelease(Buffer_t* buffer) {
    assert( buffer != NULL );

    return BufferRealloc(buffer, buffer->size + 1);
}

/* read count objects to buffer from fp */
BufferErr_t BufferRead(Buffer_t* buffer, size_t count, FILE* fp) {
    assert( buffer    != NULL );
    assert( fp != NULL );

    BufferErr_t flag = BUFFER_OK;

    flag = BufferRealloc(buffer, count + 1);
    if (flag != BUFFER_OK) {
        return flag;
    }

    size_t read_count = fread(buffer->data, buffer->element_size, count, fp);
    if (read_count != count) {
        return BUFFER_FREAD_FAILED;
    }

    memset(BUFFER_IDX(count), 0, buffer->element_size);
    buffer->size = count + 1;
    
    return BUFFER_OK;
}

BufferErr_t BufferPush(Buffer_t* buffer, const void* source, size_t count) {
    assert( buffer != NULL );
    assert( source != NULL );
    assert( count % buffer->element_size == 0 );

    size_t offset = count / buffer->element_size;
    if (buffer->size + offset + 1 >= buffer->capacity) {
        BufferRealloc(buffer, buffer->capacity + 2*offset + 2);
    }

    memcpy(BUFFER_IDX(buffer->size), source, count);
    buffer->size += offset;

    return BUFFER_OK;
}

static void* MovePtr(void* base, size_t i, size_t element_size) {
    return (void*)((size_t)base + i * element_size);
}
