#ifndef IO_H
#define IO_H

#include <stddef.h>

typedef enum IOErr_t {
    IO_OK                           = 0,
    IO_FILE_NOT_REGULAR             = 1,
    IO_FILE_NOT_FOUND_OR_NO_ACCESS  = 2,
    IO_FOPEN_FAILED                 = 3,
    IO_BUFFER_FAILED                = 4
} IOErr_t;

typedef struct Buffer_t Buffer_t;

IOErr_t BufferGet(Buffer_t* buffer, const char* file_name);
IOErr_t GetFileSize(const char* file_name, size_t* file_size);

char* GetWord(const char* str);
size_t SkipWord(const char* str);

char* MultiStrCat(size_t count, ...);

#endif /* IO_H */