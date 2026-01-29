#include "../../include/middle_end/ast_optimization.h"

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "../../include/ast/ast.h"














/*
#define cL TreeCopySubtree(node->left, node)
#define cR TreeCopySubtree(node->right, node)
#define c(x) \
    NodeInit(NULL, NULL, NULL, TYPE_NUMBER, x)

#define IS_VALUE(ptr, val) \
    (ptr->type == TYPE_NUMBER && isEqual(ptr->data.number, val))

static TreeElemType NodeOptimization(Tree_t* tree, Node_t* node);
static TreeElemType CreateConstNode(Tree_t* tree, Node_t* node, Node_t** parent_ptr);
static TreeElemType ConstOptimizationAdd(Tree_t* tree, Node_t* node, Node_t** parent_ptr);
static TreeElemType ConstOptimizationSub(Tree_t* tree, Node_t* node, Node_t** parent_ptr);
static TreeElemType ConstOptimizationMul(Tree_t* tree, Node_t* node, Node_t** parent_ptr);
static TreeElemType ConstOptimizationDiv(Tree_t* tree, Node_t* node, Node_t** parent_ptr);
static TreeElemType ConstOptimizationExp(Tree_t* tree, Node_t* node, Node_t** parent_ptr);

static double GetFuncOp(Operation_t operation, const double* const left_number, 
                                               const double* const right_number);

TreeErr_t TreeOptimization(Tree_t* tree) {
    assert( tree != NULL );

    NodeOptimization(tree, tree->root);
    TreeUpdateSize(tree);

    size_t cur_size = tree->size;
    while (true) {
        NodeOptimization(tree, tree->root);
        TreeUpdateSize(tree);

        if (cur_size == tree->size) {
            break;
        }

        cur_size = tree->size;
    }

    return TREE_OK;
}

static TreeElemType NodeOptimization(Tree_t* tree, Node_t* node) {
    assert( tree != NULL );
    assert( node != NULL );
    // fprintf(stderr, "NodeOptimization: %p\n", node);
    // getchar();

    TreeElemType left_type = TYPE_UNDEFINED,
                 right_type = TYPE_UNDEFINED;
    if (node->left) {
        left_type = NodeOptimization(tree, node->left);
    }
    if (node->right) {
        right_type = NodeOptimization(tree, node->right);
    }

    Node_t** parent_ptr = (node->parent) ? GetParentNodePointer(node) : &tree->root;
    if ((left_type == TYPE_NUMBER || left_type == TYPE_UNDEFINED) && right_type == TYPE_NUMBER) {
        return CreateConstNode(tree, node, parent_ptr);
    }

    if (node->type != TYPE_OPERATION) {
        return node->type;
    }

    switch (node->data.operation) {
    case OPERATION_ADD:
        return ConstOptimizationAdd(tree, node, parent_ptr);

    case OPERATION_SUB:
        return ConstOptimizationSub(tree, node, parent_ptr);

    case OPERATION_MUL:
        return ConstOptimizationMul(tree, node, parent_ptr);

    case OPERATION_DIV:
        return ConstOptimizationDiv(tree, node, parent_ptr);

    case OPERATION_EXP:
        return ConstOptimizationExp(tree, node, parent_ptr);
    
    default:
        break;
    }

    return node->type;
}

static TreeElemType CreateConstNode(Tree_t* tree, Node_t* node, Node_t** parent_ptr) {
    assert( node != NULL );
    assert( parent_ptr != NULL );
    // fprintf(stderr, "CreateConstNode: %p\n", node);
    // getchar();

    Node_t* parent = node->parent;

    const double* const left_number = (node->left) ? &node->left->data.number : NULL;
    const double* const right_number = &node->right->data.number;

    double value = GetFuncOp(node->data.operation, 
                                left_number, 
                                right_number);

    TreeDeleteSubtree(tree, node);
    *parent_ptr = NodeInit(parent, NULL, NULL, TYPE_NUMBER, value);

    return TYPE_NUMBER;
}

#define STR(x_) #x_

#define ConstOtimizationHandler(func_name, expressions)                                     \
static TreeElemType func_name(Tree_t* tree, Node_t* node, Node_t** parent_ptr) {            \
    assert( node != NULL );                                                                 \
    assert( parent_ptr != NULL );                                                           \
    // fprintf(stderr, STR(func_name)": %p\n", node);                                       \
    // getchar();                                                                           \
                                                                                            \
    Node_t* parent = node->parent;                                                          \
    Node_t* new_node = NULL;                                                                \
                                                                                            \
    expressions                                                                             \
                                                                                            \
    TreeDeleteSubtree(tree, node);                                                          \
    *parent_ptr = new_node;                                                                 \
    new_node->parent = parent;                                                              \
                                                                                            \
    return TYPE_OPERATION;                                                                  \
}

ConstOtimizationHandler(
    ConstOptimizationAdd,
    if      (IS_VALUE(node->left,  0.f)) new_node = cR;
    else if (IS_VALUE(node->right, 0.f)) new_node = cL;
    else    return TYPE_OPERATION;
)

ConstOtimizationHandler(
    ConstOptimizationSub,
    if      (IS_VALUE(node->right, 0.f) ) new_node = cL;
    else    return TYPE_OPERATION;
)

ConstOtimizationHandler(
    ConstOptimizationMul,
    if      (IS_VALUE(node->left,  0.f) ) new_node = c(0.f);
    else if (IS_VALUE(node->right, 0.f) ) new_node = c(0.f);
    else if (IS_VALUE(node->left,  1.f) ) new_node = cR;
    else if (IS_VALUE(node->right, 1.f) ) new_node = cL;
    else    return TYPE_OPERATION;
)

ConstOtimizationHandler(
    ConstOptimizationDiv,
    if      (IS_VALUE(node->left,  0.f) ) new_node = c(0.f);
    else if (IS_VALUE(node->right, 1.f) ) new_node = cL;
    else    return TYPE_OPERATION;
)

ConstOtimizationHandler(
    ConstOptimizationExp,
    if      (IS_VALUE(node->right, 0.f)) new_node = c(1.f);
    else if (IS_VALUE(node->left,  1.f)) new_node = c(1.f);
    else if (IS_VALUE(node->right, 1.f)) new_node = cL;
    else    return TYPE_OPERATION;
)

static double GetFuncOp(Operation_t operation, const double* const left_number, 
                                               const double* const right_number) {
    double a = (left_number) ? *left_number : -1;
    double b = *right_number;

    switch (operation) {
    case OPERATION_ADD:
        return a + b;

    case OPERATION_SUB:
        return a - b;

    case OPERATION_MUL:
        return a * b;

    case OPERATION_DIV:
        return a / b;

    case OPERATION_EXP:
        return pow(a, b);

    case OPERATION_SQRT:
        return sqrt(b);

    case OPERATION_LN:
        return log(b);

    case OPERATION_LOG:
        // log(a)/log(b)
        fprintf(stderr, "OPERATION_LOG in GetFuncOp\n");
        break;

    case OPERATION_SIN:
        return sin(b * (M_PI / 180.0));

    case OPERATION_COS:
        return cos(b * (M_PI / 180.0));

    case OPERATION_TAN:
        return tan(b * (M_PI / 180.0));

    case OPERATION_COT:
        return 1.0 / tan(b * (M_PI / 180.0));

    case OPERATION_SINH:
        return sinh(b * (M_PI / 180.0));

    case OPERATION_COSH:
        return cosh(b * (M_PI / 180.0));

    case OPERATION_TANH:
        return tanh(b * (M_PI / 180.0));

    case OPERATION_COTH:
        return 1.0 / tanh(b * (M_PI / 180.0));
    
    case OPERATION_ASIN:
        return asin(b * (M_PI / 180.0));

    case OPERATION_ACOS:
        return acos(b * (M_PI / 180.0));

    case OPERATION_ATAN:
        return atan(b * (M_PI / 180.0));

    case OPERATION_ACOT:
        return atan(1.0 / (b * (M_PI / 180.0)));

    case OPERATION_UNDEF:
        fprintf(stderr, "UNDEFINED_OPERATION IN GetFuncOp\n");
        break;

    default:
        fprintf(stderr, "default in GetFuncOp\n");
        break;
    }

    return 0;
}
*/