#include <stdio.h>

#include "include/ast.h"
#include "include/front_end/front_end.h"

const char* file_name = "syntax_test.c";

int main() {
    AST* ast = NULL;

    FrontEnd(&ast, file_name);

    AST_Destroy(&ast);

    return 0;
}