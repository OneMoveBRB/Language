#ifndef ASM_INSTRUCTIONS_H
#define ASM_INSTRUCTIONS_H

#include <stddef.h>

const int MAX_LEN = 256;

const char* asm_base =              "PUSH 0\n"
                                    "POPR RAX\n\n"
                                    "PUSH 0\n"
                                    "POPR RBX\n\n"
                                    "PUSH 0\n"
                                    "POPR RCX\n\n"
                                    "CALL main\n"
                                    "HLT\n\n\n";

const char* move_rax_by_one =       ": move_rax_by_one\n"
                                    "PUSH 1\n"
                                    "PUSHR RAX\n"
                                    "ADD\n"
                                    "POPR RAX\n"
                                    "RET\n\n\n";

const char* move_rax_by_one_call =  "CALL move_rax_by_one\n";

const char* enter_scope =           ": enter_scope\n"
                                    "PUSHR RBX\n"
                                    "POPM [RAX]\n"
                                    "PUSHR RAX\n"
                                    "POPR  RBX\n"
                                    "CALL move_rax_by_one\n"
                                    "RET\n\n\n";

const char* enter_scope_call =      "CALL enter_scope\n";

const char* exit_scope =            ": exit_scope\n"
                                    "PUSHR RBX\n"
                                    "POPR RAX\n"
                                    "PUSHM [RBX]\n"
                                    "POPR RBX\n"
                                    "RET\n\n\n";

const char* exit_scope_call =       "CALL exit_scope\n";

const char* set_rcx_offset =        ": set_rcx_offset\n"
                                    "PUSHR RBX\n"
                                    "ADD\n"
                                    "POPR RCX\n"
                                    "RET\n\n\n";

const char* get_rax_by_offset =     ": get_rax_by_offset\n"
                                    "CALL set_rcx_offset\n"
                                    "PUSHM [RCX]\n"
                                    "RET\n\n\n";

const char* bool_cmp =              " false_comparison_result#\n"
                                    "PUSH 1\n"
                                    "JMP truth_comparison_result#\n"
                                    ":   false_comparison_result#\n"
                                    "PUSH 0\n"
                                    ":   truth_comparison_result#\n\n";

const char* unary_bool_cmp =        "PUSH 1\n"
                                    "JA false_comparison_result#\n"
                                    "PUSH 1\n"
                                    "JMP truth_comparison_result#\n"
                                    ":   false_comparison_result#\n"
                                    "PUSH 0\n"
                                    ":   truth_comparison_result#\n\n";

const char* ram_push =              "POPM [RAX]\n"
                                    "CALL move_rax_by_one\n\n";

const char* ret =                   "RET\n\n";

const char* out =                   "OUT\n\n";

#endif /* ASM_INSTRUCTIONS_H */