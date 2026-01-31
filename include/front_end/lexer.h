#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef struct List_t List_t;

size_t LexicalAnalysis(const char* s, List_t* tokens);

#endif /* LEXER_H */
