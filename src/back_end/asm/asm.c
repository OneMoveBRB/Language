#include "../../../include/back_end/asm/asm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include "../../../include/back_end/asm/asm_dump.h"
#include "../../../include/io.h"
#include "../../../clibs/HashTable/include/hash_table.h"
#include "../../../clibs/HashTable/include/hash_table_dump.h"

#define FREE(ptr) free(ptr); ptr = NULL;
#define CHECK_FLAG(body)                    \
    if (flag != ASM_OK) {                   \
        body;                               \
        return flag;                        \
    }
#define PUSH_RET(err)                       \
    *error = err;                           \
    FREE(word);                             \
    return (size_t)-1;

static AssemblerErr_t FirstIteration(Assembler* assembler, HashTable_t* labels);
static AssemblerErr_t SecondIteration(Assembler* assembler, HashTable_t* labels, 
                                                            HashTable_t* instruction_templates);
static size_t AssemblerPush(Assembler* assembler, AssemblerErr_t* error, size_t i, char mode_choice, ...);
static size_t AssemblerSkipSpaces(Assembler* assembler, size_t i);

const int INITIAL_CAPACITY = 8;

InstructionMapping instruction_template_list[] = {
    {"IN"   ,	IN   },
    {"OUT"  ,	OUT  },
    {"PUSH" ,	PUSH },
    {"POP"  ,	POP  },
    {"PUSHR",	PUSHR},
    {"POPR" ,	POPR },
    {"ADD"  ,	ADD  },
    {"SUB"  ,	SUB  },
    {"MUL"  ,	MUL  },
    {"DIV"  ,   DIV  },
    {"SQRT" ,   SQRT },
    {"JA"   ,   JA   },
    {"JAE"  ,   JAE  },
    {"JB"   ,   JB   },
    {"JBE"  ,   JBE  },
    {"JE"   ,   JE   },
    {"JNE"  ,   JNE  },
    {"JMP"  ,   JMP  },
    {"CALL" ,   CALL },
    {"RET"  ,   RET  },
    {"HLT"  ,	HLT  },
    {"MAIN" ,   MAIN },
    {"RAX"  ,   RAX  },
    {"RBX"  ,   RBX  },
    {"RCX"  ,   RCX  },
    {"PUSHM",   PUSHM},
    {"POPM" ,   POPM }
};

size_t instruction_template_list_size = sizeof(instruction_template_list)/sizeof(InstructionMapping);

AssemblerErr_t AssemblerInit(Assembler* assembler, Buffer_t* buffer) {
    assert( assembler != NULL );
    assert( buffer    != NULL );

    assembler->input_file.file_name = NULL;
    assembler->input_file.buffer    = buffer;
    assembler->input_file.cur_line  = 0;
    assembler->bytecode             = NULL;
    assembler->start_ip             = 0;

    return ASM_OK;
}

AssemblerErr_t AssemblerDestroy(Assembler* assembler) {
    assert( assembler != NULL );

    assembler->input_file.file_name = NULL;
    assembler->input_file.cur_line  = (size_t)-1;

    if (assembler->input_file.buffer != NULL) {
        BufferDestroy(&assembler->input_file.buffer);
    }
    if (assembler->bytecode != NULL) {
        StackDestroy(&assembler->bytecode);
    }
    assembler->start_ip = (size_t)-1;

    return ASM_OK;
}

AssemblerErr_t AssemblerWriteFile(Assembler* assembler, const char* file_name, size_t* wrote_cnt) {
    assert( assembler != NULL );
    assert( file_name != NULL );

    Stack_t* bytecode = assembler->bytecode;

    FILE* fp = fopen(file_name, "wb");
    if (fp == NULL) {
        return ASM_FOPEN_FAILED;
    }

    if (fwrite(&assembler->start_ip, sizeof(size_t), 1, fp) != 1) {
        fclose(fp);
        return ASM_FWRITE_FAILED;
    }

    if (fwrite(&bytecode->meta.size, sizeof(size_t), 1, fp) != 1) {
        fclose(fp);
        return ASM_FWRITE_FAILED;
    }

    size_t num_of_wrote_elements = fwrite(bytecode->data, sizeof(int), bytecode->meta.size, fp);
    
    fclose(fp);

    if (num_of_wrote_elements != bytecode->meta.size) {
        return ASM_FWRITE_FAILED;
    }

    if (wrote_cnt != NULL) {
        *wrote_cnt = num_of_wrote_elements;
    }

    return ASM_OK;
}

