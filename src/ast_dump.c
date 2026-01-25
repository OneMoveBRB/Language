#include "../include/ast_dump.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../include/io.h"

static size_t DotInitNodes(AST_Node* node, FILE* fp, size_t* node_cnt);

typedef struct AST_OperationMapping {
    const char* string;
    AST_ElemOperation operation;
} AST_OperationMapping;

typedef struct ConstTypeMapping {
    const char* string;
    ConstType type;
} ConstTypeMapping;

AST_OperationMapping ast_operation_dict[] = {
    {";"     ,      AST_ELEM_OPERATION_SENTINEL  },
    
    {"+"     ,      AST_ELEM_OPERATION_ADD       },
    {"-"     ,      AST_ELEM_OPERATION_SUB       },
    {"*"     ,      AST_ELEM_OPERATION_MUL       },
    {"/"     ,      AST_ELEM_OPERATION_DIV       },

    {"\\<"     ,      AST_ELEM_OPERATION_LT        },
    {"\\>"     ,      AST_ELEM_OPERATION_GT        },
    {"\\<="    ,      AST_ELEM_OPERATION_LE        },
    {"\\>="    ,      AST_ELEM_OPERATION_GE        },
    {"=="    ,      AST_ELEM_OPERATION_EE        },
    {"!="    ,      AST_ELEM_OPERATION_NE        },

    {"="     ,      AST_ELEM_OPERATION_ASSIGNMENT},

    {"if"    ,      AST_ELEM_OPERATION_IF        },
    {"else"  ,      AST_ELEM_OPERATION_ELSE      },
    {"while" ,      AST_ELEM_OPERATION_WHILE     },
    {"break" ,      AST_ELEM_OPERATION_BREAK     },
    {"return",      AST_ELEM_OPERATION_RETURN    }
};

size_t ast_operation_dict_size = sizeof(ast_operation_dict)/sizeof(AST_OperationMapping);

ConstTypeMapping ast_const_dict[] = {
    {"short", CONST_TYPE_SHORT},
    {"int", CONST_TYPE_INT},
    {"long", CONST_TYPE_LONG},
    {"double", CONST_TYPE_DOUBLE},
    {"char", CONST_TYPE_CHAR}
};

size_t ast_const_dict_size = sizeof(ast_const_dict)/sizeof(ConstTypeMapping);

const char* GetStrOp(AST_ElemOperation operation) {
    for (size_t i = 0; i < ast_operation_dict_size; i++) {
        if (ast_operation_dict[i].operation == operation) {
            return ast_operation_dict[i].string;
        }
    }

    return NULL;
}

const char* GetStrConst(ConstType type) {
    for (size_t i = 0; i < ast_const_dict_size; i++) {
        if (ast_const_dict[i].type == type) {
            return ast_const_dict[i].string;
        }
    }

    return NULL;
}

void DotVizualizeTree(const AST* ast, const char* file_name) {
    assert( ast != NULL );
    assert( file_name != NULL );

    FILE* fp = fopen(file_name, "w");
    if (fp == NULL) {
        return; 
    }

    fprintf(fp, "digraph AST {\n\t");
    fprintf(fp, "rankdir=HR;\n\t");
    fprintf(fp, "node [shape=record, style=filled, fillcolor=lightblue];\n\t");
    fprintf(fp, "edge [fontsize=10,  color=black];\n\n\t");

    size_t node_cnt = 0;
    DotInitNodes(ast->root, fp, &node_cnt);

    fprintf(fp, "\n}");

    fclose(fp);

    const char* command = MultiStrCat(3, "dot -Tsvg ", file_name, " > img.svg");
    if (command == NULL) {
        fprintf(stderr, "CAN'T GET COMMAND\n");
        return;
    }

    system(command);

    free((char*)command);
    command = NULL;

    return;
}

