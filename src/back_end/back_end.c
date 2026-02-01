#include "../../include/back_end/back_end.h"

#include <stdio.h>
#include <assert.h>

#include "../../include/ast/ast.h"
#include "../../include/back_end/asm_gener.h"
#include "../../include/back_end/asm/asm.h"
#include "../../clibs/Buffer/include/buffer.h"

#define CHECK_FLAG(body)                        \
    if (flag != ASM_OK) {                       \
        fprintf(stderr, "%d\n", flag);          \
        body;                                   \
        return flag;                            \
    }

const char* output_file_name = "bytecode.bin";

static AssemblerErr_t ByteCodeGeneration(Buffer_t* assembly_code);

BackEndErr_t BackEnd(AST* ast) {
    assert( ast != NULL );

    Buffer_t* assembly_code = BufferInit(0, sizeof(char));
    if (assembly_code == NULL) {
        return BACK_END_BUFFER_FAILED;
    }

    BackEndErr_t flag = AssemblyCodeGeneration(ast, assembly_code);
    
    BufferRelease(assembly_code);

    ByteCodeGeneration(assembly_code); //TODO - error handler

    // BufferDestroy(&assembly_code); // Destroy in assembler

    return BACK_END_OK;
}

static AssemblerErr_t ByteCodeGeneration(Buffer_t* assembly_code) {
    assert( assembly_code != NULL );
    
    Assembler assembler = {0};

    AssemblerErr_t flag = ASM_OK;

    flag = AssemblerInit(&assembler, assembly_code);
    CHECK_FLAG(AssemblerDestroy(&assembler););

    flag = Translation(&assembler);
    CHECK_FLAG(AssemblerDestroy(&assembler););

    size_t wrote_cnt = 0;
    flag = AssemblerWriteFile(&assembler, output_file_name, &wrote_cnt);
    CHECK_FLAG(AssemblerDestroy(&assembler););

    AssemblerDestroy(&assembler);

    return flag;
}