AssemblerErr_t Translation(Assembler* assembler) {
    assert( assembler != NULL );

    AssemblerErr_t flag = ASM_OK;

    HashTable_t* instruction_templates = HashTableInit();
    if (instruction_templates == NULL) {
        return ASM_HASH_TABLE_FAILED;
    }

    for (size_t i = 0; i < instruction_template_list_size; i++) {
        HashTableErr_t hash_flag = HashTableInsert(instruction_templates, 
                        instruction_template_list[i].word, strlen(instruction_template_list[i].word), 
                        &instruction_template_list[i].id, sizeof(unsigned int));

        if (hash_flag != HASH_TABLE_OK) {
            HashTableDestroy(&instruction_templates);
            return ASM_HASH_TABLE_FAILED;
        }
    }

    HashTable_t* labels = HashTableInit();
    if (labels == NULL) {
        return ASM_HASH_TABLE_FAILED;
    }

    flag = FirstIteration(assembler, labels);
    CHECK_FLAG(
        HashTableDestroy(&instruction_templates);
        HashTableDestroy(&labels);
    );

    // DotVizualizeHashTable(labels, "labels.txt");    

    flag = SecondIteration(assembler, labels, instruction_templates);

    HashTableDestroy(&instruction_templates);
    HashTableDestroy(&labels);

    return flag;

}

static AssemblerErr_t FirstIteration(Assembler* assembler, HashTable_t* labels) {
    assert( assembler != NULL );
    assert( labels    != NULL );

    char* arr = (char*)assembler->input_file.buffer->data;
    assembler->input_file.cur_line = 1;

    size_t instruction_cnt = 0;

    for (size_t i = 0; arr[i] != '\0'; ) {
        i = AssemblerSkipSpaces(assembler, i);

        if (arr[i] == ':') {
            size_t start = i; ++i;
            i = AssemblerSkipSpaces(assembler, i);

            char* word = GetWord(arr + i);
            ASM_VERIFY(assembler, word != NULL, 'a', ASM_NO_LABEL, return ASM_IO_FAILED;);

            i += strlen(word);
            ASM_VERIFY(assembler, strlen(word) != 0, 'a', ASM_NO_LABEL, return ASM_IO_FAILED;);

            HashTableErr_t flag = HashTableInsert(labels, word, strlen(word), 
                                                          &instruction_cnt, sizeof(size_t));
            ASM_VERIFY(assembler, flag == HASH_TABLE_OK, 'h', flag, return ASM_HASH_TABLE_FAILED;);

            FREE(word);
            
            for (size_t j = start; j < i; j++) {
                arr[j] = ' ';
            }
        }

        i = AssemblerSkipSpaces(assembler, i);

        size_t offset = SkipWord(arr + i);
        if (offset != 0) {
            ++instruction_cnt;
        }

        i += offset;

    }

    assembler->input_file.cur_line = 1;

    return ASM_OK;
}

static AssemblerErr_t SecondIteration(Assembler* assembler, HashTable_t* labels, 
                                                            HashTable_t* instruction_templates) {
    assert( assembler             != NULL );
    assert( labels                != NULL );
    assert( instruction_templates != NULL );

    AssemblerErr_t flag = ASM_OK;
    char* arr = (char*)assembler->input_file.buffer->data;
    
    assembler->input_file.cur_line = 1;

    assembler->bytecode = StackInit(INITIAL_CAPACITY, sizeof(int), "bytecode.txt");
    ASM_VERIFY(assembler, assembler->bytecode != NULL, 's', -1, return ASM_STACK_FAILED;);

    for (size_t i = 0; arr[i] != '\0'; ) {
        // fprintf(stderr, "i : %zu\n", i);
        i = AssemblerSkipSpaces(assembler, i);

        InstructionType instruction_type = UNDEF;

        i = AssemblerPush(assembler, &flag, i, 'i', instruction_templates, &instruction_type);
        CHECK_FLAG()

        i = AssemblerSkipSpaces(assembler, i);

        switch (instruction_type) {
        case PUSH:{
            i = AssemblerPush(assembler, &flag, i, 'n');
            CHECK_FLAG()
            break;
        }
        
        case PUSHR:
        case POPR: {
            i = AssemblerPush(assembler, &flag, i, 'r', instruction_templates);
            CHECK_FLAG()
            break;
        }

        case JA:
        case JAE:
        case JB:
        case JBE:
        case JE:
        case JNE:
        case JMP:
        case CALL: {
            i = AssemblerPush(assembler, &flag, i, 'l', labels);
            CHECK_FLAG()
            break;
        }

        case PUSHM:
        case POPM: {
            i = AssemblerPush(assembler, &flag, i, 'm', instruction_templates);
            CHECK_FLAG()
            break;
        }

        case MAIN: {
            assembler->start_ip = assembler->bytecode->meta.size;
            CHECK_FLAG()
            break;
        }

        case UNDEF:
        case IN:
        case OUT:
        case POP:
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case SQRT:
        case RET:
        case HLT: {
            break;
        }
        
        default:
            assert(0);
            break;
        }

        i = AssemblerSkipSpaces(assembler, i);
    }

    assembler->input_file.cur_line = 1;

    return ASM_OK;
}

static size_t AssemblerSkipSpaces(Assembler* assembler, size_t i) {
    assert( assembler != NULL );
    
    const char* arr   = (const char*)assembler->input_file.buffer->data;
    const size_t size = assembler->input_file.buffer->size;
    size_t* cur_line   = &assembler->input_file.cur_line;

    for (; isspace(arr[i]) && i < size; i++) {
        if (arr[i] == '\n') *cur_line += 1;
    };

    while (arr[i] == ';') {
        for (; arr[i] != '\n' && arr[i] != '\0' && i < size; i++);

        for (; isspace(arr[i]) && i < size; i++) {
            if (arr[i] == '\n') *cur_line += 1;
        };
    }

    return i;
}

