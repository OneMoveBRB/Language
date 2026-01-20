#ifndef SYNTAX_H
#define SYNTAX_H

#include <stddef.h>

typedef struct AST AST;
typedef struct List_t List_t;

AST* SyntaxAnalysis(List_t* tokens);

#endif /* SYNTAX_H */