static size_t DotInitNodes(AST_Node* node, FILE* fp, size_t* node_cnt) {
    assert( node != NULL );
    assert(  fp != NULL  );

    // fprintf(stderr, "NOde: %p\n", node);
    size_t left_node = 0, right_node = 0;
    if (node->left != NULL) {
        left_node = DotInitNodes(node->left, fp, node_cnt);
    }

    if (node->right != NULL) {
        right_node = DotInitNodes(node->right, fp, node_cnt);
    }

    ++(*node_cnt);

    if (node->type == AST_ELEM_TYPE_CONST) {
        ConstType const_type = node->data.constant.type;

        fprintf(
            fp, 
            "node%zu [label=\"{{{<f0> %p"
            " | <f1> type = CONST | <f5> %s | <f2> data = "
            , 
            *node_cnt, node, GetStrConst(const_type)
        );

        switch (const_type) {
        case CONST_TYPE_SHORT:
            fprintf(fp, "%hd", node->data.constant.data.short_const);
            break;

        case CONST_TYPE_INT:
            fprintf(fp, "%d", node->data.constant.data.int_const);
            break;

        case CONST_TYPE_LONG:
            fprintf(fp, "%ld", node->data.constant.data.long_const);
            break;
        
        case CONST_TYPE_DOUBLE:
            fprintf(fp, "%lg", node->data.constant.data.double_const);
            break;

        case CONST_TYPE_CHAR:
            fprintf(fp, "%c", node->data.constant.data.char_const);
            break;

        case CONST_TYPE_VOID:
            fprintf(fp, "void");
            break;

        case CONST_TYPE_UNDEFINED:
            assert(0);
        
        default:
            assert(0);
        }

        fprintf(
            fp,
            "}}"
            " | { <f3> left: %p | <f4> right: %p}}\"];\n\t",
            node->left, node->right
        );
    }
            
    else if (node->type == AST_ELEM_TYPE_OPERATION) { 
        fprintf(fp, "node%zu [label=\"{{{<f0> %p | <f1> type = OPERATION | <f2> data = %s}}"
                    " | { <f3> left: %p | <f4> right: %p}}\"];\n\t", 
                *node_cnt, node, GetStrOp(node->data.operation), node->left, node->right);
        }

    else if (node->type == AST_ELEM_TYPE_DECLARATION) { 
        fprintf(fp, "node%zu [label=\"{{{<f0> %p | <f1> type = DECLARATION | <f2> data = %s}}"
                    " | { <f3> left: %p | <f4> right: %p}}\"];\n\t", 
                *node_cnt, node, GetStrConst(node->data.declaration_type), node->left, node->right);
        }

    else if (node->type == AST_ELEM_TYPE_VARIABLE)
        fprintf(fp, "node%zu [label=\"{{{<f0> %p | <f1> type = VARIABLE | <f2> data = %s}}"
                    " | { <f3> left: %p | <f4> right: %p}}\"];\n\t", 
                *node_cnt, node, node->data.variable, node->left, node->right);

    if (node->left != NULL) {
        if (node->left->parent == node) {
            fprintf(fp, "node%zu:f3 -> node%zu [color=red, dir=both, arrowhead=normal];\n\t", *node_cnt, left_node);
        } else {
            fprintf(fp, "node%zu:f3 -> node%zu [color=purple, arrowhead=normal];\n\t", *node_cnt, left_node);
            fprintf(fp, "node%zu -> node%zu:f3 [color=purple, arrowhead=normal];\n\t", left_node, *node_cnt);
        }
    }
    if (node->right != NULL) {
        if (node->right->parent == node) {
            fprintf(fp, "node%zu:f4 -> node%zu [color=green, dir=both, arrowhead=normal];\n\t", *node_cnt, right_node);
        } else {
            fprintf(fp, "node%zu:f4 -> node%zu [color=purple, arrowhead=normal];\n\t", *node_cnt, right_node);
            fprintf(fp, "node%zu -> node%zu:f4 [color=purple, arrowhead=normal];\n\t", right_node, *node_cnt);
        }
    }

    return *node_cnt;
}