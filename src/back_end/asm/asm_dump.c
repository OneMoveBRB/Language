#include "../../../include/back_end/asm/asm_dump.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "../../../include/back_end/asm/asm.h"
#include "../../../include/io.h"
#include "../../../clibs/Stack/include/stack_dump.h"
#include "../../../clibs/HashTable/include/hash_table.h"
#include "../../../clibs/HashTable/include/hash_table_dump.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define PURPLE "\033[95m"

#define va_arg_enum(type) ((type)va_arg(args, int))

typedef enum ErrorType {
    ERROR_TYPE_ASM          = 0,
    ERROR_TYPE_IO           = 1,
    ERROR_TYPE_BUFFER       = 2,
    ERROR_TYPE_HASH_TABLE   = 3
} ErrorType;

typedef struct ErrorMapping {
    int err;
    const char* str;
    ErrorType err_type;
} ErrorMapping;

static const char* AssemblerErrorMessage(ErrorType err_type, ...);
static const char* GetFileLine(FileInfo* file_info);
#ifdef ASM_DEBUG
static void PrintStack(Stack_t* stack, const char* stack_name, FILE* fp);
static void PrintBuffer(Buffer_t* buffer, const char* buffer_name, FILE* fp);
static const char* assembler_file_name = "assembler.txt";
#endif /* ASM_DEBUG */

static ErrorMapping error_table[] = {
    {ASM_FOPEN_FAILED,               "ASM_FOPEN_FAILED",               ERROR_TYPE_ASM       },
    {ASM_FWRITE_FAILED,              "ASM_FWRITE_FAILED",              ERROR_TYPE_ASM       },
    {ASM_NO_LABEL,                   "ASM_NO_LABEL",                   ERROR_TYPE_ASM       },
    {ASM_INVALID_INSTRUCTION,        "ASM_INVALID_INSTRUCTION",        ERROR_TYPE_ASM       },
    {ASM_INVALID_LABEL,              "ASM_INVALID_LABEL",              ERROR_TYPE_ASM       },
    {ASM_INVALID_REGISTER,           "ASM_INVALID_REGISTER",           ERROR_TYPE_ASM       },
    {ASM_INVALID_NUMBER,             "ASM_INVALID_NUMBER",             ERROR_TYPE_ASM       },
    {IO_FILE_NOT_REGULAR,            "IO_FILE_NOT_REGULAR",            ERROR_TYPE_IO        },
    {IO_FILE_NOT_FOUND_OR_NO_ACCESS, "IO_FILE_NOT_FOUND_OR_NO_ACCESS", ERROR_TYPE_IO        },
    {BUFFER_OVERFLOW,                "BUFFER_OVERFLOW",                ERROR_TYPE_BUFFER    },
    {BUFFER_FREAD_FAILED,            "BUFFER_FREAD_FAILED",            ERROR_TYPE_BUFFER    },
    {HASH_TABLE_CANT_CALLOC_NODE,    "HASH_TABLE_CANT_CALLOC_NODE",    ERROR_TYPE_HASH_TABLE},
    {HASH_TABLE_BUCKETS_OVERFLOW,    "HASH_TABLE_BUCKETS_OVERFLOW",    ERROR_TYPE_HASH_TABLE},
    {HASH_TABLE_NODE_MEMORY_LEAK,    "HASH_TABLE_NODE_MEMORY_LEAK",    ERROR_TYPE_HASH_TABLE}
};

static size_t error_table_size = sizeof(error_table)/sizeof(ErrorMapping);

