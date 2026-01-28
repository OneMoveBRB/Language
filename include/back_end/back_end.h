#ifndef BACK_END_H
#define BACK_END_H

typedef enum BackEndErr_t {
    BACK_END_OK,
    BACK_END_ERROR,
    BACK_END_BUFFER_FAILED,
    BACK_END_SYMBOL_TABLE_FAILED
} BackEndErr_t;

typedef struct AST AST;

BackEndErr_t CodeGeneration(AST* ast);

#endif /* BACK_END_H */