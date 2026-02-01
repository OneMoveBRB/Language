#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>

typedef struct Buffer_t {
    void* data;
    size_t size;
    size_t capacity;
    size_t element_size;
} Buffer_t;

typedef enum BufferErr_t {
    BUFFER_OK                           = 0,
    BUFFER_OVERFLOW                     = 1,
    BUFFER_FREAD_FAILED                 = 2
} BufferErr_t;

Buffer_t* BufferInit(size_t capacity, size_t element_size);
BufferErr_t BufferRealloc(Buffer_t* buffer, size_t new_capacity);
BufferErr_t BufferDestroy(Buffer_t** buffer_ptr);

BufferErr_t BufferRelease(Buffer_t* buffer);
BufferErr_t BufferRead(Buffer_t* buffer, size_t count, FILE* fp);
BufferErr_t BufferPush(Buffer_t* buffer, const void* source, size_t count);

#endif /* BUFFER_H */