int AssemblerDump(Assembler* assembler, char err_type, ...) {
    assert( assembler != NULL );

    int error = -1;
    const char* error_message = NULL;

    va_list args;
    va_start(args, err_type);

    switch (err_type) {
    case 'a': {
        AssemblerErr_t assembler_error = va_arg_enum(AssemblerErr_t);
        error_message = AssemblerErrorMessage(ERROR_TYPE_ASM, assembler_error);
        // assert( error_message != NULL );

        error = assembler_error;

        break;
    }

    case 'b': {
        BufferErr_t buffer_error = va_arg_enum(BufferErr_t);
        error_message = AssemblerErrorMessage(ERROR_TYPE_BUFFER, buffer_error);
        // assert( error_message != NULL );

        error = buffer_error;

        break;
    }

    case 'h': {
        HashTableErr_t hash_table_error = va_arg_enum(HashTableErr_t);
        error_message = AssemblerErrorMessage(ERROR_TYPE_HASH_TABLE, hash_table_error);
        // assert( error_message != NULL );
        
        error = hash_table_error;

        break;
    }

    case 'i': {
        IOErr_t io_error = va_arg_enum(IOErr_t);
        error_message = AssemblerErrorMessage(ERROR_TYPE_IO, io_error);
        // assert( error_message != NULL );

        error = io_error;

        break;
    }

    case 's': {
        StackErr_t stack_error = va_arg_enum(StackErr_t);
        error_message = StackErrorMessage(stack_error);
        // assert( error_message != NULL );

        error = stack_error;

        break;
    }
            
    default:
        assert(0);
        break;
    }

    va_end(args);


    fprintf(stderr, 
        "=========================== "GREEN"ASSEMBLER VERIFY FAILED"RESET" ==========================\n"
        RED "Error" RED ": " PURPLE "%s" RESET " at %s:%zu\n", 
        error_message, assembler->input_file.file_name, assembler->input_file.cur_line
    );

    const char* line = GetFileLine(&assembler->input_file);
    if (err_type == 'a' && error == ASM_NO_LABEL) {
        fprintf(stderr, "   %zu |    %s " RED "..." RESET " << must be a register after %s\n", 
            assembler->input_file.cur_line, line, line);
    } else if (err_type == 'a' && error == ASM_INVALID_INSTRUCTION) {
        fprintf(stderr, "   %zu |   " RED "%s" RESET " << invalid instruction\n", 
            assembler->input_file.cur_line, line);
    } else if (err_type == 'a' && error == ASM_INVALID_LABEL) {
        fprintf(stderr, "   %zu |   " RED "%s" RESET " << invalid label\n", 
            assembler->input_file.cur_line, line);
    } else if (err_type == 'a' && error == ASM_INVALID_REGISTER) {
        fprintf(stderr, "   %zu |   " RED "%s" RESET " << invalid register\n", 
            assembler->input_file.cur_line, line);
    } else if (err_type == 'a' && error == ASM_INVALID_NUMBER) {
        fprintf(stderr, "   %zu |   " RED "%s" RESET " << invalid number\n", 
            assembler->input_file.cur_line, line);
    } 
    free((char*)line); line = NULL;

#ifdef ASM_DEBUG
    FILE* debug_fp = fopen(assembler_file_name, "w");
    if (debug_fp == NULL) {
        debug_fp = stderr;
    }

    fprintf(debug_fp, "=========================== ASSEMBLER DUMP ==========================\n");

    VarInfo debug_info = assembler->debug_memory;
    fprintf(debug_fp, RED "%s" RESET " from %s at %s:%d\n\n", error_message,
            debug_info.func, debug_info.file, debug_info.line);

    fprintf(debug_fp,
        "############# FILE_INFO #############\n"
        "File_name: %s\tCur_line: %zu\n\n",
        assembler->input_file.file_name, assembler->input_file.cur_line
    );

    fprintf(debug_fp,
        "############# BUFFER #############\n"
        "All information in buffer.txt\n\n"
    );

    FILE* buffer_fp = fopen("buffer.txt", "w");
    if (buffer_fp == NULL) {
        buffer_fp = debug_fp;
    }

    if (assembler->input_file.buffer == NULL) {
        fprintf(buffer_fp, "Buffer: %p\n", assembler->input_file.buffer);
    } else {
        PrintBuffer(assembler->input_file.buffer, "buffer", buffer_fp);
    }

    if (buffer_fp != debug_fp) {
        fclose(buffer_fp);
    }

    fprintf(debug_fp,
        "############# BYTECODE_STACK #############\n"
        "All information in bytecode_stack.txt\n\n"
    );

    FILE* bytecode_stack_fp = fopen("bytecode_stack.txt", "w");
    if (bytecode_stack_fp == NULL) {
        bytecode_stack_fp = debug_fp;
    }

    if (assembler->bytecode == NULL) {
        fprintf(bytecode_stack_fp, "Bytecode_stack: %p\n", assembler->bytecode);
    } else {
        PrintStack(assembler->bytecode, "bytecode_stack", bytecode_stack_fp);
    }

    if (bytecode_stack_fp != debug_fp) {
        fclose(bytecode_stack_fp);
    }

    fprintf(debug_fp,
        "############# START_IP #############\n"
        "Start_ip: %zu\n\n\n", assembler->start_ip
    );

    fclose(debug_fp);
#endif /* ASM_DEBUG */
    return error;
}

