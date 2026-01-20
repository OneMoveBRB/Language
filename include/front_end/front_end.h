#ifndef FRONT_END_H
#define FRONT_END_H

typedef enum TokenType {
    TOKEN_TYPE_UNDEF,

    TOKEN_TYPE_VARIABLE,
    TOKEN_TYPE_CONST,

    TOKEN_TYPE_SHORT,
    TOKEN_TYPE_INT,
    TOKEN_TYPE_LONG,
    TOKEN_TYPE_DOUBLE,
    TOKEN_TYPE_CHAR,
    TOKEN_TYPE_VOID,

    TOKEN_TYPE_BOOL_TRUE,
    TOKEN_TYPE_BOOL_FALSE,

    TOKEN_TYPE_COMMA,
    TOKEN_TYPE_SEMICOLON,

    TOKEN_TYPE_ROUND_BRACKET_OPEN,
    TOKEN_TYPE_ROUND_BRACKET_CLOSE,
    TOKEN_TYPE_CURLY_BRACKET_OPEN,
    TOKEN_TYPE_CURLY_BRACKET_CLOSE,
    TOKEN_TYPE_SQUARE_BRACKET_OPEN,
    TOKEN_TYPE_SQUARE_BRACKET_CLOSE,
    
    TOKEN_TYPE_BIN_ADD,
    TOKEN_TYPE_BIN_SUB,
    TOKEN_TYPE_BIN_MUL,
    TOKEN_TYPE_BIN_DIV,

    TOKEN_TYPE_BIN_LT, // <
    TOKEN_TYPE_BIN_GT, // >
    TOKEN_TYPE_BIN_LE, // <=
    TOKEN_TYPE_BIN_GE, // >=
    TOKEN_TYPE_BIN_EE, // ==
    TOKEN_TYPE_BIN_NE, // !=

    TOKEN_TYPE_LAND,   // &&
    TOKEN_TYPE_LOR,    // ||

    TOKEN_TYPE_ASSIGNMENT,

    TOKEN_TYPE_STATEMENT_IF,
    TOKEN_TYPE_STATEMENT_ELSE,
    TOKEN_TYPE_STATEMENT_WHILE,
    TOKEN_TYPE_STATEMENT_BREAK,
    TOKEN_TYPE_STATEMENT_RETURN,

    TOKEN_TYPE_END
} TokenType;

#ifndef CONST
#define CONST
typedef enum ConstType {
    CONST_TYPE_UNDEFINED,
    CONST_TYPE_SHORT,
    CONST_TYPE_INT,
    CONST_TYPE_LONG,
    CONST_TYPE_DOUBLE,
    CONST_TYPE_CHAR
} ConstType;

typedef union ConstData {
    short short_const;
    int int_const;
    long long_const;
    double double_const;
    char char_const;
} ConstData;

typedef struct Const {
    ConstType type;
    ConstData data;
} Const;
#endif /* CONST */

typedef union TokenData {
    char* variable;
    Const constant;
} TokenData;

typedef struct Token {
    TokenType type;
    TokenData data;
} Token;

typedef enum FrontEndErr_t {
    FRONT_END_OK,
    FRONT_END_BUFFER_FAILED
} FrontEndErr_t;

typedef struct AST AST;

FrontEndErr_t FrontEnd(AST** ast, const char* file_name);

#endif /* FRONT_END_H */