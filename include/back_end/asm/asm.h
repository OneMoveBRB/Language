#ifndef ASM_H
#define ASM_H

#include <stddef.h>

#include "../../../clibs/Buffer/include/buffer.h"
#include "../../../clibs/Stack/include/stack.h"

// #define _DEBUG

#ifdef _DEBUG
#define ASM_DEBUG
#endif /* _DEBUG */ 

#ifdef ASM_DEBUG
#include "asm_dump.h"

#define STRINGIFY(x_) #x_
#define STR(x_) STRINGIFY(x_)

#define ASM_VERIFY(asm, expr, err_type, err, body)                          \
    asm->debug_memory = INIT;                                               \
    if (!(expr)) {                                                          \
        AssemblerDump(asm, err_type, err);                                  \
        body;                                                               \
    }

#ifndef VAR_INFO_STRUCT
#define VAR_INFO_STRUCT
#define INIT (VarInfo){__FILE__, __func__, __LINE__}
typedef struct VarInfo{
    const char* file;
    const char* func;
    int line;
} VarInfo;
#endif /* VAR_INFO_STRUCT */

#else
#define RELEASE
#define ASM_VERIFY(asm, expr, err_type, err, body)                          \
    if (!(expr)) {                                                          \
        AssemblerDump(asm, err_type, err);                                  \
        body;                                                               \
    }

#endif /* ASM_DEBUG */

typedef struct FileInfo {
    const char* file_name;
    Buffer_t* buffer;
    size_t cur_line;
} FileInfo;

typedef struct Assembler {
    FileInfo input_file;
    Stack_t* bytecode;
    size_t start_ip;
#ifdef ASM_DEBUG
    VarInfo debug_memory;
#endif /* ASM_DEBUG */
} Assembler;

typedef enum AssemblerErr_t {
    ASM_OK                  = 0,
    ASM_OVERFLOW            = 1,
    ASM_FOPEN_FAILED        = 2,
    ASM_FWRITE_FAILED       = 3,

    ASM_IO_FAILED           = 4,
    ASM_BUFFER_FAILED       = 5,
    ASM_STACK_FAILED        = 6,
    ASM_HASH_TABLE_FAILED   = 7,

    ASM_NO_LABEL            = 8,
    ASM_INVALID_INSTRUCTION = 9,
    ASM_INVALID_LABEL       = 10,
    ASM_INVALID_REGISTER    = 11,
    ASM_INVALID_NUMBER      = 12
} AssemblerErr_t;

typedef struct InstructionMapping {
    const char* word;
    unsigned int id;
} InstructionMapping;

typedef enum InstructionType {
    UNDEF   = 0,
    IN      = 1,
    OUT     = 2,
    PUSH    = 3,
    POP     = 4,
    PUSHR   = 5,
    POPR    = 6,
    ADD     = 7,
    SUB     = 8,
    MUL     = 9,
    DIV     = 10,
    SQRT    = 11,
    JA      = 12,
    JAE     = 13,
    JB      = 14,
    JBE     = 15,
    JE      = 16,
    JNE     = 17,
    JMP     = 18,
    CALL    = 19,
    RET     = 20,
    HLT     = 21,
    PUSHM   = 22,
    POPM    = 23,
    MAIN    = 24
} InstructionType;

typedef enum RegsType {
    RAX     = 1,
    RBX     = 2,
    RCX     = 3
} RegsType;

AssemblerErr_t AssemblerInit(Assembler* assembler, Buffer_t* buffer);
AssemblerErr_t AssemblerDestroy(Assembler* assembler);

AssemblerErr_t Translation(Assembler* assembler);
AssemblerErr_t AssemblerWriteFile(Assembler* assembler, const char* file_name, size_t* wrote_cnt);

#endif /* ASM_H */