static const char* AssemblerErrorMessage(ErrorType err_type, ...) {
    va_list args;
    va_start(args, err_type);

    int error = -1;
    switch (err_type) {
    case ERROR_TYPE_ASM:
        error = va_arg_enum(AssemblerErr_t);
        break;

    case ERROR_TYPE_IO:
        error = va_arg_enum(IOErr_t);
        break;
    
    case ERROR_TYPE_BUFFER:
        error = va_arg_enum(BufferErr_t);
        break;

    case ERROR_TYPE_HASH_TABLE:
        error = va_arg_enum(HashTableErr_t);
        break;
    
    default:
        assert(0);
        break;
    }

    va_end(args);

    for (size_t i = 0; i < error_table_size; ++i) {
        if (error_table[i].err_type == err_type && error_table[i].err == error) {
            return error_table[i].str;
        }
    }
    return NULL;
}

static const char* GetFileLine(FileInfo* file_info) {
    assert( file_info != NULL );

    size_t line_num = 1;
    const char* line_begin_ptr = (const char*)file_info->buffer->data;

    while (file_info->cur_line != line_num) {
        if (*line_begin_ptr == '\0') {
            break;
        }
        if (*line_begin_ptr == '\n') {
            ++line_num;
        }
        ++line_begin_ptr;
    }

    const char* line_end_ptr = line_begin_ptr;
    while (*line_end_ptr != '\n' && *line_end_ptr != '\0') {
        ++line_end_ptr;
    }
    
    return strndup(line_begin_ptr, (size_t)line_end_ptr - (size_t)line_begin_ptr);
}

#ifdef ASM_DEBUG
static void PrintStack(Stack_t* stack, const char* stack_name, FILE* fp) {
    assert( stack       != NULL );
    assert( stack_name  != NULL );
    assert( fp          != NULL );

    fprintf(fp,
        "############# %s #############\n"
        "Element_size: %zu\n" 
        "Size: %zu\tCapacity: %zu\n", 
    stack_name, stack->meta.element_size, stack->meta.size, stack->meta.capacity);

    fprintf(fp, "Data: ");

    int* stack_data = (int*)stack->data;
    for (size_t i = 0; i < StackSize(stack); i++) {
        fprintf(fp, "%d ", stack_data[i]);
    }   fprintf(fp, "\n\n");
    
    return;
}

static void PrintBuffer(Buffer_t* buffer, const char* buffer_name, FILE* fp) {
    assert( buffer       != NULL );
    assert( buffer_name  != NULL );
    assert( fp           != NULL );

    fprintf(fp,
        "############# %s #############\n"
        "Element_size: %zu\n" 
        "Size: %zu\tCapacity: %zu\n", 
    buffer_name, buffer->element_size, buffer->size, buffer->capacity);

    fprintf(fp, "Data: \n");

    char* buffer_data = (char*)buffer->data;
    for (size_t i = 0; i < buffer->size; i++) {
        fprintf(fp, "%c", buffer_data[i]);
    }   fprintf(fp, "\n\n");
    
    return;
}
#endif /* ASM_DEBUG */
