#include "../../include/ast/ast.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../../include/utils.h"

#define va_arg_enum(type)   ((type)va_arg(args, int))
#define EmptyNodeInit       AST_NodeInit(NULL, NULL, NULL, AST_ELEM_TYPE_UNDEFINED)

AST* AST_Init() {
    AST* ast = (AST*)calloc(1, sizeof(AST));
    if (ast == NULL) {
        return NULL;
    }

    ast->root = NULL;
    ast->size = 0;

    return ast;
}

AST_Err_t AST_Destroy(AST** ast) {
    assert( ast != NULL );

    if ((*ast)->root != NULL ) 
        PostorderTraversal((*ast)->root, AST_NodeDestroy);

    FREE(*ast);

    return AST_OK;
}

AST_Node* AST_NodeInit(AST_Node* parent, AST_Node* left, AST_Node* right, AST_ElemType type, ...) {
    AST_Node* node = (AST_Node*)calloc(1, sizeof(AST_Node));
    if (node == NULL) {
        return NULL;
    }

    node->parent = parent;
    node->left = left;
    node->right = right;
    node->type = type;
    node->data.variable = NULL;

    if (node->left) {
        node->left->parent = node;
    }
    if (node->right) {
        node->right->parent = node;
    }

    va_list args;
    va_start(args, type);

    switch (type) {
    case AST_ELEM_TYPE_DECLARATION:
        node->data.declaration_type = va_arg_enum(ConstType);
        break;

    case AST_ELEM_TYPE_OPERATION:
        node->data.operation = va_arg_enum(AST_ElemOperation);
        break;

    case AST_ELEM_TYPE_VARIABLE:
        node->data.variable = strdup(va_arg(args, char*));
        break;

    case AST_ELEM_TYPE_CONST: {
        ConstType const_type = va_arg_enum(ConstType);
        node->data.constant.type = const_type;

        switch (const_type) {
        case CONST_TYPE_SHORT:
            node->data.constant.data.short_const = (short)va_arg(args, int); // short
            break;

        case CONST_TYPE_INT:
            node->data.constant.data.int_const = va_arg(args, int);
            break;
        
        case CONST_TYPE_LONG:
            node->data.constant.data.long_const = va_arg(args, long);
            break;

        case CONST_TYPE_DOUBLE:
            node->data.constant.data.double_const = va_arg(args, double);
            break;

        case CONST_TYPE_CHAR:
            node->data.constant.data.char_const = (char)va_arg(args, int); // char
            break;

        case CONST_TYPE_VOID:
            assert(0);

        case CONST_TYPE_UNDEFINED:
            assert(0);

        default:
            assert(0);
        }

        break;
    }

    case AST_ELEM_TYPE_UNDEFINED:
        break;

    default:
        assert(0);
    }

    va_end(args);

    return node;
}

AST_Err_t AST_NodeDestroy(AST_Node** node_ptr) {
    assert( node_ptr != NULL );

    AST_Node* node = *node_ptr;
    
    switch (node->type) {
    case AST_ELEM_TYPE_DECLARATION:
        node->data.declaration_type = CONST_TYPE_UNDEFINED;
        break;

    case AST_ELEM_TYPE_CONST: 
        node->data.constant.data.long_const = 0;
        node->data.constant.type = CONST_TYPE_UNDEFINED;
        break;
    
    case AST_ELEM_TYPE_OPERATION:
        node->data.operation = AST_ELEM_OPERATION_UNDEFINED;
        break;

    case AST_ELEM_TYPE_VARIABLE:
        FREE(node->data.variable);
        break;

    case AST_ELEM_TYPE_UNDEFINED:
        assert(0);
    
    default:
        assert(0);
        break;
    }

    AST_Node** parent_ptr = GetParentNodePointer(node);
    if (parent_ptr != NULL) {
        *parent_ptr = NULL;
    }

    node->parent = NULL;
    node->right = NULL;
    node->left = NULL;
    
    FREE(*node_ptr);

    return AST_OK;
}

size_t PostorderTraversal(AST_Node* node, AST_NodeFunc func) {
    assert( node != NULL );

    // printf("(");

    size_t vertex_cnt = 0;

    if (node->left != NULL) {
        vertex_cnt += PostorderTraversal(node->left, func);
        // fprintf(stderr, "L\n");
    }

    if (node->right != NULL) {
        vertex_cnt += PostorderTraversal(node->right, func);
        // fprintf(stderr, "R\n");
    }

    if (func) {
        func(&node);
    }

    // printf(")");

    return vertex_cnt + 1;
}

AST_Node** GetParentNodePointer(AST_Node* node) {
    assert( node != NULL );

    if (node->parent == NULL) {
        return NULL;
    }

    if (node->parent->left == node){
        return &node->parent->left;
    } else if (node->parent->right == node) {
        return &node->parent->right;
    }

    fprintf(stderr, "GetParentNodePointer failed!\n");
    return NULL;
}