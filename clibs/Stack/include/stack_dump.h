#ifndef STACK_DUMP_H
#define STACK_DUMP_H

#ifndef STACK_DEBUG
#define STACK_DEBUG
#endif /* STACK_DEBUG */

#include "../include/stack.h"

StackErr_t StackVerify(Stack_t* stack);
StackErr_t StackDump(Stack_t* stack, StackErr_t error);

size_t calculateDataHash(const Stack_t* stack);
void fillPoison(void* data, size_t size);
const char* StackErrorMessage(StackErr_t error);

#endif /* STACK_DUMP_H */