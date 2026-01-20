#ifndef AST_DUMP_H
#define AST_DUMP_H

#include "ast.h"

const char* GetStrOp(AST_ElemOperation operation);
const char* GetStrConst(ConstType type);

void DotVizualizeTree(const AST* ast, const char* file_name);

#endif /* AST_DUMP_H */