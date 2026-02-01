#ifndef ASM_GENER_H
#define ASM_GENER_H

#include "back_end.h"

typedef struct AST AST;
typedef struct Buffer_t Buffer_t;

BackEndErr_t AssemblyCodeGeneration(AST* ast, Buffer_t* assembly_code);

#endif /* ASM_GENER_H */