static size_t AssemblerPush(Assembler* assembler, AssemblerErr_t* error, size_t i, char mode_choice, ...) {
    assert( assembler != NULL );

    const char* arr = (const char*)assembler->input_file.buffer->data;

    char* word = GetWord(arr + i);
    ASM_VERIFY(assembler, word != NULL, 'i', -1, PUSH_RET(ASM_IO_FAILED););

    size_t word_len = strlen(word);
    ASM_VERIFY(assembler, word_len != 0, 'i', -1, PUSH_RET(ASM_IO_FAILED););

    i += word_len;

    va_list args;
    va_start(args, mode_choice);

    switch (mode_choice) {
    case 'i': {
        HashTable_t* instruction_templates = va_arg(args, HashTable_t*);
        InstructionType* instruction_type_ptr = va_arg(args, InstructionType*);

        const InstructionType* instruction_type = (InstructionType*)HashTableFind(instruction_templates, 
                                                                                  word, word_len);
        ASM_VERIFY(
            assembler, instruction_type != NULL, 'a', ASM_INVALID_INSTRUCTION,
            PUSH_RET(ASM_INVALID_INSTRUCTION);
        );

        if (instruction_type_ptr != NULL) {
            *instruction_type_ptr = *instruction_type;
        }

        StackErr_t flag = StackPush(assembler->bytecode, instruction_type);
        ASM_VERIFY(
            assembler, flag == STACK_OK, 's', flag,
            PUSH_RET(ASM_STACK_FAILED);
        );
        // fprintf(stderr, "Instruction: %d\n", instruction_type);

        break;
    }

    case 'l': {
        HashTable_t* labels = va_arg(args, HashTable_t*);

        const int* label_id = (int*)HashTableFind(labels, word, word_len);
        ASM_VERIFY(
            assembler, label_id != NULL, 'a', ASM_INVALID_LABEL,
            PUSH_RET(ASM_INVALID_LABEL);
        );

        StackErr_t flag = StackPush(assembler->bytecode, label_id);
        ASM_VERIFY(
            assembler, flag == STACK_OK, 's', flag,
            PUSH_RET(ASM_STACK_FAILED);
        );
        // fprintf(stderr, "Label: %d\n", label_id);

        break;
    }

    case 'm': {
        HashTable_t* instruction_templates = va_arg(args, HashTable_t*);

        ASM_VERIFY(
            assembler, word_len == 5, 'a', ASM_INVALID_REGISTER,
            PUSH_RET(ASM_INVALID_REGISTER);
        );
        ASM_VERIFY(
            assembler, word[0] == '[' && word[4] == ']', 'a', ASM_INVALID_REGISTER,
            PUSH_RET(ASM_INVALID_REGISTER);
        );

        const RegsType* reg_type = (RegsType*)HashTableFind(instruction_templates, 
                                                            word + 1, word_len - 2);
        ASM_VERIFY(
            assembler, reg_type != NULL, 'a', ASM_INVALID_REGISTER,
            PUSH_RET(ASM_INVALID_REGISTER);
        );

        StackErr_t flag = StackPush(assembler->bytecode, reg_type);
        ASM_VERIFY(
            assembler, flag == STACK_OK, 's', flag,
            PUSH_RET(ASM_STACK_FAILED);
        );
        // fprintf(stderr, "MemReg: %d\n", reg_type);

        break;
    }

    case 'n': {
        int number = atoi(word);
        ASM_VERIFY(
            assembler, (!(number == 0 && strcmp(word, "0") != 0)), 'a', ASM_INVALID_NUMBER,
            PUSH_RET(ASM_INVALID_NUMBER);
        );

        StackErr_t flag = StackPush(assembler->bytecode, &number);
        ASM_VERIFY(
            assembler, flag == STACK_OK, 's', flag,
            PUSH_RET(ASM_STACK_FAILED);
        );
        // fprintf(stderr, "Number: %d\n", number);

        break;
    }

    case 'r': {
        HashTable_t* instruction_templates = va_arg(args, HashTable_t*);

        const RegsType* reg_type = (RegsType*)HashTableFind(instruction_templates, word, word_len);
        ASM_VERIFY(
            assembler, reg_type != NULL, 'a', ASM_INVALID_REGISTER,
            PUSH_RET(ASM_INVALID_REGISTER);
        );

        StackErr_t flag = StackPush(assembler->bytecode, reg_type);
        ASM_VERIFY(
            assembler, flag == STACK_OK, 's', flag,
            PUSH_RET(ASM_STACK_FAILED);
        );
        // fprintf(stderr, "Reg: %d\n", reg_type);

        break;
    }

    default:
        assert(0);
        break;
    }

    va_end(args);
    
    FREE(word);

    return i;
}
