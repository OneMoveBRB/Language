#include "../include/io.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include <sys/stat.h>

#ifdef __linux__
    #include <sys/types.h>
    #include <unistd.h>
    #define STAT(f, s) stat(f, s)
#else
    #define STAT(f, s) _stat(f, s)
    #define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif

#include "../../Buffer/include/buffer.h"

IOErr_t BufferGet(Buffer_t* buffer, const char* file_name) {
    assert( buffer != NULL );
    assert( file_name != NULL );

    FILE* fp = fopen(file_name, "r");
    if (fp == NULL) {
        return IO_FOPEN_FAILED;
    }

    size_t file_size = 0;
    IOErr_t flag = GetFileSize(file_name, &file_size);
    if (flag != IO_OK) {
        fclose(fp);
        return flag;
    }

    if (BufferRead(buffer, file_size, fp) != BUFFER_OK) {
        fclose(fp);
        return IO_BUFFER_FAILED;
    }

    fclose(fp);

    return IO_OK;
}

IOErr_t GetFileSize(const char* file_name, size_t* file_size) {
    assert( file_name != NULL );

	struct stat fileStatbuff;
	if ((STAT(file_name, &fileStatbuff) != 0)) {
		return IO_FILE_NOT_FOUND_OR_NO_ACCESS;
	}

    if (!S_ISREG(fileStatbuff.st_mode)) {
        return IO_FILE_NOT_REGULAR;
    }

    *file_size = (size_t)fileStatbuff.st_size;

	return IO_OK;
}

char* GetWord(const char* str) {
    assert( str != NULL );

    size_t offset = SkipWord(str);
    if (offset == 0) {
        return NULL;
    }

    return strndup(str, offset);
}

size_t SkipWord(const char* str) {
    assert( str != NULL );

    size_t idx = 0;

    for (; !isspace(str[idx]) && str[idx] != '\0'; ++idx);

    return idx;
}

char* MultiStrCat(size_t count, ...) {
    size_t size = 0;

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; i++) {
        size += strlen(va_arg(args, char*));
    }

    va_end(args);

    char* str = (char*)calloc(size + 1, sizeof(char));
    if (str == NULL) {
        return NULL;
    }

    va_start(args, count);

    for (size_t i = 0; i < count; i++) {
        strcat(str, va_arg(args, char*));
    }

    va_end(args);

    return str;
}