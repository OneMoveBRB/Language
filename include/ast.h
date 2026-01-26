#ifndef AST_H
#define AST_H

#include <stddef.h>

typedef enum AST_ElemType {
    AST_ELEM_TYPE_UNDEFINED,
    AST_ELEM_TYPE_DECLARATION,
    AST_ELEM_TYPE_OPERATION,
    AST_ELEM_TYPE_VARIABLE,
    AST_ELEM_TYPE_CONST
} AST_ElemType;

typedef enum AST_ElemOperation {
    AST_ELEM_OPERATION_UNDEFINED,

    AST_ELEM_OPERATION_SENTINEL,

    AST_ELEM_OPERATION_ADD,
    AST_ELEM_OPERATION_SUB,
    AST_ELEM_OPERATION_MUL,
    AST_ELEM_OPERATION_DIV,

    AST_ELEM_OPERATION_LT,
    AST_ELEM_OPERATION_GT,
    AST_ELEM_OPERATION_LE,
    AST_ELEM_OPERATION_GE,
    AST_ELEM_OPERATION_EE,
    AST_ELEM_OPERATION_NE,

    AST_ELEM_OPERATION_LAND,
    AST_ELEM_OPERATION_LOR,

    AST_ELEM_OPERATION_ASSIGNMENT,

    AST_ELEM_OPERATION_IF,
    // AST_ELEM_OPERATION_ELIF,
    AST_ELEM_OPERATION_ELSE,
    AST_ELEM_OPERATION_WHILE,
    AST_ELEM_OPERATION_BREAK,

    AST_ELEM_OPERATION_CALL,
    AST_ELEM_OPERATION_RETURN
} AST_ElemOperation;

#ifndef CONST
#define CONST
typedef enum ConstType {
    CONST_TYPE_UNDEFINED,
    CONST_TYPE_SHORT,
    CONST_TYPE_INT,
    CONST_TYPE_LONG,
    CONST_TYPE_DOUBLE,
    CONST_TYPE_CHAR,
    CONST_TYPE_VOID
} ConstType;

typedef union ConstData {
    short   short_const;
    int     int_const;
    long    long_const;
    double  double_const;
    char    char_const;
} ConstData;

typedef struct Const {
    ConstType type;
    ConstData data;
} Const;
#endif /* CONST */

typedef union AST_ElemData {
    ConstType declaration_type;
    AST_ElemOperation operation;
    char* variable;
    Const constant;
} AST_ElemData;

typedef struct AST_Node {
    AST_ElemType type;
    AST_ElemData data;
    struct AST_Node* parent;
    struct AST_Node* left;
    struct AST_Node* right;
} AST_Node;

typedef struct AST {
    AST_Node* root;
    size_t size;
} AST;

typedef enum AST_Err_t {
    AST_OK
} AST_Err_t;

typedef AST_Err_t (*AST_NodeFunc)(AST_Node**);

AST*      AST_Init();
AST_Err_t AST_Destroy(AST** ast);
AST_Node* AST_NodeInit(AST_Node* parent, AST_Node* left, AST_Node* right, AST_ElemType type, ...);
AST_Err_t AST_NodeDestroy(AST_Node** node_ptr);

size_t PostorderTraversal(AST_Node* node, AST_NodeFunc func);
AST_Node** GetParentNodePointer(AST_Node* node);

#endif /* AST_H */
