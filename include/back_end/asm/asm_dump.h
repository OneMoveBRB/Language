#ifndef ASM_DUMP_H
#define ASM_DUMP_H

typedef struct Assembler Assembler;

int AssemblerDump(Assembler* assembler, char err_type, ...);

#endif /* ASM_DUMP_H */